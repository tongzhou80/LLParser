
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <asmParser/irBuilder.h>
#include <transform/ben-alloc/benAlloc.cpp>

// A module pass template

class CtxVarPass: public Pass {
    const int nlevel = 3;
public:
    CtxVarPass() {
        set_is_module_pass();
    }

    void get_target_functions(Module* module, std::set<Function*>& funcs) {
        BenAllocPass* ben_alloc = new BenAllocPass();
        for (auto i: ben_alloc->alloc_set()) {
            get_callers(module, i, funcs);
        }
    }

    void get_callers(Module* module, string& alloc, std::set<Function*>& funcs) {
        Function* alloc_f = module->get_function(alloc);

        if (!alloc_f) {
            return;
        }

        std::set<Function*> old_set;
        old_set.insert(alloc_f);

        while (nlevel--) {
            std::set<Function*> new_set;
            for (auto f: old_set) {
                for (auto ci: f->caller_list()) {
                    new_set.insert(ci->function());
                }
                if (f != alloc_f)
                    funcs.insert(f);
            }

            old_set = new_set;
        }


    }

    void instrument_entry(Function* func) {
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

REGISTER_PASS(CtxVarPass);
