//
// Created by tzhou on 8/27/17.
//

#include <utilities/flags.h>
#include <ir/instruction.h>
#include <asmParser/sysDict.h>
#include <utilities/strings.h>
#include "callInstFamily.h"
#include "bitcastInst.h"

CallInstFamily::CallInstFamily() {
    _is_indirect_call = false;
    _is_varargs = false;
    _has_bitcast = false;
    _called = NULL;
    _target_inst = NULL;
}

void CallInstFamily::init_raw_field() {
    set_raw_field("cconv", "");
    set_raw_field("ret-attrs", "");
    set_raw_field("args", "");
}


/** @brief Replace the called function.
 * This function does not create a function declaration for the
 * new function, it assumes the new function is valid
 *
 * Side effects:
 * The old callee will have one less caller.
 * The new callee will have one more caller.
 */
void CallInstFamily::replace_callee(string callee) {
    Function* new_callee = SysDict::module()->get_function(callee);
    if (new_callee == NULL) {
        throw FunctionNotFoundError(callee);
    }
    string old = called_function()->name();
    Strings::replace(_raw_text, old, callee);  // todo: this should be fine, but I am not sure
    set_called_function(new_callee);

    new_callee->append_user(this);

    Function* old_callee = SysDict::module()->get_function(old);
    guarantee(old_callee, "callee %s not found", old.c_str());
    old_callee->remove_user(this);
//    auto& vec = old_callee->user_list();
//    auto newend = std::remove(vec.begin(), vec.end(), this);
//    vec.erase(newend, vec.end());
}

void CallInstFamily::replace_args(string newargs) {
    string oldargs = get_raw_field("args");
    Strings::replace(_raw_text, oldargs, newargs);
    set_raw_field("args", newargs);
}

void CallInstFamily::resolve_callee_symbol(string fn_name) {
    /* args parsing is more complex, not deal with it for now */
    Function* callee = SysDict::module()->get_function(fn_name);
    if (callee == NULL) {
        callee = SysDict::module()->create_child_function_symbol(fn_name);
    }

    set_called_function(callee);
    callee->append_user(this);

    // now this instruction is considered fully parsed;
    // Type info is always ignored for now
    set_is_fully_parsed();

    if (CallInstParsingVerbose) {
        printf( "  name: |%s|\n",
                fn_name.c_str());
    }
}

void CallInstFamily::try_resolve_indirect_call() {
    guarantee(parent() != NULL, "Parent must not be NULL when resolving indirect calls");

    for (auto rit = parent()->end(); rit != parent()->begin();) {
        --rit;
        Instruction* I = *rit;
        //zpl("name: |%s|   label: |%s|", I->name().c_str(), ('%'+called_label()).c_str());
        if (I->name() == '%' + called_label()) {
            set_target_inst(I);
            if (I->type() == Instruction::BitCastInstType) {
                string value = ((BitCastInst*)I)->value_str();
                if (value[0] == '@') {
                    //zpl("resolved indirect call target to %s (%s)", value.c_str(), raw_c_str());
                    resolve_callee_symbol(&value[1]);
                }
            }
            //zpl("got call %s", raw_c_str());
            //zpl("got target %s", I->raw_c_str())
        }
    }
}
