//
// Created by tzhou on 8/10/17.
//

#include <set>
#include <algorithm>
#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <di/diEssential.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>

#include "contextGenerator.h"

struct MFunc {
    string old_name;
    string new_name;
    bool add_id;

    MFunc(string oldname, string newname, bool addid):
            old_name(oldname), new_name(newname), add_id(addid) {}
};

class AddDeclarePass: public Pass {
    string _lang;
    bool _use_indi;
public:
    AddDeclarePass() {
        set_is_module_pass();

        _lang = "all";
    }

    ~AddDeclarePass() {

    }


    void init_lang(Module* module) {
        if (_lang == "c" || _lang == "cpp" || _lang == "all") {
            _alloc_set.push_back(new MFunc("malloc", "ben_malloc", true));
            _alloc_set.push_back(new MFunc("calloc", "ben_calloc", true));
            _alloc_set.push_back(new MFunc("realloc", "ben_realloc", true));
            _free_set.push_back(new MFunc("free", "ben_free", false));
        }

        if (_lang == "cpp" || _lang == "all") {
            _alloc_set.push_back(new MFunc("_Znam", "ben_malloc", true));
            _alloc_set.push_back(new MFunc("_Znwm", "ben_malloc", true));
            _free_set.push_back(new MFunc("_ZdaPv", "ben_free", false));
            _free_set.push_back(new MFunc("_ZdlPv", "ben_free", false));
        }

        Module* cur_module = SysDict::module();
        for (auto t: _alloc_set) {
            insert_declaration(cur_module, t->old_name, t->new_name, t->add_id);
            if (_use_indi) {
                string indi_name = t->new_name;
                Strings::replace(indi_name, "ben_", "indi_");
                insert_declaration(cur_module, t->old_name, indi_name, false);
            }
        }

        for (auto t: _free_set) {
            insert_declaration(cur_module, t->old_name, t->new_name, t->add_id);
            if (_use_indi) {
                string indi_name = t->new_name;
                Strings::replace(indi_name, "ben_", "indi_");
                insert_declaration(cur_module, t->old_name, indi_name, false);
            }
        }
//
//        insert_declaration("malloc", "ben_malloc", true);
//        insert_declaration("calloc", "ben_calloc", true);
//        insert_declaration("realloc", "ben_realloc", true);
//        insert_declaration("free", "ben_free", false);
//
//        insert_declaration("malloc", "indi_malloc", false);
//        insert_declaration("calloc", "indi_calloc", false);
//        insert_declaration("realloc", "indi_realloc", false);
//        insert_declaration("free", "indi_free", false);

    }

    bool run_on_module(Module* module) override {
        init_lang(module);

        int nlevel = 2;
        if (has_argument("nlevel")) {
            nlevel = std::stoi(get_argument("nlevel"));
        }
        if (has_argument("logclone")) {
            _logclone = true;
            _clone_log.open("clone.log");
        }
        if (has_argument("min-cxt")) {
            _min_cxt = std::stoi(get_argument("min-cxt"));
            zpd(_min_cxt)
        }
        if (has_argument("lang")) {
            _lang = get_argument("lang");
        }
        if (has_argument("indi")) {
            _use_indi = true;
        }
    }
//
//    void replace_free() {
//        for (auto t: _free_set) {
//            if (Function* free_fp = SysDict::module()->get_function(t->old_name)) {
//                for (auto ci: free_fp->caller_list()) {
//                    ci->replace_callee(t->new_name);
//                }
//            }
//        }
//
//        //todo: non-dirty way
//        if (!_use_indi) {
//            return;
//        }
//
//        string suffixes[3] = {" ", ",", ")"};
//        for (auto F: SysDict::module()->function_list()) {
//            for (auto B: F->basic_block_list()) {
//                for (auto I: B->callinst_list()) {
//                    for (auto& suf: suffixes) {
//                        //string targets[4] = {"malloc", "calloc", "realloc", "free"};
//                        for (auto& t: _alloc_set) {
//                            string old = "@"+t->old_name+suf;
//                            if (I->raw_text().find(old) != string::npos) {
//                                Strings::replace(I->raw_text(), old, "@indi_"+t->old_name+suf);
//                            }
//                        }
//                        for (auto& t: _free_set) {
//                            string old = "@"+t->old_name+suf;
//                            if (I->raw_text().find(old) != string::npos) {
//                                Strings::replace(I->raw_text(), old, "@indi_"+t->old_name+suf);
//                            }
//                        }
//                    }
//                }
//            }
//        }
//    }

    bool insert_declaration(Module* m, string oldname, string newname, bool add_id=true) {
        Function* func = m->get_function(oldname);

        if (func == NULL) {
            return false;
        }
        guarantee(func->is_external(), "malloc family should be external");

        if (m->get_function(newname)) {
            return false; // return if already inserted
        }

        /* manipulate the text */
        string text = func->raw_text();
        string old_call = oldname + '(';
        string new_call = newname + "(i32, ";
        //if (!add_id || Strings::startswith(oldname, "f90_")) {  // a bit funky
        if (!add_id) {
            new_call = newname + '(';
        }

        Strings::replace(text, old_call, new_call);
        Function* newfunc = IRBuilder::create_function_declaration(text);
        m->insert_function_after(func, newfunc);
    }

};


REGISTER_PASS(AddDeclarePass);
