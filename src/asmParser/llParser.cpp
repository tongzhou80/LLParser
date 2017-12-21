//
// Created by GentlyGuitar on 6/6/2017.
//


#include <cstdlib>
#include <cassert>
#include <cstring>
#include <iomanip>
#include <peripheral/sysArgs.h>
#include <utilities/mutex.h>
#include <ir/irEssential.h>
#include <di/diEssential.h>
#include <inst/instEssential.h>
#include <passes/passManager.h>
#include <utilities/flags.h>
#include "irBuilder.h"
#include "llParser.h"
#include "instParser.h"
#include "sysDict.h"


// class InstStats

void InstStats::collect_inst_stats(Instruction *ins) {
    _op_count[ins->opcode()]++;

    if (ins->type() != Instruction::UnknownInstType) {
        _parsed++;
    }
}

void InstStats::report() {
    std::cout << "==================== Inst Count ====================\n";
    size_t total = 0;
    for (auto op: _op_count) {
        std::cout << std::setw(20) << op.first << " insts: " << op.second << '\n';
        total += op.second;
    }
    std::cout << std::setw(20) << "parsed" << " insts: " << _parsed << "/" << total << '\n';
    std::cout << "\n";
}


// class LLParser

LLParser::LLParser() {
    initialize();
}

LLParser::~LLParser() {
    delete _inst_parser;
}

void LLParser::set_llvm_version(string v) {
    /* version string should contains 1 or 2 dots, such as 3.8.0 */
    _major_version = v[0] - '0';
    _minor_version = atof((const char*)&v[2]);
}

LLParser::LLParser(const char* file) {
    _file_name = file;
    initialize();
}

void LLParser::initialize() {
    _module = NULL;
    _inst_parser = new InstParser();
}

void LLParser::parse_header(Module* module) {
    assert(_ifs.is_open() && "file not open, can't parse");

    MAX_LINE_LEN = 4096;
    MAX_VALUE_LEN = 1024;
    const char* id_format = "; ModuleID = '%[^']'";
    char* id = new char[MAX_LINE_LEN]();
    int matched = sscanf(line().c_str(), id_format, id);
    if (matched == 1) {
        module->set_module_id(id);
        get_real_line();
    }
    delete[] id;
    guarantee(matched == 1 || matched == 0, "Bad module header\nline %d: %s", _line_number, line().c_str());

    if (matched == 0) {
        fprintf(stderr, "WARNING: %s", "The input file does not start with '; ModuleID = '. It's likely the input file is not in llvm format.\n");
    }

    while (1) {
        const char* header_format = "%[^=]= \"%[^\"]\"";
        char key[MAX_VALUE_LEN];
        char value[MAX_VALUE_LEN];
        memset(key, 0, MAX_VALUE_LEN*sizeof(char));
        memset(value, 0, MAX_VALUE_LEN*sizeof(char));
        int matched1 = sscanf(line().c_str(), header_format, key, value);
        //zpl("matched: %d", matched1);

        if (matched1 == 2) {
            char* striped_key = Strings::strip(key);
            char* striped_value = Strings::strip(value);
            module->set_header(striped_key, striped_value);

            if (strcmp(key, "source_filename") == 0) {
                module->set_name(value);
            }
        }
        else {
            break;
        }
        get_real_line();
    }
}

void LLParser::parse_module_level_asms() {
    while (true) {
        if (Strings::startswith(line(), "module asm")) {
            SysDict::module()->add_module_level_asm(line());
        }
        else {
            break;
        }
        get_real_line();
    }
}

// todo: structs need finer-grain parse
void LLParser::parse_structs(Module* module) {
    while (true) {
        if (line()[0] == '%') {
            StructType* st = new StructType();
            st->set_raw_text(line());
            module->add_struct_type(st);

            /* get name */
            string name = parse_complex_structs();  // name contains '%'
            st->set_name(name.substr(1));

            match('=', true);
            match("type", true);
        }
        else {
            break;
        }
        get_real_line();
    }
}

void LLParser::parse_comdats() {
    Module* module = SysDict::module();
    while (_char == '$') {
        module->set_language(Module::Language::cpp); //todo: not sure if this is a good way to do it
        Comdat* value = new Comdat();
        value->set_raw_text(line());
        module->add_comdat(value);
        get_real_line();
    }
}

