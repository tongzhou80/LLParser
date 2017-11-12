//
// Created by GentlyGuitar on 6/7/2017.
//

#include <asmParser/sysDict.h>
#include <asmParser/instParser.h>
#include "irEssential.h"
#include <inst/instEssential.h>
#include <di/diEssential.h>

Instruction::Instruction(): Value() {
    _is_fully_parsed = false;
    //_parser = SysDict::instParser;  // for synchronous inst parsing
    _type = UnknownInstType;
    _has_assignment = false;
    _parent = NULL;
    _dbg_id = -1;
    _debug_loc = NULL;
}

Function* Instruction::function() {
    if (parent()) {
        return parent()->parent();
    }
    else {
        return NULL;
    }
}

int Instruction::get_index_in_block() {
    guarantee(parent(), "no parent");
    return parent()->get_instruction_index(this);
}

Point2D<int> Instruction::get_position_in_function() {
    int x = parent()->get_index_in_function();
    int y = get_index_in_block();
    return Point2D<int>(x, y);
}

Module* Instruction::module() const {
    return parent()->module();
}

void Instruction::copy_metadata_from(Instruction *i) {
    //_properties = i->properties();
    _dbg_id = i->dbg_id();
}

DILocation* Instruction::debug_loc() {
    if (!_debug_loc) {
        MetaData* md = module()->get_debug_info(_dbg_id);
        _debug_loc = dynamic_cast<DILocation*>(md);
    }

    return _debug_loc;
}

Instruction* Instruction::clone() {
    Instruction* i;
    switch (type()) {
        case Instruction::AllocaInstType:
            i = new AllocaInst(*dynamic_cast<AllocaInst*>(this));
            break;
        case Instruction::BranchInstType:
            i = new BranchInst(*dynamic_cast<BranchInst*>(this));
            break;
        case Instruction::CallInstType:
            i = new CallInst(*dynamic_cast<CallInst*>(this));
            break;
        case Instruction::InvokeInstType:
            i = new InvokeInst(*dynamic_cast<InvokeInst*>(this));
            break;
        case Instruction::LoadInstType:
            i = new LoadInst(*dynamic_cast<LoadInst*>(this));
            break;
        case Instruction::StoreInstType:
            i = new StoreInst(*dynamic_cast<StoreInst*>(this));
            break;
        case Instruction::BitCastInstType:
            i = new BitCastInst(*dynamic_cast<BitCastInst*>(this));
            break;
        default:
            guarantee(0, "sanity");
    }
    i->set_parent(NULL);
    return i;
}