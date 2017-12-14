//
// Created by tlaber on 6/17/17.
//

#ifndef LLPARSER_INSTBUILDER_H
#define LLPARSER_INSTBUILDER_H

#include <utilities/macros.h>
#include <ir/basicBlock.h>

class LLParser;
class Instruction;
class Function;

class GlobalVariable;

class IRBuilder {
public:
    static string get_new_local_varname();
    static string get_new_global_varname();
        
    static Instruction* create_instruction(string& text, LLParser* llparser=NULL);
    static Function* create_function_declaration(string& text, LLParser* llparser=NULL);

    /* Module */
    static GlobalVariable* add_global_string(Module* m, const string& s);
    static CallInst* create_printf_callinst(Module* m, GlobalVariable* gv, const std::vector<string>& args=std::vector<string>());
};

#endif //LLPARSER_INSTBUILDER_H
