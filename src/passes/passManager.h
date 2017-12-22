//
// Created by GentlyGuitar on 6/8/2017.
//

#ifndef LLPARSER_PASSMANAGER_H
#define LLPARSER_PASSMANAGER_H

#include <vector>
#include <type_traits>
#include <ir/module.h>
#include "../ir/instruction.h"

class Pass;
class Instruction;
class BasicBlock;
class Function;
class Module;


class PassManager {
//    std::vector<Pass*> _parse_time_passes;
//    std::vector<Pass*> _custom_passes;
//    PassList* _parse_time_passes;
//    PassList* _passes;
    std::vector<Pass*> _global_passes;
    std::vector<Pass*> _module_passes;
    std::vector<Pass*> _function_passes;
    std::vector<Pass*> _basic_block_passes;
    std::vector<Pass*> _instruction_passes;
    string _pass_lib_path;
public:
    PassManager();
    ~PassManager();
    void initialize_passes();
    void add_parse_time_pass(Pass* p);
    void add_pass(Pass* p);
    void add_pass(string name);
    Pass* load_pass(string name);

    void insert_with_priority(std::vector<Pass*>& list, Pass* p);

    void apply_global_passes();
    void apply_module_passes(Module* module);
    void apply_function_passes(Function* func);
    void apply_basic_block_passes(BasicBlock* bb);
    void apply_instruction_passes(Instruction* inst);

    void apply_passes(Module* module);

    void apply_initializations();
    void apply_finalization();
    void apply_initializations(Module* module);
    void apply_finalization(Module* module);
    void apply_initializations(Function* function);
    void apply_finalization(Function* function);

    Pass* get_pass(std::vector<Pass*>& passes, int i);

    static PassManager* pass_manager;
    static void init();
    static void destroy();

    /* deprecated */
    void apply_parse_time_passes(Module* module);
    void apply_parse_time_passes(Function* func);
    void apply_parse_time_passes(BasicBlock* bb);
    void apply_parse_time_passes(Instruction* inst);
    void apply_parse_epilogue();
};


//template <typename T>
//void PassManager::apply_parse_time_passes(T *unit) {
//    _parse_time_passes->apply_passes(unit);
//}






#endif //LLPARSER_PASSMANAGER_H