/**@brief Try to parse a global variable from the current line
 *
 * Return the new GV* on success
 * Return NULL if the line is not a gv line
 *
 * @param module
 * @return
 */
GlobalVariable* LLParser::parse_global(Module * module) {
    GlobalVariable* gv = new GlobalVariable();
    inc_intext_pos();
    get_word('=');

    Strings::strip(_word);
    gv->set_name(_word);
    parser_assert(!_word.empty(), "");
    get_word();

    // todo: 'alias' does not have to be right after '='
    if (_word == "alias") {
        return NULL;
    }
    gv->set_raw_text(line());
    module->add_global_variable(gv);
    return gv;
}


void LLParser::parse_globals(Module * module) {
    while (line()[0] == '@') {
        GlobalVariable* gv = parse_global(module);
        if (!gv) {
            set_text(_text);
            break;
        }
        get_real_line();
    }
}

void LLParser::parse_aliases() {
    while (line()[0] == '@') {
        Alias* alias = new Alias();

        inc_intext_pos();
        get_word(); // name
        alias->set_name(_word);
        get_word();
        guarantee(_word == "=", " ");
        get_word();
        if (IRFlags::is_linkage_flag(_word)) {
            alias->set_raw_field("linkage", _word);
            get_word();
        }

        parser_assert(_word == "alias", " ");
        string aliasee_ty = parse_compound_type();
        match(',');
        get_lookahead();
        bool has_bitcast = false;
        if (_lookahead == "bitcast") {
            has_bitcast = true;
            jump_ahead();
            match('(');
            string old_ty = parse_compound_type();

        }
        else {
            string aliasee_ty_p = parse_compound_type();
            parser_assert(aliasee_ty_p == aliasee_ty + '*', "sanity check");
        }

        match('@');
        get_word();
        alias->set_raw_field("aliasee", _word);
        if (has_bitcast) {
            match("to");
            parse_compound_type();
            match(')');
        }

        parser_assert(_eol, "should be end of line");

        alias->set_raw_text(line());
        SysDict::module()->add_alias(alias->name(), alias);
        get_real_line();
    }
}

void LLParser::parse_functions() {
    while (true) {
        if (!_ifs) {
            if (!NoParserWarning) {
                fprintf(stderr, "WARNING: Reached end-of-file during function parsing\n");
            }
            break;
        }
        // should either start with 'declare' or 'define'
        guarantee(!line().empty(), "");
        if (line()[2] == 'f') {
            parse_function_definition();
        }
        else if (line()[2] == 'c') {
            parse_function_declaration();
        }
        else {
            break;
        }

        get_real_line(); // consume one more line, so the current line becomes a new function header or the next block
    }
}

/*
 * Function Syntax
 * define [linkage] [visibility] [DLLStorageClass]
       [cconv] [ret attrs]
       <ResultType> @<FunctionName> ([argument list])
       [(unnamed_addr|local_unnamed_addr)] [fn Attrs] [section "name"]
       [comdat [($name)]] [align N] [gc] [prefix Constant]
       [prologue Constant] [personality Constant] (!name !N)* { ... }

 * Parameter Syntax
 * <type> [parameter Attrs] [name]
 */
/**@brief Parse a string that contains a function's name and args, return a Function* with no parent
 *
 * @return
 */
Function* LLParser::parse_function_name_and_args() {
    /* To parse all args we must parse types, leave as todo */
    guarantee(_char == '@', "Bad function header");
    inc_inline_pos();

    get_word('(', false, false);
    string name = _word;
    string params = jump_to_end_of_scope();  // todo: parse params
    int dbg = -1;

    // fast-forward to !dbg
    int pos = text().find("!dbg");
    if (pos != string::npos) {
        set_intext_pos(pos+strlen("!dbg !"));
        get_word();
        dbg = std::stoi(_word);
    }

    if (FunctionParsingVerbose) {
        printf("  name: |%s|\n"
               "  args: |%s|\n",
               name.c_str(), params.c_str());
    }

    Function* func = SysDict::module()->get_function(name);

    if (func != NULL) {
        guarantee(0, " ");
//        //module->append_function(func);
//        SysDict::module()->set_as_resolved(func);
//        guarantee(!func->is_external(), "redeclare function %s\nline %d: %s", name.c_str(), _line_number, line().c_str());
//        guarantee(!func->is_defined(), "redefine function %s\nline %d: %s", name.c_str(), _line_number, line().c_str());
    }
    else {
        //func = SysDict::module()->create_child_function(name);
        func = new Function();
        func->set_name(name);
    }

    if (dbg > -1) {
        func->set_dbg_id(dbg);
    }

    return func;
}

