//
// Created by GentlyGuitar on 6/6/2017.
//


#include <cstdlib>
#include <cassert>
#include <cstring>
#include <peripheral/sysArgs.h>
#include <utilities/mutex.h>
#include <peripheral/timer.h>
//#include <peripheral/sysArgs.h>
#include "llParser.h"
#include "instParser.h"
#include "sysDict.h"
#include "ir/irEssential.h"
#include "ir/di/diEssential.h"
#include "inst/instEssential.h"
#include "passes/passManager.h"
#include "utilities/flags.h"
#include "irBuilder.h"


LLParser::LLParser() {
    initialize();
}

LLParser::~LLParser() {
    if (_inst_parser) {
        delete _inst_parser;
    }
}

/* version string should contains 1 or 2 dots, such as 3.8.0 */
void LLParser::set_llvm_version(string v) {
    _major_version = v[0] - '0';
    _minor_version = atof((const char*)&v[2]);
}

LLParser::LLParser(const char* file) {
    _file_name = file;
    initialize();
}

void LLParser::initialize() {
    //set_llvm_version("5.0.0");
    set_llvm_version("3.8.0");
    _asm_format = 0; // normal file produced by clang -S -emit-llvm

    _has_structs = false;
    _has_globals = false;
    _unresolved = 0;

    MAX_LINE_LEN = 4096;
    MAX_VALUE_LEN = 1024;
    _line_number = 0;

    _module = NULL;
    _inst_parser = new InstParser();
}

void LLParser::set_done(bool v) {
    Locks::llparser_done_lock->lock();
    _done = v;
    Locks::llparser_done_lock->unlock();
}

bool LLParser::is_done() {
    Locks::llparser_done_lock->lock();
    bool ret = _done;
    Locks::llparser_done_lock->unlock();
    return ret;
}

