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

class IRBuilder {
public:
    static Instruction* create_instruction(string& text, LLParser* llparser=NULL);
    static Function* create_function_declaration(string& text, LLParser* llparser=NULL);
};

#endif //LLPARSER_INSTBUILDER_H
