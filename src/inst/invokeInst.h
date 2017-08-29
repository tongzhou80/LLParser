//
// Created by tzhou on 8/27/17.
//

#ifndef LLPARSER_INVOKEINST_H
#define LLPARSER_INVOKEINST_H

#include "callInstFamily.h"

class InvokeInst: public CallInstFamily {
public:
    InvokeInst();

    virtual void init_raw_field();

};

#endif //LLPARSER_INVOKEINST_H
