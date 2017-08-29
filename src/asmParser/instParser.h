//
// Created by tlaber on 6/10/17.
//

#ifndef LLPARSER_INSTPARSER_H
#define LLPARSER_INSTPARSER_H

#include <ir/irEssential.h>
#include <utilities/macros.h>
#include <peripheral/stringParser.h>

typedef Instruction* (*inst_parse_routine) (string&, bool);

class InstParser: public StringParser {

    //static std::map<string, inst_parse_routine> _table;
public:
    InstParser();

    Instruction* parse(string& );
    void parse(Instruction* inst);
    string get_opcode(string& );
    string get_first_word(string& );
    string parse_basic_type();
    string parse_compound_type();
    void parse_function_pointer_type();
    void fastforwad_bitcast();

    Instruction* do_alloca(bool has_assignment=1);
    void do_call(Instruction* inst);
    void do_call_family(Instruction* inst);
    void do_load(Instruction* inst);
    void do_bitcast(Instruction* inst);

    static Instruction* create(string& text);

    //static void parse_instruction(Instruction** ip);
};

#endif //LLPARSER_INSTPARSER_H