/* sscanf does not deal with empty field, so parse it manually
 *
 * Function Syntax
 * define [linkage] [visibility] [DLLStorageClass]
       [cconv] [ret attrs]
       <ResultType> @<FunctionName> ([argument list])
       [(unnamed_addr|local_unnamed_addr)] [fn Attrs] [section "name"]
       [comdat [($name)]] [align N] [gc] [prefix Constant]
       [prologue Constant] [personality Constant] (!name !N)* { ... }

 * Parameter Syntax
 * <type> [parameter Attrs] [name]
 */
Function* LLParser::parse_function_header() {
    Function* func = create_function(line());
    return func;
}

/**@brief Parse the function header and return a Function pointer
 *
 * Basic blocks are not parsed yet.
 *
 * @param text
 * @return
 */
Function* LLParser::create_function(string &text) {
    int len = text.length();
    if (text[len-1] == '{') {
        len--;
    }
    if (text[len-1] == ' ') {
        len--;
    }
    Function* func = new Function();
    set_line(text.substr(0, len));
    get_word();
    if (_word == "declare") {
        func->set_is_external();
    }
    else if (_word == "define") {
        func->set_is_defined();
    }
    else {
        guarantee(0, "function starts with neither declare nor define");
    }
    string linkage, cconv, ret_attrs, ret_type;


    /* process optional flags */
    set_linkage(func);
    set_visibility(func);
    set_dll_storage_class(func);
    set_cconv(func);
    set_ret_attrs(func);



    /* A function pointer return type looks like this:
     *   declare void (i32)* @signal(i32, void (i32)*) local_unnamed_addr #3
     * Not parse this return type for now
     */
    ret_type = parse_compound_type();
    skip_ws();

    parser_assert(_char == '@', "Bad function header");
    inc_inline_pos();

    get_word('(', false, false);
    string name = _word;
    func->set_name(name);
    string params = jump_to_end_of_scope();  // todo: parse params
    int dbg = -1;

    // fast-forward to !dbg
    int pos = this->text().find("!dbg");
    if (pos != string::npos) {
        set_intext_pos(pos+strlen("!dbg !"));
        get_word();
        dbg = std::stoi(_word);
        func->set_dbg_id(dbg);
    }

    if (CheckDebugInfo && func->is_defined()) {
        parser_assert(dbg != -1, "should have debug info");
    }

    if (FunctionParsingVerbose) {
        printf("  name: |%s|\n"
                       "  args: |%s|\n",
               name.c_str(), params.c_str());
    }

    func->set_raw_text(line());
    return func;
}

void LLParser::parse_function_definition() {
    Function* func = parse_function_header();
    SysDict::module()->append_new_function(func);
    get_real_line();
    /* parse basic blocks */
    while (1) {
        BasicBlock* bb = func->create_basic_block();
        if (func->basic_block_num() == 1) {
            bb->set_is_entry();
            func->set_entry_block(bb);
        }
        parse_basic_block(bb);

        if (UseLabelComments) {
            getline_nonempty();
        }
        else {
            get_real_line();
        }

        if (line()[0] == '}') {
            bb->set_is_exit();
            break;
        }
    }
}

void LLParser::parse_basic_block_header(BasicBlock *bb, bool use_comment) {
    bb->set_raw_text(line());

    // get label
    const string label_start = "; <label>:";
    if (use_comment) {
        inc_inline_pos(label_start.size());
        get_word();
        bb->set_name(_word);
    }
    else {
        get_word(':');
        bb->set_name(_word);
    }

    if (!Strings::contains(line(), "; preds")) {
        return;
    }

    // get preds, if any
    get_word('%');
    get_word(',');
    bb->append_pred(_word);
    while (!_eol) {
        get_word('%');
        get_word(',');
        bb->append_pred(_word);
    }
}

