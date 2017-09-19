//
// Created by tzhou on 9/19/17.
//

#include <passes/pass.h>
#include <ir/irEssential.h>
#include <asmParser/sysDict.h>

class SplitModulePass: public Pass {
public:
    SplitModulePass() {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) {
        string out = SysDict::filename();
        FILE* header_fp = fopen((out+".header").c_str(), "w");
        print_data_prior_to_functions(header_fp, module);
    }

    void print_data_prior_to_functions(FILE* fp, Module* m) {
        fprintf(fp, "; ModuleID = '%s'\n", m->module_id().c_str());
    }
};

REGISTER_PASS(SplitModulePass);

