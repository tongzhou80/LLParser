//
// Created by tzhou on 12/20/17.
//
#include <passes/pass.h>
#include <ir/irEssential.h>

class PrintFunctionPass: public Pass {
    size_t _inst_cnt;
public:
    PrintFunctionPass() {
        set_is_function_pass();

        _inst_cnt = 0;
    }

    bool run_on_function(Function* function) override {
        std::cout << function->name() << ": " << function->instruction_count() << " instructions" << std::endl;
        _inst_cnt += function->instruction_count();
    }

    bool do_finalization(Module* M) override {
        std::cout << "in total: " << M->function_list().size() << " functions, "
                  << _inst_cnt << " instructions" << std::endl;
    }
};

REGISTER_PASS(PrintFunctionPass);