void LLParser::remove_tail_comments() {
    int pos = line().find(';');
    set_line(line().substr(0, pos));
}

void LLParser::parse_basic_block(BasicBlock* bb) {
    while (1) {
        parser_assert(line()[0] != '}', "Not a block");

        /* parse header first */
        if (line()[0] == ';') {
            parse_basic_block_header(bb, true);
            get_real_line();
        }
        else if (line()[0] != ' ' && Strings::contains(line(), ":")) {
            parse_basic_block_header(bb);
            get_real_line();
        }
        else {
            // assume starting with a blank space means a instruction line
            guarantee(line()[0] == ' ', "unrecognized block sentence: %s", line().c_str());
            //bb->set_raw_text(line());
            //get_real_line();
        }

//        /* parse header first */
//        if (Strings::startswith(line(), "; <label>:")) {
//            parse_basic_block_header(bb);
//            get_real_line();
//        }
//        else if (line()[0] != ' ' && Strings::contains(line(), ":")) {
//            parse_basic_block_header(bb);
//            get_real_line();
//        }
//        else {
//            // assume starting with a blank space means a instruction line
//            guarantee(line()[0] == ' ', "unrecognized block sentence: %s", line().c_str());
//            //bb->set_raw_text(line());
//            //get_real_line();
//        }

        set_line_to_full_instruction();

        remove_tail_comments();

        if(line().find_first_not_of(' ') == std::string::npos) {
            getline_nonempty();
            continue;
        }

//
//        int dbg_pos = line().find(", !dbg");
//        string dbg_text;
//        if (dbg_pos != string::npos) {
//            dbg_text = line().substr(dbg_pos);
//            set_line(line().substr(0, dbg_pos));
//        }

        Instruction* inst = parse_instruction_line(bb);

//        if (dbg_pos != string::npos) {
//            set_line(dbg_text);
//            parse_debug_info(inst);
//            inst->append_raw_text(dbg_text);
//        }

        string opcode = inst->opcode();

        //zpl("opcode: %s", opcode.c_str())
        if (IRFlags::is_terminator_inst(opcode)) {
            break;
        }

        get_real_line();
    }
}

/* some instruction takes more than one line */
void LLParser::set_line_to_full_instruction() {
//    if (line().find("switch i8 %phitmp") != line().npos) {
//        zpl("%s", line().c_str());
//        exit(0);
//    }
    string full = line();
    if (Strings::endswith(line(), "[") && Strings::startswith(line(), "switch")) {
        do {
            full += '\n';
            get_real_line();
            full += line();
        } while (!Strings::startswith(line(), "]"));
    }
    else if (Strings::contains(line(), " invoke ")) {
        full += '\n';
        get_real_line();
        parser_assert(Strings::startswith(line(), "to"), "invalid invoke inst");
        full += line();
    }
    set_line(full);
}

void LLParser::parse_debug_info(Instruction* inst) {
    /* there could be some other metadata following the !dbg such as
     * br label %9, !dbg !313, !llvm.loop !314
     */
    string head = ", !dbg !";
    guarantee(Strings::startswith(line(), head), "Bad debug info");
    inc_inline_pos(head.size());
    get_word(',');
    inst->set_dbg_id(std::stoi(_word));
}

Instruction* LLParser::parse_instruction_line(BasicBlock *bb) {
    // inst will be appended to bb
    Instruction* inst = IRBuilder::create_instruction(line(), this);
    bb->append_instruction(inst);  // now parsing the instruction shouldn't need bb's info (data flow)
    inst->set_owner(bb->parent()->name());  // todo: only for debug use, this _owner will not change when the owner's name changes

#ifdef LLDEBUG
    //_stats.collect_inst_stats(inst);
#endif
    if (InstructionParsingVerbose) {
        printf("Inst line raw: %s\n", line().c_str());
    }

    return inst;
}

