//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_GLOBALVARIABLE_H
#define LLPARSER_GLOBALVARIABLE_H

#include "value.h"
#include "shadow.h"

class GlobalVariable: public Value{
public:
    GlobalVariable(): Value() {}
};

#endif //LLPARSER_GLOBALVARIABLE_H
