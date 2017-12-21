//
// Created by tzhou on 12/20/17.
//
#include <passes/pass.h>
#include <ir/irEssential.h>

class PrintFunctionPass: public Pass {
    size_t func_cnt;
public:
    PrintFunctionPass() {
        set_is_function_pass();

        func_cnt = 0;
    }

    bool run_on_function(Function* function) override {
        for (auto B: function->basic_block_list()) {
            std::cout << B->name() << std::endl;
        }
    }

    bool
};

REGISTER_PASS(PrintFunctionPass);