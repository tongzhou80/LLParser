
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <asmParser/irBuilder.h>

// A module pass template

class WrfDebugPass: public Pass {
public:
    WrfDebugPass() {
        set_is_module_pass();
    }

    void print_nelem(Module* module) {
        Function* f = module->get_function("f90_ben_ptr_alloc04");
        for (auto ci: f->caller_list()) {
            string nelem = ci->get_nth_arg_by_split(1);
            zps(nelem)
        }
            
//        GlobalVariable* str = IRBuilder::add_global_string(module, "test");
//        CallInst* ci = IRBuilder::create_printf_callinst(module, str);
//        Function* f = module->get_function("main");
//        auto bb = f->basic_block_list()[0];
//        bb->insert_instruction(5, (Instruction*)ci);
//        module->print_to_file("out.ll");
    }

    bool run_on_module(Module* module) override {
        print_nelem(module);
        return true;
    }
};

REGISTER_PASS(WrfDebugPass);
