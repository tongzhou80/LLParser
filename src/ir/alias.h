//
// Created by tzhou on 9/7/17.
//

#ifndef LLPARSER_ALIAS_H
#define LLPARSER_ALIAS_H

#include "value.h"

class Function;

class Alias: public Value {
    Value* _aliasee;
public:
    Value* aliasee()                                              { return _aliasee; }
};

#endif //LLPARSER_ALIAS_H
