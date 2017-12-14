
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <asmParser/irBuilder.h>

// A module pass template

class AddPrintfPass: public Pass {
public:
    AddPrintfPass() {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) override {
        GlobalVariable* str = IRBuilder::add_global_string(module, "test");
        IRBuilder::create_printf_callinst(module, str);
        
        module->print_to_file("out.ll");
        return true;
    }
};

REGISTER_PASS(AddPrintfPass);
