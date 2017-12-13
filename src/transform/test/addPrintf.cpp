
#include <passes/pass.h>
#include <ir/irEssential.h>

// A module pass template

class AddPrintfPass: public Pass {
public:
    AddPrintfPass() {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) override {
        for (auto F: module->function_list()) {
            std::cout << F->name() << std::endl;
        }
    }
};

REGISTER_PASS(AddPrintfPass);