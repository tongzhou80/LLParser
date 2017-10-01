//
// Created by tlaber on 6/10/17.
//

#include <cstring>
#include <cassert>
#include <algorithm>
#include <utilities/flags.h>
#include <utilities/strings.h>
#include <inst/instEssential.h>
#include <inst/branchInst.h>
#include "instParser.h"
#include "sysDict.h"

//std::map<string, inst_parse_routine> InstParser::_table;
//int InstParser::MAX_VALUE_LEN = 1024;


Instruction* InstParser::create_instruction(string &text) {
    set_text(text);
    string op;
    string name;
    bool has_assignment = false;

    if (Strings::startswith(text, "%")) {
        get_word();
        name = _word;
        match('=');
        has_assignment = true;
    }

    get_word_of(" ,");
    op = _word;
    Instruction* inst = NULL;
    //void (InstParser::*parse_routine) (Instruction*) = NULL;

    if (op == "tail" || op == "musttail" || op == "notail") {
        op = "call";
    }

    /* unaware of the instruction type at this point */
    switch (op[0]) {
        case 'a': {
            if (op == "alloca") {
                //inst = new AllocaInst();
            }
            break;
        }
        case 'b': {
            if (op == "bitcast") {
                inst = new BitCastInst();
                //parse_routine = &InstParser::do_bitcast;
            }
            else if (op == "br") {
                inst = new BranchInst();
                //parse_routine = &InstParser::do_branch;
            }
            break;
        }
        case 'c': {
            if (op == "call") {
                inst = new CallInst();
                //parse_routine = &InstParser::do_call_family;
            }
            break;
        }
        case 'i': {
            if (op == "invoke") {
                inst = new InvokeInst();
                //parse_routine = &InstParser::do_call_family;
            }
            break;
        }
        case 'l': {
            if (op == "load") {
                inst = new LoadInst();
                //parse_routine = &InstParser::do_load;
            }
            break;
        }
        default: {
            break;
        }
    }

    if (!inst) {
        inst = new Instruction();
    }

    inst->set_opcode(op);
    inst->set_has_assignment(has_assignment); // have to repeat this for every inst
    if (has_assignment) {
        inst->set_name(name);
    }
    inst->set_raw_text(text);

    if (inst->type() != Instruction::UnknownInstType) {
        parse(inst);
        //(this->*parse_routine)(inst);
        parse_metadata(inst);
        //
    }
    return inst;

}

void InstParser::parse(Instruction *inst) {
    switch (inst->type()) {
//        case Instruction::AllocaInstType: {
//
//        }
        case Instruction::BranchInstType: {
            do_branch(inst);
            break;
        }
        case Instruction::CallInstType: {
            do_call_family(inst);
            break;
        }
        case Instruction::InvokeInstType: {
            do_call_family(inst);
            break;
        }
        case Instruction::LoadInstType: {
            do_load(inst);
            break;
        }
        case Instruction::BitCastInstType :
            do_bitcast(inst);
            break;
        default:
            guarantee(0, "sanity");
    }
}

/**@brief Parse instruction's metadata
 *
 * The rest of text(_intext_pos) should start with ", !" or " !"
 * @param ins
 */
void InstParser::parse_metadata(Instruction *ins) {
    if (_eol) {
        return;
    }

    if (_char == ',') {
        inc_intext_pos();
    }
    match(" !");
    get_word('!');
    if (_word == "dbg ") {
        ins->set_dbg_id(parse_integer());
    }

    /* todo: more metadata */
}

/*
 * <result> = [tail | musttail | notail ] call [fast-math flags] [cconv] [ret attrs] <ty>|<fnty> <fnptrval>(<function args>) [fn attrs]
             [ operand bundles ]
 *
 * 'ty':   the type of the call instruction itself which is also the type of the return value.
 *         Functions that return no value are marked void.
 * 'fnty': shall be the signature of the function being called.
 *         The argument types must match the types implied by this signature.
 *         This type can be omitted if the function is not varargs.
 *
 * <result> = invoke [cconv] [ret attrs] <ty>|<fnty> <fnptrval>(<function args>) [fn attrs]
              [operand bundles] to label <normal label> unwind label <exception label>

 *   %call.i4.i93 = invoke i8* @_Znam(i64 20) #12
          to label %invoke.cont unwind label %lpad, !dbg !557
 *
 */
