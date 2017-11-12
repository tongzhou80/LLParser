//
// Created by GentlyGuitar on 6/7/2017.
//

#ifndef LLPARSER_BASICBLOCK_H
#define LLPARSER_BASICBLOCK_H

#include <vector>
#include <inst/callInstFamily.h>
#include "value.h"
#include "irEssential.h"

class Instruction;
class Function;

class BasicBlock: public Value {
    std::vector<Instruction*> _instruction_list;
    Function* _parent;
    bool _is_entry;
    bool _is_exit;
    std::vector<BasicBlock*> _preds;
    std::vector<string> _pred_labels;
    std::vector<BasicBlock*> _successors;
    std::vector<CallInstFamily*> _callinst_list;  // The functions called by this function
public:
    typedef std::vector<Instruction*> InstList;
    typedef InstList::iterator inst_iterator;


    BasicBlock();

    typedef std::vector<Instruction*>::iterator iterator;
    std::vector<Instruction*>& instruction_list()     { return _instruction_list; }

    /// Adding or removing elements will invalidate this iterator
    iterator begin()                                  { return _instruction_list.begin(); }
    iterator end()                                    { return _instruction_list.end(); }
    int inst_num()                                    { return _instruction_list.size(); }

    void append_instruction(Instruction* ins);
    void insert_instruction(int pos, Instruction* inst);
    void check_insertion_side_effects_on_module(Instruction* ins);
    void check_deletion_side_effects_on_module(Instruction* ins);
    bool insert_instruction_before(Instruction* old, Instruction* neu);
    bool insert_instruction_after(Instruction* old, Instruction* neu);
    bool insert_instruction_after(Instruction* old, InstList& neus);
    void resolve_callinsts();

    void replace(iterator iter, Instruction* neu);
    void replace(Instruction* old, Instruction* neu);

    int get_instruction_index(Instruction* inst);
    int get_index_in_function();

    Function* parent() const                              { return _parent; }
    void set_parent(Function* f)                          { _parent = f; }
    Module* module() const;

    bool is_entry()                                       { return _is_entry; }
    bool set_is_entry(bool v=1)                           { _is_entry = v; }

    bool is_exit()                                        { return _is_exit; }
    bool set_is_exit(bool v=1)                            { _is_exit = v; }

    void append_pred(string label)                        { _pred_labels.push_back(label); }

    /**@ Return a list of CallInstFamily* in this block, original order not guaranteed
     *
     * @return
     */
    std::vector<CallInstFamily*>& callinst_list()         { return _callinst_list; }

    /**@brief return a clone of this basic block, except the parent of the copy if NULL
     *
     * @return
     */
    BasicBlock* clone();

    void print_to_stream(FILE* fp);
    void print_to_stream(std::ostream& ofs);
};

#endif //LLPARSER_BASICBLOCK_H
