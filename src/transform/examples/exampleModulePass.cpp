//
// Created by tzhou on 9/19/17.
//

#include <passes/pass.h>
#include <ir/irEssential.h>

class ExampleModulePass: public Pass {
public:
    ExampleModulePass() {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) {
        for (auto F: module->function_list()) {
            std::cout << F->name() << std::endl;
        }
    }
};

REGISTER_PASS(ExampleModulePass);