//Instruction* LLParser::parse_instruction_line(BasicBlock *bb, string op) {
//    Instruction* inst = SysDict::instParser->parse(_line);
//    if (InstructionParsingVerbose) {
//        printf("Inst line raw: %s\n", line().c_str());
//    }
//
//    bb->append_instruction(inst);
//
//    return inst;
//}


void LLParser::parse_function_declaration() {
    Function* func = parse_function_header();
    SysDict::module()->append_new_function(func);
}

void LLParser::parse_attributes(Module *module) {
    if (SysArgs::get_flag("debug-info")) {
        module->unnamed_metadata_list().reserve(4096);
    }

    while (_ifs && Strings::startswith(line(), "attributes")) {
        Attribute* attr = new Attribute();
        module->append_attribute(attr);
        attr->set_raw_text(line());

        get_real_line();
    }

}

void LLParser::parse_metadatas(Module *module) {
    while (_ifs && line()[0] == '!') {
        inc_inline_pos();
        get_word();
        MetaData* data;
        if (!Strings::is_number(_word)) {
            data = new MetaData();
            module->set_named_metadata(_word, data);
        }
        else {
            if (!UseSplitModule) {
                guarantee(std::stoi(_word) == module->unnamed_metadata_list().size(), "bad dbg id numbering");
            }

            get_word('!');
            if (_char == '{') {
                data = new MetaData();
            }
            else {
                get_word('(');
                if (_word == "DIFile") {
                    data = new DIFile();
                    parse_di_fields(data);
                    //data = parse_difile();
                }
                else if (_word == "DISubprogram") {
                    //data = parse_disubprogram();
                    data = new DISubprogram();
                    parse_di_fields(data);
                }
                else if (_word == "DILexicalBlock") {
                    //data = parse_dilexicalblock();
                    data = new DILexicalBlock();
                    parse_di_fields(data);
                }
                else if (_word == "DILexicalBlockFile") {
                    //data = parse_dilexicalblockfile();
                    data = new DILexicalBlockFile();
                    parse_di_fields(data);
                }
                else if (_word == "DILocation") {
                    //data = parse_dilocation();
                    data = new DILocation();
                    parse_di_fields(data);
                }
                else {
                    data = new MetaData();
                }
            }

            data->set_number(module->unnamed_metadata_list().size());
            module->append_unnamed_metadata(data);
        }

        data->set_raw_text(line());
        data->set_parent(module);
        data->resolve_non_refs();
        get_real_line();
    }

    guarantee(_ifs.eof(), "should be end of file, line: %d", line_number());
    //zpl("Module %s: end of file, total: %d", module->name_as_c_str(), _line_number);
}

void LLParser::parse_di_fields(MetaData* data)  {
    while (!_eol) {
        get_word_of(",)");
        int colon_pos = _word.find(':');
        string key = _word.substr(0, colon_pos);
        string value = _word.substr(colon_pos+2);
        data->set_raw_field(key, value);
    }
}

Module* LLParser::parse(string file) {
    reset_parser();
    _file_name = file;
    return parse();
}

void LLParser::resolve() {

}

void LLParser::print_stats() {
    _stats.report();
}

Module* LLParser::parse() {
/* 1. the parser always read in one line ahead, namely _line
 *    the next parsing phase will start from _line
 * 2. empty line is always skipped since they should not have meaning
 */
    _ifs.open(_file_name.c_str());
    if (!_ifs.is_open()) {
        fprintf(stderr, "open file %s failed.\n", _file_name.c_str());
        return NULL;
    }

    _module = new Module();
    _module->set_input_file(filename());
    SysDict::add_module(module());


    getline_nonempty();
    parse_header(module());
    parse_module_level_asms();
    parse_structs(module());
    parse_comdats();
    parse_globals(module());
    parse_aliases();
    parse_functions();
    parse_attributes(module());
    parse_metadatas(module());

    if (UseSplitModule) {
        return module();
    }

    // DILocation is slightly more complicated, so resolve some data in advance
    // Update: now resolve all types of DIXXX
    module()->resolve_after_parse();
    module()->check_after_parse();

#ifndef PRODUCTION
    /* perform post check */
    //SysDict::module()->check_after_parse();
#endif
    PassManager* pm = PassManager::pass_manager;
    pm->apply_passes(module());

    return module();
}


