//
// Created by tzhou on 8/27/17.
//

#include "invokeInst.h"

InvokeInst::InvokeInst() {
    set_type(Instruction::InvokeInstType);

    init_raw_field();
}

void InvokeInst::init_raw_field() {
    CallInstFamily::init_raw_field();
}
