//
// Created by tzhou on 9/18/17.
//

#ifndef LLPARSER_CONTEXTGENERATOR_H
#define LLPARSER_CONTEXTGENERATOR_H

#include <ir/module.h>

class ContextGenerator {
public:
    void generate(Module* module, string alloc="malloc", int nlevel=1);
};

#endif //LLPARSER_CONTEXTGENERATOR_H
