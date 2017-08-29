//
// Created by tlaber on 6/10/17.
//

#include <cstring>
#include <cassert>
#include <utilities/flags.h>
#include "instParser.h"
#include <utilities/strings.h>
#include "sysDict.h"
#include <inst/instEssential.h>
#include <algorithm>


//std::map<string, inst_parse_routine> InstParser::_table;
//int InstParser::MAX_VALUE_LEN = 1024;

InstParser::InstParser() {
    MAX_VALUE_LEN = 1024;
    _intext_pos = 0;
    _char = 0;
    _eol = false;
//    _table["alloca"] = InstParser::do_alloca;
//    _table["call"] = InstParser::do_call;
}


void InstParser::parse(Instruction *inst) {
    string text = inst->raw_text();
    set_text(text);
    switch (inst->type()) {
        case Instruction::CallInstType: {
            do_call_family(inst); break;
        }
        case Instruction::InvokeInstType: {
            //do_call_family(inst); break;
        }
        case Instruction::LoadInstType: {
            break;
            //do_load(inst); break;
        }
        case Instruction::BitCastInstType :
            do_bitcast(inst);
            break;
    }
}


// Get first word of the right hand side
string InstParser::get_first_word(string& text) {
    if (Strings::startswith(text, "%")) {
        const char* fm1 = "  %%%[^ ] = %[^ ]";
        char var_name[1024];
        char opcode1[128];
        int matched = sscanf(text.c_str(), fm1, var_name, opcode1);

        /* !! needs to change as more fields are parsed in the instruction line */
        guarantee(matched == 2, "Bad instruction: %s", text.c_str());
        zpl("match: %d, id: %s, op: %s", matched, var_name, opcode1);
        return opcode1;
    }
    else {
        const char* fm2 = "  %[^ ]";
        char opcode2[128];
        int matched = sscanf(text.c_str(), fm2, opcode2);
        guarantee(matched == 1, "Bad instruction: %s", text.c_str());
        zpl("match: %d, op: %s", matched, opcode2);
        return opcode2;
    }
}


///* Tedious but still LL(1), thank God */
//void InstParser::parse_type() {
//    if (_word[0] == 'v') {
//        guarantee(_word == "void", " ");
//    }
//    else if (_word[0] == 'i') {
//        if (_word[_word.size()-1] == '*') {
//            //zpl("int pointer: %s", _word.c_str());
//        }
//    }
//    else if (_word[0] == 'f') {
//
//    }
//    else if (_word[0] == 'd') {
//
//    }
//    else if (_word[0] == 'h') {
//
//    }
//    else if (_word[0] == '<') {
//        string vec_type = _word.substr(1);
//        get_word('>');
//        inc_intext_pos();
//        vec_type = vec_type + ' ' + _word;
//        //zpl("vector: %s", _word.c_str());
//    }
//    else if (_word[0] == '%'){
////        guarantee(Strings::startswith(_word, "%union") || Strings::startswith(_word, "%struct"),
////                  "Bad type start with: %s", word.c_str());
//    }
//    else if (_word[0] == '{') {  // %5 = tail call { i64, { float, float }* } @quantum_new_matrix(i32 2, i32 2) #9, !dbg !2700
//        int braces = 1;
//        while (braces != 0) {
//            inc_intext_pos();
//            if (_char == '{') {
//                ++braces;
//            }
//            else if (_char == '}') {
//                --braces;
//            }
//        }
//
//        // _char should be '}' now
//        guarantee(_char == '}', " ");
//        inc_intext_pos(2);
//
//    }
//    else if (_word[0] == '[') {
//        /* an array type like  [5 x float]** */
//        get_word(']');
//        while (_char == '*') {
//            inc_intext_pos();
//        }
//    }
//    else {
//        guarantee(0, "bad type: %s", _text.c_str());
//    }
//}

/* Tedious but still LL(1), thank God */
string InstParser::parse_basic_type() {
    skip_ws();

    string fulltype;
    switch (_char) {
        case 'v': {
            get_word();
            fulltype = _word;
            guarantee(_word == "void", " ");
            break;
        }
        case 'i': {  // i8, i16, i32...
            get_word();
            fulltype = _word;
            break;
        }
        case 'f': {  // float
            get_word();
            fulltype = _word;
            break;
        }
        case 'd': {  // double
            get_word();
            fulltype = _word;
            break;
        }
        case 'h': {  // half
            get_word();
            fulltype = _word;
            break;
        }
        case '<': {  // vector
            fulltype = jump_to_end_of_scope();
            break;
        }
        case '%': {
            get_word();
            fulltype = _word;
            break;
        }
        case '{': {  // structure type like %5 = tail call { i64, { float, float }* } @quantum_new_matrix(i32 2, i32 2) #9, !dbg !2700
            fulltype = jump_to_end_of_scope();
            break;
        }
        case '[': {  // array type
            fulltype = jump_to_end_of_scope();
            break;
        }
        default: {
            parser_assert(0, text(), "bad type!");
        }
    }

    while (!_eol && _char == '*') {
        fulltype += _char;
        inc_intext_pos();
    }

    return fulltype;
}

