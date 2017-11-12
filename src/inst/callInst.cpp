//
// Created by tzhou on 6/11/17.
//

#include <algorithm>
#include <asmParser/sysDict.h>
#include <asmParser/instParser.h>
#include <utilities/strings.h>
#include <utilities/flags.h>
#include "callInst.h"
#include "bitcastInst.h"

CallInst::CallInst() {
    set_type(Instruction::CallInstType);

    init_raw_field();
}

void CallInst::init_raw_field() {
    CallInstFamily::init_raw_field();
//    set_raw_field("tail", "");
//    set_raw_field("fast-math", "");
}
//
//Instruction* CallInst::clone() {
//    auto ci = new CallInst(*this);
//    ci->set_parent(NULL);
//    return ci;
//}

