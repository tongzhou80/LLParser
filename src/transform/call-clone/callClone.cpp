//
// Created by tlaber on 6/25/17.
//

#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>
#include <di/diEssential.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <set>
#include "contextGenerator.h"

class CallClonePass: public Pass {
    bool PrintCloning;
    bool TracingVerbose;
    std::set<string> _black;
    bool _has_overlapped_path;
    std::ofstream _ofs;
public:
    CallClonePass() {
        set_is_module_pass();

        _has_overlapped_path = false;

        PrintCloning = 1;
        TracingVerbose = 1;
    }

    bool run_on_module(Module* module) {
        ContextGenerator cg;
        cg.generate(module, "malloc", 3);
//        Function* malloc = module->get_function("malloc");
//        if (!malloc) {
//            return false;
//        }
//
//        int cnt = 0;
//
//        do {
//            if (TracingVerbose) {
//                printf("\nround: %d\n", cnt);
//            }
//
//            cnt++;
//            _black.clear();
//            _has_overlapped_path = false;  // always assume this round is the last round
//            auto& malloc_users = malloc->user_set();
//            Function::InstList malloc_users_copy = malloc_users; // user_set might change during the iteration since new functions may be created
//            for (auto uit = malloc_users_copy.begin(); uit != malloc_users_copy.end(); ++uit) {
//                Function* func = (*uit)->function();
//                do_clone(func, module);
//                //auto& users = func->user_set();
//
//            }
//        } while (_has_overlapped_path && cnt < 4);
//
//        string out = SysDict::filename() + name();
//        SysDict::module()->print_to_file(out);
        return true;
    }

    void do_clone(Function* f, Module *module) {
        if (TracingVerbose) {
            printf("inpsect func: %s\n", f->name_as_c_str());
        }

        // not allow loop in the call graph
        if (_black.find(f->name()) != _black.end()) {
            return;
            //guarantee(0, "should not recheck a node: %s", f->name().c_str());
        }
        else {
            _black.insert(f->name());
        }

        auto& users = f->user_set();
        Function::InstSet users_copy = users;
//    if (users_copy.size() > 1) {
//        if (f->name() == "BZ2_bzReadOpen") {
//            zpl("%s is called in ", f->name().c_str());
//            for (auto u: users) {
//                zpl("  %s", u->function()->name().c_str());
//            }
//        }
//    }
        for (int i = 0; i < users_copy.size(); ++i) {
            CallInst* ci = (CallInst*)users_copy[i];
            if (i > 0) {
                _has_overlapped_path = true;  // set the flag to scan again
                Function* fclone = f->clone();
                module->append_new_function(fclone);

                if (PrintCloning) {
                    printf("cloned %s to %s\n", f->name_as_c_str(), fclone->name_as_c_str());
                }
                ci->replace_callee(fclone->name());
            }

            if (_black.find(ci->function()->name()) == _black.end()) {
                do_clone(ci->function(), module);
            }
        }
    }

    //bool do_finalization(Module* module);
};


REGISTER_PASS(CallClonePass);
