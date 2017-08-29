//
// Created by tzhou on 8/12/17.
//

#ifndef LLPARSER_BITCASTINST_H
#define LLPARSER_BITCASTINST_H


#include <map>
#include <ir/irEssential.h>
#include <utilities/macros.h>

class BitCastInst: public Instruction {
    string _ret_str;
    string _ty_str;
    string _ty2_str;
    string _value_str;
public:
    BitCastInst();

    const string& ret_str() const {
        return _ret_str;
    }

    const string& ty_str() const {
        return _ty_str;
    }

    const string& ty2_str() const {
        return _ty2_str;
    }

    const string &value_str() const {
        return _value_str;
    }

    void set_value_str(const string &_value_str) {
        BitCastInst::_value_str = _value_str;
    }

    void set_ret_str(const string &_ret_str) {
        BitCastInst::_ret_str = _ret_str;
    }

    void set_ty_str(const string &_ty_str) {
        BitCastInst::_ty_str = _ty_str;
    }

    void set_ty2_str(const string &_ty2_str) {
        BitCastInst::_ty2_str = _ty2_str;
    }



    static void parse(Instruction* inst);
};


#endif //LLPARSER_BITCASTINST_H