void InstParser::do_call_family(Instruction* inst) {
    /* corner cases:
     * %6 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272) %4, i8* %5)
     */
    CallInstFamily* ci = dynamic_cast<CallInstFamily*>(inst);

    string ret_ty, fnty;

    if (InstFlags::in_tails(_word)) {
        ci->set_raw_field("tail", _word);
        get_word();
    }

    parser_assert(_word == "call" || _word == "invoke", "word: %s", _word.c_str());

    get_lookahead();
    if (inst->type() == Instruction::CallInstType) {
        if (InstFlags::in_fastmaths(_lookahead)) {
            ci->set_raw_field("fast-math", _lookahead);
            jump_ahead();
            get_lookahead();
        }
    }

    if (InstFlags::in_cconvs(_lookahead)) {
        ci->set_raw_field("cconv", _lookahead);
        jump_ahead();
        get_lookahead();
    }

    if (InstFlags::in_param_attrs(_lookahead)) {
        // This assert is in the document, but not in practice
        //assert(_word == "zeroext" || _word == "signext" || _word == "inreg" || _word == "noalias" &&
        //          "Only ‘zeroext‘, ‘signext‘, and ‘inreg‘ attributes are valid here for return type");
        ci->set_raw_field("ret-attrs", _lookahead);
        jump_ahead();
    }

    parse_basic_type();

    skip_ws();

    /* could be either function parameter types or part of ret_ty if ret_ty is function pointer */
    /* Samples
     * %12 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %11, i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.9.134, i64 0, i64 0), i32 %0) #14
     */
    // void (ty..) is for varargs type check
    // void (ty..)[*|**] is a return type, which is a function pointer type
    if (_char == '(') {
        string args_sig = jump_to_end_of_scope();

        /* ()* => part of pointer type */
        if (_char == '*') {
            while (_char == '*') {
                inc_intext_pos();
            }
        } // (...) variable args check
        else {
            ci->set_is_varargs();
            parser_assert(Strings::endswith(args_sig, "...)"), "vararg signature should end with '...)'");
            ci->set_raw_field("fnty", args_sig);
        }
        inc_intext_pos();
    }



    if (CallInstParsingVerbose) {
        printf( "call:\n"
                "  ret_ty: |%s|\n"
                "  fnty: |%s|\n",
              ret_ty.c_str(), fnty.c_str());
    }

    /* deal with bitcast first if at all
     * corner cases:
     * tail call void bitcast (void (%struct.bContext*, %struct.uiBlock.22475* (%struct.bContext*, %struct.ARegion*, i8*)*, i8*)* @uiPupBlock
     * to void (%struct.bContext*, %struct.uiBlock* (%struct.bContext*, %struct.ARegion*, i8*)*, i8*)*)
     * (%struct.bContext* %C, %struct.uiBlock* (%struct.bContext*, %struct.ARegion*, i8*)* nonnull @wm_enum_search_menu, i8* %0) #3
     */

    if (_char == 'b') {
        match("bitcast (");
        ci->set_has_bitcast();

        //parse_function_pointer_type();
        parse_compound_type();
        skip_ws();
        parser_assert(_char == '%' || _char == '@', " ");
    }


    // %15 = call i64 (i8*, i8*, ...) bitcast (i64 (...)* @f90_auto_alloc04 to i64 (i8*, i8*, ...)*)
    // (i8* nonnull %14, i8* bitcast (i32* @.C323_shell_ to i8*))"

    /* direct function call */
    if (_char == '@') {
        inc_intext_pos();
        string fn_name;
        if (ci->has_bitcast()) {
            get_word();
            fn_name = _word;
            match("to");
            parse_compound_type();
            match(')');  // args start here
        }
        else {
            get_word('(', false, false);
            fn_name = _word;
        }

        ci->set_raw_field("fnptrval", fn_name);
    }
    else if (_char == '%') {
        // todo: indirect calls
        inc_intext_pos();
        get_word('(', false, false);
        string label = _word;
        ci->set_is_indirect_call();
        ci->set_called_label(label);
        ci->set_raw_field("fnptrval", label);
        parser_assert(!ci->has_bitcast(), "just check");
    }
    else {
        parser_assert(0, "Expect '%%' or '@', char: |%c|, pos: %d", _char, _intext_pos);
    }

    /* do args */
    string args = jump_to_end_of_scope();
    ci->set_raw_field("args", args.substr(1, args.size()-2)); // strip the ()

    /* The optional function attributes list */
    while (!_eol) {
        get_lookahead_of(", ");
        if (_lookahead[0] == '#') {
            jump_ahead();
            ci->set_raw_field("fn-attrs", _word);
        }
        else {
            break;
        }
    }

    if (inst->type() == Instruction::InvokeInstType) {
        match("to label ", true);
        get_word();
        inst->set_raw_field("normal-label", _word);
        match("unwind label ");
        get_word(',');
        inst->set_raw_field("exception-label", _word);
    }
    return;
}



/**
 * Assume always have "align"
 * Syntax
 * <result> = load [volatile] <ty>, <ty>* <pointer>[, align <alignment>][, !nontemporal !<index>][, !invariant.load !<index>][, !invariant.group !<index>][, !nonnull !<index>][, !dereferenceable !<deref_bytes_node>][, !dereferenceable_or_null !<deref_bytes_node>][, !align !<align_node>]
   <result> = load atomic [volatile] <ty>, <ty>* <pointer> [singlethread] <ordering>, align <alignment> [, !invariant.group !<index>]
   !<index> = !{ i32 1 }
   !<deref_bytes_node> = !{i64 <dereferenceable_bytes>}
   !<align_node> = !{ i64 <value_alignment> }
 *
 * @param inst
 */
