
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
        CallInst* ci = IRBuilder::create_printf_callinst(module, str, "i8* %a");
        Function* f = module->get_function("main");
        auto bb = f->basic_block_list()[0];
        bb->insert_instruction(5, (Instruction*)ci);
        module->print_to_file("out.ll");
        return true;
    }
};

REGISTER_PASS(AddPrintfPass);
