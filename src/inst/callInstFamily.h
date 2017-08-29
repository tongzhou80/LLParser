//
// Created by tzhou on 8/27/17.
//

#ifndef LLPARSER_CallInstFamily_H
#define LLPARSER_CallInstFamily_H

#include <ir/instruction.h>

class CallInstFamily: public Instruction {
protected:
    bool _is_indirect_call;
    bool _is_varargs;
    bool _has_bitcast;

    /* for indirect calls */
    string _called_label;
    Function* _called;
    Instruction* _target_inst;  // the reference for indirect call's target
public:
    CallInstFamily();

    virtual void init_raw_field();
    Function* called_function()                       { return _called; }
    void set_called_function(Function* f)             { _called = f; }

    bool is_indirect_call() const {
        return _is_indirect_call;
    }

    void set_is_indirect_call(bool _indirect_call=1) {
        CallInstFamily::_is_indirect_call = _indirect_call;
    }

    bool is_varargs() const {
        return _is_varargs;
    }

    void set_is_varargs(bool _is_varargs=1) {
        CallInstFamily::_is_varargs = _is_varargs;
    }

    bool has_bitcast() const {
        return _has_bitcast;
    }

    void set_has_bitcast(bool _has_bitcast=1) {
        CallInstFamily::_has_bitcast = _has_bitcast;
    }

    const string &called_label() const {
        return _called_label;
    }

    void set_called_label(const string &_called_label) {
        CallInstFamily::_called_label = _called_label;
    }

    Instruction *target_inst() const {
        return _target_inst;
    }

    void set_target_inst(Instruction *_target_inst) {
        CallInstFamily::_target_inst = _target_inst;
    }

    void replace_callee(string callee);
    void replace_args(string newargs);

    void try_resolve_indirect_call();
    void resolve_callee_symbol(string fn_name);

};

#endif //LLPARSER_CallInstFamily_H
