//
// Created by tlaber on 7/5/17.
//

#ifndef LLPARSER_LOADINST_H
#define LLPARSER_LOADINST_H


#include <map>
#include <ir/irEssential.h>
#include <utilities/macros.h>

class LoadInst: public Instruction {
    string _ret_ty_str;
    string _addr_str;
    string _pointer_ty_str;
    short _align;
public:
    LoadInst();

    string pointer_type_str()                           { return get_raw_field("ty") + '*'; }

    static void parse(Instruction* inst);
};


#endif //LLPARSER_LOADINST_H
