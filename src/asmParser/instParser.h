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

    Instruction* parse(string& );
    void parse(Instruction* inst);


    void parse_function_pointer_type();
    Instruction* create_instruction(string &text);
    void parse_metadata(Instruction* ins);

    /* the do_xxx functions will not check dynamic_cast */
    void do_alloca(Instruction* ins);
    void do_branch(Instruction* ins);
    void do_call_family(Instruction* ins);
    void do_load(Instruction* ins);
    void do_store(Instruction* ins);
    void do_bitcast(Instruction* ins, bool is_embedded=false);
    void do_getelementptr(Instruction* ins, bool is_embedded=false);


    //static void parse_instruction(Instruction** ip);
};

#endif //LLPARSER_INSTPARSER_H
