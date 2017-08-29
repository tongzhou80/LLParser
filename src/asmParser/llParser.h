//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_LLPARSER_H
#define LLPARSER_LLPARSER_H

#include <fstream>
#include <peripheral/FileParser.h>
#include "ir/basicBlock.h"
#include "ir/module.h"
#include "utilities/symbol.h"
#include "utilities/macros.h"
#include "instParser.h"

class Module;
class StructType;
class Function;
class BasicBlock;

class DIFile;
class DISubprogram;
class DILocation;
class DILexicalBlock;
class DILexicalBlockFile;

class InstParser;

class SysDict;

class LLParser: public FileParser {
    int _major_version;
    float _minor_version;
    int _asm_format;
    bool _has_structs;
    bool _has_globals;
    int _unresolved;  // The number of unresolved functions
    bool _debug_info;
    Module* _module;
    InstParser* _inst_parser;
public:
    int MAX_LINE_LEN;
    int MAX_VALUE_LEN;

    LLParser();
    ~LLParser();
    LLParser(const char* file);
    void initialize();

    Module* module()                                                        { return _module; }
    InstParser* inst_parser()                                               { return _inst_parser; }
    void inc_inline_pos(int steps=1)                                        { inc_intext_pos(steps); }
    void set_line(string l)                                                 { set_text(l); }

    void set_llvm_version(string v);
    Module* parse();
    Module* parse(string file);
    void parse_header(Module* );
    void parse_module_level_asms();
    void parse_structs(Module* );
    void parse_comdats();
    void parse_globals(Module* );
    void parse_functions(Module* );
    void parse_function_declaration(Module* );
    void parse_function_definition(Module* );
    Function* parse_function_header(Module* );
    Function* create_function(string& text);
    Function* parse_function_name_and_args();
    void parse_basic_block(BasicBlock* bb);
    void parse_basic_block_header(BasicBlock* bb);
    Instruction* parse_instruction_line(BasicBlock* bb);
    void parse_instruction_table(BasicBlock* bb, string op="");
    void parse_attributes(Module* module);
    void parse_metadatas(Module* module);
    DIFile* parse_difile();
    DISubprogram* parse_disubprogram();
    DILexicalBlock* parse_dilexicalblock();
    DILexicalBlockFile* parse_dilexicalblockfile();
    DILocation* parse_dilocation();
    void parse_di_fields(MetaData* data);
    void parse_debug_info(Instruction* );
    void parse_function_pointer_type();


    void set_line_to_full_instruction();
    void remove_tail_comments();
};







#endif //LLPARSER_LLPARSER_H
