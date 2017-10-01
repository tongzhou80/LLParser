//
// Created by tzhou on 10/1/17.
//


#include <passes/pass.h>
#include <ir/irEssential.h>
#include <iomanip>

class InstcountPass: public Pass {
    size_t _parsed;
    std::map<string, size_t> _op_count;
public:
    InstcountPass(): _parsed(0) {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) override {
        size_t nfunc = 0;
        size_t nbb = 0;
        for (auto F: module->function_list()) {
            nfunc++;
            for (auto B: F->basic_block_list()) {
                nbb++;
                for (auto I: B->instruction_list()) {
                    _op_count[I->opcode()]++;
                    if (I->type() != Instruction::UnknownInstType) {
                        _parsed++;
                    }
                }
            }
        }

        std::cout << "==================== Inst Count ====================\n";
        size_t total = 0;
        for (auto op: _op_count) {
            std::cout << std::setw(20) << op.first << " insts: " << op.second << '\n';
            total += op.second;
        }
        std::cout << std::setw(20) << "parsed" << " insts: " << _parsed << "/" << total << '\n';
        std::cout << "\n";
    }
};

REGISTER_PASS(InstcountPass);