//
// Created by tlaber on 7/5/17.
//

#include "loadInst.h"
#include <asmParser/instParser.h>

LoadInst::LoadInst() {
    set_type(Instruction::LoadInstType);
}

void LoadInst::parse(Instruction *inst) {
    guarantee(inst->type() == InstType::LoadInstType, " ");
    inst->parser()->do_load(inst);
}