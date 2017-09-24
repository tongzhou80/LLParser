//
// Created by tlaber on 6/10/17.
//

#ifndef LLPARSER_INSTPARSER_H
#define LLPARSER_INSTPARSER_H

#include <ir/irEssential.h>
#include <utilities/macros.h>
#include <peripheral/stringParser.h>
#include "irParser.h"

typedef Instruction* (*inst_parse_routine) (string&, bool);

class BitCastInst;

class InstParser: public IRParser {

    //static std::map<string, inst_parse_routine> _table;
public:
    InstParser();

    Instruction* parse(string& );
    void parse(Instruction* inst);


    void parse_function_pointer_type();
    Instruction* create_instruction(string &text);
    void parse_metadata(Instruction* ins);

    /* the do_xxx functions will not check dynamic_cast */
    Instruction* do_alloca(bool has_assignment=1);
    void do_branch(Instruction* inst);
    void do_call_family(Instruction* inst);
    void do_load(Instruction* inst);
    void do_bitcast(Instruction* inst);

    BitCastInst* parse_inline_bitcast();


    //static void parse_instruction(Instruction** ip);
};

#endif //LLPARSER_INSTPARSER_H
