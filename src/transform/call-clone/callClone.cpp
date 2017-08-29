//
// Created by tlaber on 6/25/17.
//

#include "callClone.h"

#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>
#include <ir/di/diEssential.h>

// Each pass can be multiple types; But that shouldn't be necessary
CallClonePass::CallClonePass() {
    set_is_module_pass();

    _has_overlapped_path = false;

    PrintCloning = 1;
    TracingVerbose = 1;
}

CallClonePass::~CallClonePass() {
    printf("pass unloading is not yet implemented! Do stuff in do_finalization!\n");
}

bool CallClonePass::run_on_module(Module *module) {
    Function* malloc = module->get_function("malloc");
    int cnt = 0;

    do {
        if (TracingVerbose) {
            printf("\nround: %d\n", cnt);
        }

        cnt++;
        _black.clear();
        _has_overlapped_path = false;  // always assume this round is the last round
        auto& malloc_users = malloc->user_list();
        Function::InstList malloc_users_copy = malloc_users; // user_list might change during the iteration since new functions may be created
        for (auto uit = malloc_users_copy.begin(); uit != malloc_users_copy.end(); ++uit) {
            Function* func = (*uit)->function();
            do_clone(func, module);
            //auto& users = func->user_list();

        }
    } while (_has_overlapped_path && cnt < 4);


    string out = SysArgs::cur_target;
    if (out.empty()) {
        out = "new";
    }
    out += ".callClone";

    if (SysArgs::has_property("output")) {
        out = SysArgs::get_property("output");
    }

    SysDict::module()->print_to_file(out);
}

void CallClonePass::do_clone(Function* f, Module *module) {
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

    auto& users = f->user_list();
    Function::InstList users_copy = users;
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



REGISTER_PASS(CallClonePass);
