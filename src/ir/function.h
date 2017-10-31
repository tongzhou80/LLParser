//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_FUNCTION_H
#define LLPARSER_FUNCTION_H

#include "value.h"
#include "irEssential.h"
#include "instruction.h"
#include "basicBlock.h"
#include <vector>
#include <inst/callInstFamily.h>

class BasicBlock;
class Module;

class Instruction;
class CallInst;

class DISubprogram;


class Function: public Value {
    Module* _parent;
    std::vector<BasicBlock*> _basic_block_list;
    bool _is_external;
    bool _is_defined; // a function is either external or defined in current module, or else it is unresolved
    BasicBlock* _entry_block;
    int _dbg_id;
    DISubprogram* _di_subprogram;

    bool _is_clone;
public:
    Function();

    typedef std::vector<BasicBlock*>::iterator iterator;
    std::vector<BasicBlock*>& basic_block_list()           { return _basic_block_list; }

    iterator begin()                                       { return _basic_block_list.begin(); }
    iterator end()                                         { return _basic_block_list.end(); }

    void append_basic_block(BasicBlock* bb)                { _basic_block_list.push_back(bb); }
    bool is_external()                                     { return _is_external; }
    void set_is_external()                                 { _is_external = true; }

    bool is_defined()                                      { return _is_defined; }
    void set_is_defined(bool v=1)                          { _is_defined = v; }

    Module* parent()                                       { return _parent; }
    void set_parent(Module* m)                             { _parent = m; }

    /* debug info */
    int dbg_id()                                           { return _dbg_id; }
    void set_dbg_id(int i)                                 { _dbg_id = i; }
    DISubprogram* di_subprogram();
    void set_di_subprogram(DISubprogram* sp)               { _di_subprogram = sp; }

    BasicBlock* create_basic_block(string label);
    BasicBlock* create_basic_block();

    BasicBlock* entry_block()                              { return _entry_block; }
    void set_entry_block(BasicBlock* bb)                   { _entry_block = bb; }

    int basic_block_num()                                  { return _basic_block_list.size(); }

    int get_basic_block_index(BasicBlock* bb);
    Instruction* get_instruction(int bi, int ii);
    Instruction* get_instruction(Point2D<int> &pos);
    std::size_t instruction_count();

    std::vector<CallInstFamily*> caller_list();

    Function* clone(string new_name="");
    void rename(string name);

    void print_to_stream(FILE* fp);
    void print_to_stream(std::ostream& ofs);

    bool is_clone()                                         { return _is_clone; }
    bool set_is_clone(bool v=1)                             { _is_clone = 1; }
};


#endif //LLPARSER_FUNCTION_H
