//
// Created by GentlyGuitar on 6/7/2017.
//

#include <asmParser/sysDict.h>
#include <asmParser/instParser.h>
#include "irEssential.h"
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

void Instruction::copy_metadata_from(Instruction *i) {
    //_properties = i->properties();
    _dbg_id = i->dbg_id();
}

DILocation* Instruction::debug_loc() {
    if (!_debug_loc) {
        MetaData* md = SysDict::module()->get_debug_info(_dbg_id);
        _debug_loc = dynamic_cast<DILocation*>(md);
    }

    return _debug_loc;
}

Instruction* Instruction::clone() {
    Instruction* i = new Instruction(*this);
    i->set_parent(NULL);
    return i;
}