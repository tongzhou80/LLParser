//
// Created by tzhou on 10/16/17.
//

#include <passes/pass.h>
#include <ir/irEssential.h>
#include <asmParser/sysDict.h>
#include <asmParser/irBuilder.h>
#include <utilities/strings.h>
#include <peripheral/sysArgs.h>

// A module pass template

class RawReplaceCallPass: public Pass {
public:
    RawReplaceCallPass() {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) override {
        insert_declaration("malloc", "je_malloc", false);
        insert_declaration("calloc", "je_calloc", false);
        insert_declaration("realloc", "je_realloc", false);
        insert_declaration("free", "je_free", false);

        replace();
        string out = SysArgs::get_option("output");
        if (out.empty()) {
            out = SysDict::filename();
            Strings::replace(out, ".ll", ".je.ll");
        }
        SysDict::module()->print_to_file(out);
        return true;
    }

    void replace() {
        //todo: non-dirty way
        string suffixes[4] = {" ", ",", ")", "("};
        for (auto F: SysDict::module()->function_list()) {
            for (auto B: F->basic_block_list()) {
                for (auto I: B->callinst_list()) {
                    for (auto& suf: suffixes) {
                        string targets[4] = {"malloc", "calloc", "realloc", "free"};
                        for (auto& t: targets) {
                            string old = "@"+t+suf;
                            if (I->raw_text().find(old) != string::npos) {
                                Strings::replace(I->raw_text(), old, "@je_"+t+suf);
                            }
                        }
                    }
                }
            }
        }
    }

    bool insert_declaration(string oldname, string newname, bool add_id=true) {
        Function* func = SysDict::module()->get_function(oldname);

        if (func == NULL) {
            return 0;
        }
        guarantee(func->is_external(), "malloc family should be external");
        //Function* newfunc = SysDict::module()->create_child_function(_new_malloc);

        Function* existed = SysDict::module()->get_function(newname);
        if (existed) {
            return 0; // return if already inserted
        }

        /* manipulate the text */
        string text = func->raw_text();
        string old_call = oldname + '(';
        string new_call = newname + "(i32, ";
        if (!add_id || Strings::startswith(oldname, "f90_")) {  // a bit funky
            new_call = newname + '(';
        }

        Strings::replace(text, old_call, new_call);
        Function* newfunc = IRBuilder::create_function_declaration(text);
        SysDict::module()->insert_function_after(func, newfunc);
    }
};

REGISTER_PASS(RawReplaceCallPass);