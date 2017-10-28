//
// Created by tzhou on 10/28/17.
//

#include <cstdio>
#include <ir/function.h>
#include <passes/pass.h>
#include <asmParser/sysDict.h>

class LinkTracePass: public Pass {
    string _target;
public:
    LinkTracePass() {
        set_is_global_pass();
    }

    bool run_on_global() override {
        if (!has_argument("target")) {
            std::cerr << "LinkTrace pass has a required argument `target` that should be the name of the linked file! Exit...\n";
            exit(1);
        }
        _target = get_argument("target");

        Module* target = SysDict::get_module(_target);
        for (auto F: target->function_list()) {
            F->dump();
        }

        //printf("number of modules: %lu\n", SysDict::module_table().size());
    }
};

REGISTER_PASS(LinkTracePass)