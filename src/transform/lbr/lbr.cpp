
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <asmParser/irBuilder.h>
#include <transform/ben-alloc/benAlloc.cpp>

// A module pass template

class LBRPass: public Pass {
    int _nlevel = 2;
public:
    LBRPass() {
        set_is_module_pass();
    }

    void do_initialization() override {
        if (has_argument("nlevel")) {
            _nlevel = std::stoi(get_argument("nlevel"));
        }

        // nlevel here must decrement
        _nlevel--;
    }

    void get_target_functions(Module* module, std::set<Function*>& funcs) {
        BenAllocPass* ben_alloc = new BenAllocPass();
        ben_alloc->do_initialization();
        for (auto i: ben_alloc->alloc_set()) {
            get_callers(module, i->old_name, funcs, _nlevel);
        }
    }

    void get_callers(Module* module, string& alloc, std::set<Function*>& funcs, int nlevel) {
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
                funcs.insert(f);
            }

            old_set = new_set;
        }

        for (auto i: old_set) {
            funcs.insert(i);
        }
    }

    void instrument_calls(Module* module, std::set<Function*>& funcs, int mode) {
        for (auto F: funcs) {
            for (auto ci: F->caller_list()) {
                /* create instrument instructions */
                string v1 = IRBuilder::get_new_local_varname();
                Instruction* i1 = IRBuilder::create_instruction(v1+" = load i32, i32* @sopt.ctx, align 4");
                string v2 = IRBuilder::get_new_local_varname();
                string i2_text = v2 + " = add i32 " + v1 + ", " + std::to_string(ci->dbg_id());
                Instruction* i2 = IRBuilder::create_instruction(i2_text);
                Instruction* i3 = IRBuilder::create_instruction("store i32 "+v2+", i32* @sopt.ctx, align 4");

                string v4 = IRBuilder::get_new_local_varname();
                Instruction*i4 = IRBuilder::create_instruction(v4+" = load i32, i32* @sopt.ctx, align 4");
                string v5 = IRBuilder::get_new_local_varname();
                string i5_text = v5 + " = sub i32 " + v4 + ", " + std::to_string(ci->dbg_id());
                Instruction* i5 = IRBuilder::create_instruction(i5_text);
                Instruction* i6 = IRBuilder::create_instruction("store i32 "+v5+", i32* @sopt.ctx, align 4");

                /* insert at exit */
                int ci_pos = ci->get_index_in_block();
                ci->parent()->insert_instruction(ci_pos+1, i6);
                ci->parent()->insert_instruction(ci_pos+1, i5);
                ci->parent()->insert_instruction(ci_pos+1, i4);

                /* insert at entry */
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
        std::set<Function*> funcs;
        get_target_functions(module, funcs);
        zpd(funcs.size())
        instrument_calls(module, funcs, 0);
        module->print_to_file(SysDict::get_pass_out_name("lbr"));
        return true;
    }
};

REGISTER_PASS(LBRPass);
