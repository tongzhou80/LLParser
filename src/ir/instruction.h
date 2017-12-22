//
// Created by GentlyGuitar on 6/7/2017.
//

#ifndef LLPARSER_INSTRUCTION_H
#define LLPARSER_INSTRUCTION_H

#include <peripheral/timer.h>
#include <di/diLocation.h>
#include "value.h"

class BasicBlock;
class Function;

class DILocation;

class InstParser;

class Instruction: public Value {
public:
    enum InstType {
        UnknownInstType,
        AllocaInstType,
        LoadInstType,
        StoreInstType,
        BranchInstType,
        CallInstType,
        InvokeInstType,
        BitCastInstType,
        GetElementPtrInstType,
    };

protected:
    InstParser* _parser;
    InstType _type;
    string _opcode;
    BasicBlock* _parent;
    bool _has_assignment;
    int _dbg_id;
    DILocation* _debug_loc;
    string _owner;  // mainly for debug
public:
    Instruction();

    const string& opcode()                    { return _opcode; }
    void set_opcode(string op)                { _opcode = op; }

    string owner()                            { return _owner; }  // unsafe
    void set_owner(string owner)              { _owner = owner; }

    InstParser* parser()                      { return _parser; }

    InstType type()                           { return _type; }
    void set_type(InstType t)                 { _type = t; }

    int dbg_id()                              { return _dbg_id; }
    void set_dbg_id(int i)                    { _dbg_id = i; }

    DILocation* debug_loc();

    BasicBlock* parent() const                { return _parent; }
    void set_parent(BasicBlock* bb)           { _parent = bb; }
    Module* module() const;

    bool has_assignment()                     { return _has_assignment; }
    void set_has_assignment(bool v=true)      { _has_assignment = v; }

    Function* function();
    void copy_metadata_from(Instruction* i);

    /** @brief Return the index of this instruction in its parent's instruction list
     *
     * @return
     */
    int get_index_in_block();

    /** @brief Return a Point2D instance
     *
     * @return
     */
    Point2D<int> get_position_in_function();

    /** @brief Clone this instruction except the parent.
     *
     * Each specific instruction should override clone().
     * @return
     */
    virtual Instruction* clone();
//    Function* function()                      { return _func; }
//    void set_function(Function* f)            { _func = f; }
};


#endif //LLPARSER_INSTRUCTION_H
