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

    string addr_str()                                   { return _addr_str; }
    void set_addr_str(string s)                         { _addr_str = s; }

    string ret_type_str()                               { return _ret_ty_str; }
    void set_ret_type_str(string t)                     { _ret_ty_str = t; }

    string pointer_type_str()                           { return _pointer_ty_str; }
    void set_pointer_type_str(string t)                 { _pointer_ty_str = t; }

    int alignment()                                     { return _align; }
    void set_alignment(int a)                           { _align = a; }

    static void parse(Instruction* inst);
};


#endif //LLPARSER_LOADINST_H