void LLParser::parse_header(Module* module) {
    assert(_ifs.is_open() && "file not open, can't prase");

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
//        if (Strings::startswith(line(), "%struct") || Strings::startswith(line(), "%union")) {
//            _has_structs = true;
//
//            StructType* st = new StructType();
//            st->set_raw_text(line());
//            module->add_struct_type(st);
//        }
//        else if (Strings::startswith(line(), "%")) {
        if (Strings::startswith(line(), "%")) {
            StructType* st = new StructType();
            st->set_raw_text(line());
            module->add_struct_type(st);

            /* get name */
            inc_inline_pos();
            /* those starting with %"xxxxx" =  */
            get_word('=');
            Strings::strip(_word);
//            parser_assert(Strings::startswith(_word, "struct") ||
//                              Strings::startswith(_word, "union") ||
//                              Strings::startswith(_word, "class"), line(), "Invalid struct: %s", line().c_str());
            st->set_name(_word);

            get_word();
            parser_assert(_word == "type", line(), "Invalid struct syntax");
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

void LLParser::parse_globals(Module * module) {
    while (Strings::startswith(line(), "@")) {
        _has_globals = true;
        GlobalVariable* gv = new GlobalVariable();
        inc_intext_pos();
        get_word('=');
        Strings::strip(_word);
        gv->set_name(_word);
        get_word();

        // todo: 'alias' does not have to be right after '='
        if (_word == "alias") {
            set_text(_text);
            break;
        }
        gv->set_raw_text(line());
        module->add_global_variable(gv);
        get_real_line();
    }
}

void LLParser::parse_aliases() {
    while (Strings::startswith(line(), "@")) {
        Alias* alias = new Alias();

        inc_intext_pos();
        get_word(); // name
        alias->set_name(_word);
        get_word();
        guarantee(_word == "=", " ");
        get_word();
        if (InstFlags::in_linkages(_word)) {
            alias->set_raw_field("linkage", _word);
        }

        parser_assert(_word == "alias", text(), " ");
        get_word('@'); // skip to @
        get_word();
        alias->set_raw_field("aliasee", _word);
        parser_assert(_eol, line(), "should be end of line");

        alias->set_raw_text(line());
        SysDict::module()->add_alias(alias->name(), alias);
        get_real_line();
    }
}

/* A function either starts with "declare" or "define"
 */
void LLParser::parse_functions(Module * module) {
    int counter = 0;
    while (true) {
//        if (Strings::conatins(line(), "bzlib")) {
//            zpl("cc %s", line().c_str());
//        }
        if (Strings::startswith(line(), "define")) {
            parse_function_definition(module);
        }
        else if (Strings::startswith(line(), "declare")) {
            parse_function_declaration(module);
        }
        else {
            break;
        }

        get_real_line(); // consume one more line, so the current line becomes a new function header or the next block
    }
}

/* This function parses a string like "@BZ2_blockSort(%struct.EState* nocapture)" */
Function* LLParser::parse_function_name_and_args() {
    /* To parse all args we must parse types, leave as todo */
//    if (Strings::conatins(line(), "BZ2_bzlibVersion")) {
//        printf("hh");
//    }
    guarantee(_char == '@', "Bad function header");
    inc_inline_pos();

    get_word('(');
    string name = _word;
    string args = "";  // todo: parse args
    int dbg = -1;

    // fast-forward to !dbg
    get_word('!');
    if (!_eol) {
        get_word('!');
        guarantee(_word == "dbg ", "! not followed by dbg: %s", _word.c_str());
        get_word(' ');
        dbg = std::stoi(_word);
    }

    if (FunctionParsingVerbose) {
        printf("  name: |%s|\n"
               "  args: |%s|\n",
               name.c_str(), args.c_str());
    }

    guarantee(!Strings::conatins(args, "("), "args should not contain '(': %s", args.c_str());

    Function* func = SysDict::module()->get_function(name);

    if (func != NULL) {
        //module->append_function(func);
        SysDict::module()->set_as_resolved(func);
        guarantee(!func->is_external(), "redeclare function %s\nline %d: %s", name.c_str(), _line_number, line().c_str());
        guarantee(!func->is_defined(), "redefine function %s\nline %d: %s", name.c_str(), _line_number, line().c_str());
    }
    else {
        //func = SysDict::module()->create_child_function(name);
        func = new Function();
        func->set_name(name);

        guarantee(func != NULL, "no");
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
Function* LLParser::parse_function_header(Module* module) {
    Function* func = create_function(line());

    if (func->parent() == NULL) {
        SysDict::module()->append_new_function(func);
    }
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
    set_line(text);
    get_word();
    string declare_or_define = _word;
    string linkage, cconv, ret_attrs, ret_type;

    get_word();
    /* process optional flags */
    if (InstFlags::in_linkages(_word)) {
        linkage = _word;
        get_word();
    }

    if (InstFlags::in_cconvs(_word)) {
        cconv = _word;
        get_word();
    }

    while (InstFlags::in_param_attrs(_word)) {
        ret_attrs += _word;
        get_word();
    }

    ret_type = _word;

    /* A function pointer return type looks like this:
     *   declare void (i32)* @signal(i32, void (i32)*) local_unnamed_addr #3
     * Not parse this return type for now
     */
    ret_type = "";

    // call for todo
    while (_char != '@') {
        inc_inline_pos();
    }

    if (FunctionParsingVerbose) {
        printf("%s function:\n", declare_or_define.c_str());
    }

    /* Now we can determine which function it is */
    Function* func = parse_function_name_and_args();
    func->set_raw_text(line());

    if (declare_or_define[2] == 'c') {
        func->set_is_external();
    }
    else if (declare_or_define[2] == 'f') {
        func->set_is_defined();
    }
    else {
        guarantee(0, "function starts with neither declare nor define");
    }

    return func;
}

void LLParser::parse_function_definition(Module * module) {
    Function* func = parse_function_header(module);
    guarantee(line()[line().size()-1] == '{', "function definition should end with a '{'");

    get_real_line();
    bool first_bb = true;
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

        if (Strings::startswith(line(), "}")) {
            break;
        }
    }


//    if (UseParseTimePasses) {
//        PassManager::pass_manager->apply_parse_time_passes(func);
//    }
}

void LLParser::parse_basic_block_header(BasicBlock *bb) {
    bb->set_raw_text(line());

    // get label
    string label_start = "; <label>:";
    if (Strings::startswith(line(), label_start)) {
        inc_inline_pos(label_start.size());
        get_word();
        bb->set_name(_word);
    }
    else {
        get_word(':');
        bb->set_name(_word);
    }

    if (!Strings::conatins(line(), "; preds")) {
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
        parser_assert(!Strings::startswith(line(), "}"), line(), "Not a block");

        /* parse header first */
        if (Strings::startswith(line(), "; <label>:")) {
            parse_basic_block_header(bb);
            get_real_line();
        }
        else if (line()[0] != ' ' && Strings::conatins(line(), ":")) {
            parse_basic_block_header(bb);
            get_real_line();
        }
        else {
            // assume starting with a blank space means a instruction line
            guarantee(line()[0] == ' ', "unrecognized block sentence: %s", line().c_str());
            //bb->set_raw_text(line());
            //get_real_line();
        }

        set_line_to_full_instruction();

        remove_tail_comments();

        if(line().find_first_not_of(' ') == std::string::npos) {
            getline_nonempty();
            continue;
        }

        int dbg_pos = line().find(", !dbg");
        string dbg_text;
        if (dbg_pos != string::npos) {
            dbg_text = line().substr(dbg_pos);
            set_line(line().substr(0, dbg_pos));
        }

        Instruction* inst = parse_instruction_line(bb);

        if (dbg_pos != string::npos) {
            set_line(dbg_text);
            parse_debug_info(inst);
            inst->append_raw_text(dbg_text);
        }

        string opcode = inst->opstr();


//        if (UseParseTimePasses) {
//            PassManager::pass_manager->apply_parse_time_passes(inst);
//        }

        if (InstFlags::in_terminator_insts(opcode)) {
            break;
        }

        get_real_line();
        //getline_nonempty();
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
    else if (Strings::conatins(line(), " invoke ")) {
        full += '\n';
        get_real_line();
        parser_assert(Strings::startswith(line(), "to"), line(), "invalid invoke inst");
        full += line();
    }
    set_line(full);
}

void LLParser::parse_debug_info(Instruction* inst) {
    string head = ", !dbg !";
    guarantee(Strings::startswith(line(), head), "Bad debug info");
    inc_inline_pos(head.size());
    get_word(',');
    inst->set_dbg_id(std::stoi(_word));
}

Instruction* LLParser::parse_instruction_line(BasicBlock *bb) {
    // inst will be appended to bb
    Instruction* inst = IRBuilder::create_instruction(line(), bb);
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


void LLParser::parse_function_declaration(Module * module) {
    Function* func = parse_function_header(module);

//    if (UseParseTimePasses) {
//        PassManager::pass_manager->apply_parse_time_passes(func);
//    }
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
    while (_ifs && Strings::startswith(line(), "!")) {
        inc_inline_pos();
        get_word();
        MetaData* data;
        if (!Strings::is_number(_word)) {
            data = new MetaData();
            module->set_named_metadata(_word, data);
        }
        else {
            guarantee(std::stoi(_word) == module->unnamed_metadata_list().size(), "bad dbg id numbering");

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
        get_real_line();
    }

    guarantee(_ifs.eof(), "should be end of file");
    //zpl("Module %s: end of file, total: %d", module->name_as_c_str(), _line_number);
}

void LLParser::parse_di_fields(MetaData* data)  {
    while (!_eol) {
        get_word_until(",)");
        int colon_pos = _word.find(':');
        string key = _word.substr(0, colon_pos);
        string value = _word.substr(colon_pos+2);
        data->set_raw_field(key, value);
    }
}

DIFile* LLParser::parse_difile() {
    /* !DIFile(filename: "specrand.c", directory: "/home/tlaber/CLionProjects/LLParser/benchmarks/cpu2006/libquantum/src") */
    DIFile* data = new DIFile();
    const int nfields = 2;
    const char* fields[nfields] = {"filename", "directory"};
    for (int i = 0; i < nfields; ++i) {
        get_word(':');
        guarantee(_word == fields[i], "Bad DIFile format");
        get_word('"');
        get_word('"');
        if (i == 0) {
            data->set_filename(_word);
        }
        else if ( i ==1 ) {
            data->set_directory(_word);
        }
        get_word(',');
    }
    return data;
}

DISubprogram* LLParser::parse_disubprogram() {
    /* distinct !DISubprogram(name: "spec_rand", scope: !1200, file: !1200, line: 25, ...) */

    DISubprogram* data = new DISubprogram();
    while (!_eol) {
        get_word_until(",)");
        int colon_pos = _word.find(':');
        string key = _word.substr(0, colon_pos);
        string value = _word.substr(colon_pos+2);

        data->set_raw_field(key, value);
    }
    return data;
}
//
//DILocation* LLParser::parse_dilocation() {
//    /* !1225 = !DILocation(line: 42, column: 3, scope: !1222) */
//    /* !2404 = !DILocation(line: 0, scope: !258) */
//    DILocation* data = new DILocation();
//
//    while (!_eol) {
//        get_word_until(",)");
//        int colon_pos = _word.find(':');
//        string key = _word.substr(0, colon_pos);
//        string value = _word.substr(colon_pos+2);
//        if (value[0] == '!') {
//            value = value.substr(1);
//            guarantee(Strings::is_number(value), "! should be followed by a number!");
//        }
//
//        if (key == "line") {
//            data->set_line(std::stoi(value));
//        }
//        else if (key == "column") {
//            data->set_column(std::stoi(value));
//        }
//        else if (key == "scope") {
//            data->set_scope_id(std::stoi(value));
//        }
//        else if (key == "inlinedAt") {
//            data->set_inlined_pos(std::stoi(value));
//        }
//        else {
//            parser_assert(0, line(), "Bad key: %s", key.c_str());
//        }
//    }
////    get_word(':');
////    parser_assert(_word == "name", line(), "Bad DILocation format");
////    get_word_until(",)");
////    data->set_line(std::stoi(_word));
////
////    get_word(':');
////    if (_word == "column") {
////        get_word_until(",)");
////        data->set_column(std::stoi(_word));
////        get_word(':');
////    }
////
////    parser_assert(_word == "scope", line(), "Bad DILocation format");
////    get_word_until(",)");
////    data->set_scope_id(std::stoi(_word.substr(1)));
//
////    const int nfields = 3;
////    const char* fields[nfields] = { "line", "column", "scope" };
////    for (int i = 0; i < nfields; ++i) {
////        get_word(':');
////        guarantee(_word == fields[i], "Bad DILocation format");
////        get_word_until(",)");
////
////        if (i == 0) {
////            data->set_line(std::stoi(_word));
////        }
////        else if (i == 1) {
////            data->set_column(std::stoi(_word));
////        }
////        else if (i == 2) {
////            data->set_scope_id(std::stoi(_word.substr(1)));
////        }
////    }
//    if (!_eol) {
//        get_word(':');
//        if (_word == "inlinedAt") {
//            get_word(')');
//            data->set_inlined_pos(std::stoi(_word.substr(1)));
//        }
//    }
//
//    return data;
//}


Module* LLParser::parse(string file) {
    reset_parser();
    set_done(false);
    _file_name = file;
    return parse();
}


/* 1. the parser always read in one line ahead, namely _line
 *    the next parsing phase will start from _line
 * 2. empty line is always skipped since they should not have meaning
 */
Module* LLParser::parse() {
    _ifs.open(_file_name.c_str());
    if (!_ifs.is_open()) {
        fprintf(stderr, "open file %s failed.\n", _file_name.c_str());
        return NULL;
    }


    _module = new Module();
    SysDict::add_module(this);

    getline_nonempty();
    parse_header(module());
    parse_module_level_asms();
    parse_structs(module());
    parse_comdats();
    parse_globals(module());
    parse_aliases();
    parse_functions(module());
    parse_attributes(module());
    parse_metadatas(module());


    // DILocation is slightly more complicated, so resolve some data in advance
    // Update: now resolve all types of DIXXX
    SysDict::module()->resolve_debug_info();
    SysDict::module()->resolve_aliases();

    SysDict::module()->check_after_parse();

    Locks::pass_manager_lock->lock();
    PassManager* pm = PassManager::pass_manager;
    pm->apply_passes(module());

    pm->apply_initializations(module());

    for (auto F: module()->function_list()) {
        for (auto B: F->basic_block_list()) {
            pm->apply_passes(B);
        }
        pm->apply_passes(F);
    }

    pm->apply_finalization(module());
    Locks::pass_manager_lock->unlock();

    /* perform post check */
    return module();
}