/* The complete call instruction format is more complicated, we assume it always has the folloing form
 * <result> = [tail | musttail | notail ] call [fast-math flags] [cconv] [ret attrs] <ty>|<fnty> <fnptrval>(<function args>) [fn attrs]
             [ operand bundles ]
 *
 * 'ty':   the type of the call instruction itself which is also the type of the return value.
 *         Functions that return no value are marked void.
 * 'fnty': shall be the signature of the function being called.
 *         The argument types must match the types implied by this signature.
 *         This type can be omitted if the function is not varargs.
 */
void InstParser::do_call_family(Instruction* inst) {
    /* corner cases:
     * %6 = call dereferenceable(272) %"class.std::basic_ostream"* @_ZStlsISt11char_traitsIcEERSt13basic_ostreamIcT_ES5_PKc(%"class.std::basic_ostream"* dereferenceable(272) %4, i8* %5)
     */
    CallInst* ci = (CallInst*)inst;
    if (ci->has_assignment()) {
        get_word('=');
//        get_word();
//        parser_assert(_word[0] == '%', text(), "bad assignment start");
//        inc_intext_pos(2);
    }


    string ret_ty, fnty;


    get_lookahead();
    if (InstFlags::in_tails(_lookahead)) {
        ci->set_raw_field("tail", _lookahead);
        jump_ahead();
    }

    get_word();
    parser_assert(_word == "call", text(), "Bad call inst, _word: |%s| is not |call|", _word.c_str());


    //zpl("word: %s, in: %d", _word.c_str(), InstFlags::in_param_attrs(_word));
    get_lookahead();
    if (InstFlags::in_fastmaths(_lookahead)) {
        ci->set_raw_field("fast-math", _lookahead);
        jump_ahead();
        get_lookahead();
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
    if (_char == '(') {
        string args_sig = jump_to_end_of_scope();

        /* ()* => part of pointer type */
        if (_char == '*') {
            inc_intext_pos(2);
        } // (...) variable args check
        else {
            ci->set_is_varargs();
            parser_assert(Strings::endswith(args_sig, "...)"), text(), "vararg signature should end with '...)'");
            ci->set_raw_field("fnty", args_sig);
            inc_intext_pos();
        }
    }
    else {

    }


    /* Samples
     * %12 = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %11, i8* getelementptr inbounds ([34 x i8], [34 x i8]* @.str.9.134, i64 0, i64 0), i32 %0) #14
     */
    // void (ty..) is for varargs type check
    // void (ty..)[*|**] is a return type, which is a function pointer type
    // Not deal with this for now, ty might also contain '%' as there are global structs

    if (CallInstParsingVerbose) {
        printf( "call:\n"
                "  ret_ty: |%s|\n"
                "  fnty: |%s|\n",
              ret_ty.c_str(), fnty.c_str());
    }

    /* deal with bitcast first if at all
     * corner cases:
     * tail call void bitcast (void (%struct.bContext*, %struct.uiBlock.22475* (%struct.bContext*, %struct.ARegion*, i8*)*, i8*)* @uiPupBlock
     * to void (%struct.bContext*, %struct.uiBlock* (%struct.bContext*, %struct.ARegion*, i8*)*, i8*)*)(%struct.bContext* %C, %struct.uiBlock* (%struct.bContext*, %struct.ARegion*, i8*)* nonnull @wm_enum_search_menu, i8* %0) #3
     */

    if (_char == 'b') {
        get_word();
        guarantee(_word == "bitcast", " ");
        ci->set_has_bitcast();

        inc_intext_pos();
        //parse_function_pointer_type();
        parse_compound_type();
        skip_ws();
        parser_assert(_char == '%' || _char == '@', text(), " ");
    }

    //
    // %15 = call i64 (i8*, i8*, ...) bitcast (i64 (...)* @f90_auto_alloc04 to i64 (i8*, i8*, ...)*)
    // (i8* nonnull %14, i8* bitcast (i32* @.C323_shell_ to i8*))"

    /* direct function call */
    if (_char == '@') {
        inc_intext_pos();
        string fn_name;
        if (ci->has_bitcast()) {
            get_word();
            fn_name = _word;
            get_word();
            parser_assert(_word == "to", text(), " ");
            parse_compound_type();
            inc_intext_pos();
            guarantee(_char == '(', " ");
        }
        else {
            get_word('(', false, false);
            fn_name = _word;
        }

//        if (fn_name.find("f90_") != fn_name.npos) {
//            zpl("kkk")
//        }
        ci->resolve_callee_symbol(fn_name);
    }
    else if (_char == '%') {
        // todo: indirect calls
        inc_intext_pos();
        get_word('(', false, false);
        string label = _word;
        ci->set_is_indirect_call();
        ci->set_called_label(label);
        //zpl("indirect call: %s", inst->raw_c_str());
        if (ci->parent())
            ci->try_resolve_indirect_call();
    }
    else {
        parser_assert(0, _text, "Expect '%%' or '@', char: |%c|, pos: %d", _char, _intext_pos);
    }

    string args = jump_to_end_of_scope();
    ci->set_raw_field("args", args.substr(1, args.size()-2)); // strip the ()

    return;
}



/**
 *
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
    LoadInst* li = dynamic_cast<LoadInst*>(inst);
    guarantee(li, "Not a load inst");
    get_word('=');
    //li->set_name(_word);
    get_word();
    guarantee(_word == "load", "Not a load instruction: %s", inst->raw_c_str());

    get_lookahead();
    if (_lookahead == "volatile") {
        guarantee(0, "check if this is possible");
    }

    /* corner case:
     *    %9 = load i8* (i8*, i32, i32)*, i8* (i8*, i32, i32)** %8, align 8, !dbg !4557, !tbaa !4559
     *    %14 = load void (%class.Base*)**, void (%class.Base*)*** %13, align 8
     */
    get_word(',');
    string full_type = _word;
    // could be function pointer type
    int open_paren_count = std::count(full_type.begin(), full_type.end(), '(');
    int close_paren_count = std::count(full_type.begin(), full_type.end(), ')');

    guarantee(open_paren_count-close_paren_count == 1 || open_paren_count-close_paren_count == 0, "assumption");

    if (open_paren_count-close_paren_count == 1) {
        string first_half = _word + ", ";
        get_word(')');
        guarantee(_char == '*', "Should be function pointer type");
        full_type = first_half + _word + ")*";
        inc_intext_pos(2);
    }

    skip_ws();

    bool ret = match(full_type+'*');
    if (ret == false) {
        zpl("full type: %s", full_type.c_str());
    }
    guarantee(ret, "Load type not match: %s", _text.c_str());
    li->set_ret_type_str(full_type);
    li->set_pointer_type_str(full_type+'*');

    get_word_until(", ");
    if (_word == "getelementptr") {
        // todo: load from array
    }
    else {
        li->set_addr_str(_word);
        get_word();
        guarantee(_word == "align", " ");
        get_word(',');
        li->set_alignment(std::stoi(_word));
        li->set_is_fully_parsed();
    }

    /* TODO: debug info */
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


string InstParser::parse_compound_type() {
    /* corner case
     * tail call void bitcast (void (%struct.bContext*, %struct.uiBlock.22475* (%struct.bContext*, %struct.ARegion*, i8*)*, i8*)* @uiPupBlock
     */
    string ty = parse_basic_type();

    skip_ws();

    if (_char == '(') {
        ty += ' ';
        ty += jump_to_end_of_scope();
        guarantee(_char == '*', "Bad function pointer type: %s", _text.c_str());
        ty += '*';
        inc_intext_pos();

        // could a pointer to a function pointer, like i8* (i8*, i32, i32)** %bzalloc ...
        while (!_eol && _char == '*') {
            ty += '*';
            inc_intext_pos();
        }
    }

    return ty;
}

void InstParser::do_bitcast(Instruction *inst) {
    /* corner case
     *  %1 = bitcast void (...)* bitcast (void (i64*, i32)* @wrf_error_fatal_ to void (...)*) to void (i8*, i64, ...)*
     */
    BitCastInst* I = dynamic_cast<BitCastInst*>(inst);
    const char* op = "bitcast";
    guarantee(I, "Not a %s inst", op);
    get_word('=');
    //I->set_name(_word);
    get_word();
    guarantee(_word == string(op), "Not a %s instruction: %s", op, inst->raw_c_str());

    string old_ty = parse_compound_type();
    I->set_ty_str(old_ty);
    I->set_raw_field("ty", old_ty);
    get_word();

    if (_word == "bitcast") {
        parser_assert(_char == '(', text(), "bitcast should be followed by a ' ('");
        inc_intext_pos();
        string old_old_ty = parse_compound_type();
        get_word();

        I->set_value_str(_word);
        I->set_raw_field("value", _word);
        get_word();
        parser_assert(_word == "to", text(), "expect 'to', got: %s", _word.c_str());

        //todo: the rest of it is not parsed
    }
    else {
        I->set_value_str(_word);
        I->set_raw_field("value", _word);
        get_word();
        parser_assert(_word == "to", text(), "expect 'to', got: %s", _word.c_str());

        string new_ty = parse_compound_type();
        I->set_raw_field("ty2", new_ty);
        I->set_ty2_str(new_ty);
        //zpl("old type: %s, value: %s, new type: %s", old_ty.c_str(), _word.c_str(), new_ty.c_str());
    }

}
