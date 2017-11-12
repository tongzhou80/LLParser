//
// Created by tzhou on 6/11/17.
//

#ifndef LLPARSER_CALLINST_H
#define LLPARSER_CALLINST_H

#include <map>
#include <utilities/macros.h>
#include "callInstFamily.h"

class CallInst: public CallInstFamily {


public:
    CallInst();

    virtual void init_raw_field();


    /** @brief Return a copy of this instruction
     *
     * @return
     */
    static void parse(Instruction* inst);
};

#endif //LLPARSER_CALLINST_H
