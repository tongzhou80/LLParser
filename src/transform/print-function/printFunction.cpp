//
// Created by tzhou on 12/20/17.
//
#include <passes/pass.h>
#include <ir/irEssential.h>

class PrintFunctionPass: public Pass {
public:
    PrintFunctionPass() {
        set_is_function_pass();
    }

    bool run_on_function(Function* function) override {
        std::cout << function->name() << ": " << function->instruction_count() << " instructions" << std::endl;
    }

    bool do_finalization(Module* M) override {
        std::cout << "in total: " << M->function_list().size() << " functions" << std::endl;
    }
};

REGISTER_PASS(PrintFunctionPass);