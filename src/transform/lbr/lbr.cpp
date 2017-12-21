
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <asmParser/irBuilder.h>
#include <transform/ben-alloc/benAlloc.cpp>

// A module pass template

class LBRPass: public Pass {
    int nlevel = 3;
public:
    LBRPass() {
        set_is_module_pass();
    }

    void get_target_functions(Module* module, std::set<Function*>& funcs) {
        BenAllocPass* ben_alloc = new BenAllocPass();
        for (auto i: ben_alloc->alloc_set()) {
            get_callers(module, i->old_name, funcs);
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

        for (auto i: old_set) {
            funcs.insert(i);
        }
    }

    void instrument_calls(Module* module, std::vector<Function*>& funcs) {
        for (auto F: funcs) {
            for (auto ci: F->caller_list()) {
                
                int ci_pos = get_index_in_block();
                ci->parent()->insert_instruction(ci_pos, )
            }
        }
    }

    void test_append_global(Module* module) {
        module->append_new_global("@sopt.br.1 = thread_local global i32 0, align 4");
        module->print_to_file(SysDict::get_pass_out_name("lbr"));
    }

    bool run_on_module(Module* module) override {
        
        return true;
    }
};

REGISTER_PASS(LBRPass);
