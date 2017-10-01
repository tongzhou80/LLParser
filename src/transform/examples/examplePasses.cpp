//
// Created by tzhou on 9/19/17.
//

#include <passes/pass.h>
#include <ir/irEssential.h>

// A module pass template

class ExampleModulePass: public Pass {
public:
    ExampleModulePass() {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) override {
        for (auto F: module->function_list()) {
            std::cout << F->name() << std::endl;
        }
    }
};

REGISTER_PASS(ExampleModulePass);

/*********************************************************/


// A function pass template

class ExampleFunctionPass: public Pass {
public:
    ExampleFunctionPass() {
        set_is_function_pass();
    }

    bool run_on_function(Function* function) override {
        for (auto B: function->basic_block_list()) {
            std::cout << B->name() << std::endl;
        }
    }
};

REGISTER_PASS(ExampleFunctionPass);

/*********************************************************/


// A basic block pass template

class ExampleBasicBlockPass: public Pass {
public:
    ExampleBasicBlockPass() {
        set_is_basic_block_pass();
    }

    bool run_on_basic_block(BasicBlock* bb) override {
        for (auto I: bb->instruction_list()) {
            std::cout << I->name() << std::endl;
        }
    }
};

REGISTER_PASS(ExampleBasicBlockPass);