void InstParser::do_load(Instruction *inst) {
    /* corner case:
     *    %9 = load i8* (i8*, i32, i32)*, i8* (i8*, i32, i32)** %8, align 8, !dbg !4557, !tbaa !4559
     *    %14 = load void (%class.Base*)**, void (%class.Base*)*** %13, align 8
     */
    LoadInst* li = dynamic_cast<LoadInst*>(inst);

    get_lookahead();
    if (_lookahead == "atomic") {
        li->set_raw_field("atomic", "");
        jump_ahead();
        get_lookahead();
    }

    if (_lookahead == "volatile") {
        li->set_raw_field("volatile", "");
        jump_ahead();
    }


    string ty = parse_compound_type();
    if (_char != ',')
    syntax_check(_char == ',');
    inc_intext_pos(2);
    string ty_p = parse_compound_type();
    syntax_check(ty_p == ty + '*');

    get_word_of(" ,");

    if (_word == "getelementptr") {
        GetElementPtrInst* gepi = new GetElementPtrInst();
        do_getelementptr(gepi, true);
        match(',');
    }
    else if (_word == "bitcast") {
        BitCastInst* bci = new BitCastInst();
        do_bitcast(bci, true);
        li->set_raw_field("pointer", bci->get_raw_field("value"));
        syntax_check(bci->get_raw_field("ty2") == ty_p);
        match(',');
    }
    else {
        li->set_raw_field("pointer", _word);  // might do some syntax check on pointer
    }

    if (_eol) {
        return;
    }

    match(" align ");
    get_word(',');
    li->set_raw_field("alignment", _word);
    if (_eol) {
        return;
    }

    /* TODO: more field */
}

void InstParser::parse_function_pointer_type() {
    /* corner case
     * tail call void bitcast (void (%struct.bContext*, %struct.uiBlock.22475* (%struct.bContext*, %struct.ARegion*, i8*)*, i8*)* @uiPupBlock
     */
    if (_text.find("@WM_gesture_lasso_path_to_array to") != _text.npos) {

    }

    parse_basic_type();

    while (_char == ' ') {
        inc_intext_pos();
    }
    guarantee(_char == '(', "Bad function pointer type: %s", _text.c_str());
    int unmatched = 1;
    //bool has_close_paren = false;

    //while (unmatched != 0 && !has_close_paren) {
    while (unmatched != 0) {
        inc_intext_pos();
        if (_char == '(') {
            unmatched++;
        }
        else if (_char == ')') {
            unmatched--;
            //has_close_paren = true;
        }
    }
    inc_intext_pos();
    guarantee(_char == '*', "Bad function pointer type: %s", _text.c_str());
    inc_intext_pos(2);
}

void InstParser::do_bitcast(Instruction *inst, bool is_embedded) {
    /* corner case
     *  %1 = bitcast void (...)* bitcast (void (i64*, i32)* @wrf_error_fatal_ to void (...)*) to void (i8*, i64, ...)*
     */
    BitCastInst* I = dynamic_cast<BitCastInst*>(inst);
    if (is_embedded) {
        match('(');
    }

    string old_ty = parse_compound_type();
    I->set_raw_field("ty", old_ty);
    get_word();

    if (_word == "bitcast") {
        BitCastInst* embedded_bci = new BitCastInst();
        do_bitcast(embedded_bci, true);
        I->set_raw_field("value", embedded_bci->get_raw_field("value"));
        syntax_check(old_ty == embedded_bci->get_raw_field("ty2"));
    }
    else if (_word == "getelementptr") {
        GetElementPtrInst* gepi = new GetElementPtrInst();
        do_getelementptr(gepi, true);
    }
    else {
        I->set_raw_field("value", _word);
    }

    match("to", true);
    string new_ty = parse_compound_type();
    I->set_raw_field("ty2", new_ty);
    if (is_embedded) {
        match(')');
    }
}

//void InstParser::skip_and_check_opcode(const char* op, Instruction *inst) {
//    if (inst->has_assignment())
//        get_word('=');
//    get_word();
//    guarantee(_word == string(op), "Not a %s instruction: %s", op, inst->raw_c_str());
//}

void InstParser::do_branch(Instruction *inst) {
    /* Corner cases:
     * 1. br i1 %1352, label %1353, label %1312, !llvm.loop !110422
     *
     */
    get_word();

    // conditional
    if (_word == "i1") {
        get_word(',');
        inst->set_raw_field("cond", _word);
        get_word();
        DCHECK(_word == "label", " ");
        get_word(',');
        inst->set_raw_field("true-label", _word);
        get_word();
        DCHECK(_word == "label", " ");
        get_word_of(" ,");
        inst->set_raw_field("false-label", _word);
    } // unconditional
    else if (_word == "label") {
        get_word_of(" ,");
        inst->set_raw_field("true-label", _word);  // unconditional branches only use 'true-label'
    }
    else {
        syntax_check(0);
    }
}

void InstParser::do_alloca(Instruction *ins) {

}

void InstParser::do_getelementptr(Instruction *inst, bool is_embedded) {
    get_lookahead();
    if (_lookahead == "inbounds") {
        jump_ahead();
    }
    skip_ws();
    if (is_embedded) {
        syntax_check(_char == '(');
        string args = jump_to_end_of_scope();
    }
}