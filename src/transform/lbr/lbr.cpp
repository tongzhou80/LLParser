
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

    void instrument_calls(Module* module, std::vector<Function*>& funcs, int mode) {
        for (auto F: funcs) {
            for (auto ci: F->caller_list()) {
                /* instrument exit */
                string v1 = IRBuilder::get_new_local_varname();
                Instruction* i1 = IRBuilder::create_instruction(v1+" = load i32, i32* @sopt.ctx, align 4");
                string v2 = IRBuilder::get_new_local_varname();
                string i2_text = v2 + " = sub i32 " + v1 + ", " + std::to_string(ci->dbg_id());
                Instruction* i2 = IRBuilder::create_instruction(i2_text);
                Instruction* i3 = IRBuilder::create_instruction("store i32 "+v2+", i32* @sopt.ctx, align 4");

                int ci_pos = ci->get_index_in_block();
                ci->parent()->insert_instruction(ci_pos+1, i3);
                ci->parent()->insert_instruction(ci_pos+1, i2);
                ci->parent()->insert_instruction(ci_pos+1, i1);

                /* instrument entry */
                v1 = IRBuilder::get_new_local_varname();
                i1 = IRBuilder::create_instruction(v1+" = load i32, i32* @sopt.ctx, align 4");
                v2 = IRBuilder::get_new_local_varname();
                i2_text = v2 + " = add i32 " + v1 + ", " + std::to_string(ci->dbg_id());
                i2 = IRBuilder::create_instruction(i2_text);
                i3 = IRBuilder::create_instruction("store i32 "+v2+", i32* @sopt.ctx, align 4");

                ci_pos = ci->get_index_in_block();
                ci->parent()->insert_instruction(ci_pos, i3);
                ci->parent()->insert_instruction(ci_pos, i2);
                ci->parent()->insert_instruction(ci_pos, i1);
            }
        }
    }


    void test_append_global(Module* module) {
        module->append_new_global("@sopt.bp.1 = thread_local global i32 0, align 4");
        module->print_to_file(SysDict::get_pass_out_name("lbr"));
    }

    bool run_on_module(Module* module) override {
        module->append_new_global("@sopt.ctx = thread_local global i32 0, align 4");
        std::vector<Function*> roots;
        roots.push_back(module->get_function("malloc"));
        instrument_calls(module, roots, 0);
        module->print_to_file(SysDict::get_pass_out_name("lbr"));
        return true;
    }
};

REGISTER_PASS(LBRPass);
