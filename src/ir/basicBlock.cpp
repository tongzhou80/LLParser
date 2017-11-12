//
// Created by GentlyGuitar on 6/7/2017.
//
#include <algorithm>
#include "irEssential.h"
#include <inst/instEssential.h>
#include <asmParser/sysDict.h>

BasicBlock::BasicBlock(): Value() {
    // _instruction_list.reserve()
    _parent = NULL;
    _is_entry = false;
    _is_exit = false;
    _instruction_list.reserve(16);
}

void BasicBlock::append_instruction(Instruction *ins) {
    insert_instruction(instruction_list().size(), ins);
}

/**@brief This function will be called both during parsing and during executing passes.
 *
 * This function will try to resolve indirect call chains, but won't resolve callees.
 * A callee is only resolved after the parsing, or at the point when a new function
 * after the module is fully parsed.
 *
 * @param pos
 * @param ins
 */
void BasicBlock::insert_instruction(int pos, Instruction *ins) {
    guarantee(pos <= _instruction_list.size(), "insertion point out of range");
    auto& l = _instruction_list;
    l.insert(l.begin()+pos, ins);
    ins->set_parent(this);

    if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(ins)) {
        _callinst_list.push_back(ci);
        if (ci->is_indirect_call() && instruction_list().size() > 2) {
            Instruction* prev_ins = _instruction_list[_instruction_list.size()-2];
            //if (BitCastInst* bci = dynamic_cast<BitCastInst*>(prev_ins)) {
            if (prev_ins->name() == '%' + ci->called_label()) {
                ci->set_chain_inst(prev_ins);
            }
            //}
        }
    }
}

void BasicBlock::resolve_callinsts() {
    for (auto I: callinst_list()) {
        if (I->is_indirect_call()) {
            I->try_resolve_indirect_call();
        }
        else {
            I->resolve_direct_call();
        }
    }
}

void BasicBlock::check_insertion_side_effects_on_module(Instruction* ins) {
    if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(ins)) {
        Function* callee = ci->called_function();
        if (callee) {
            callee->append_user(ci);
        }
    }
}

void BasicBlock::check_deletion_side_effects_on_module(Instruction *ins) {
    if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(ins)) {
        Function* callee = ci->called_function();
        if (callee) {
            callee->remove_user(ci);
        }
    }
}

bool BasicBlock::insert_instruction_after(Instruction *old, Instruction *neu) {
    int i = 0;
    auto& l = _instruction_list;
    for (; i < l.size(); ++i) {
        if (l[i] == old) {
            break;
        }
    }

    if (i == l.size()) {
        return false;
    }
    else {
        insert_instruction(i+1, neu);
        return true;
    }
}

bool BasicBlock::insert_instruction_after(Instruction *old, InstList& neus) {
    int i = 0;
    auto& l = _instruction_list;
    for (; i < l.size(); ++i) {
        if (l[i] == old) {
            break;
        }
    }

    if (i == l.size()) {
        return false;
    }
    else {
        for (int j = 0; j < neus.size(); ++j) {
            Instruction* newinst = neus[j];
            insert_instruction(i+1, newinst);
            ++i;
        }

        return true;
    }
}

bool BasicBlock::insert_instruction_before(Instruction *old, Instruction *neu) {
    int i = 0;
    auto& l = _instruction_list;
    for (; i < l.size(); ++i) {
        if (l[i] == old) {
            break;
        }
    }

    if (i == l.size()) {
        return false;
    }
    else {
        insert_instruction(i, neu);
        return true;
    }

}

int BasicBlock::get_instruction_index(Instruction * inst) {
    auto& l = _instruction_list;
    if (l.empty()) {
        return -1;
    }
    else {
        return (int)std::distance(l.begin(), std::find(l.begin(), l.end(), inst));
    }
}

int BasicBlock::get_index_in_function() {
    guarantee(parent() && parent()->is_defined(), "must have a parent that is defined");
    return parent()->get_basic_block_index(this);
}

void BasicBlock::replace(iterator iter, Instruction *neu) {
    if (neu->parent()) {
        if (neu->parent() != this) {
            throw std::runtime_error("Instruction to be inserted belongs to a different basic block");
        }
    }
    else {
        neu->set_parent(this);
    }
    Instruction* old = *iter;
    *iter = neu;

    /* assume the module should be fully resolved */
    check_deletion_side_effects_on_module(old);
    check_insertion_side_effects_on_module(neu);

    old->set_parent(NULL);
    //delete old;
}

void BasicBlock::replace(Instruction *old, Instruction *neu) {
    auto it = begin();
    for (; it != end(); ++it) {
        if (*it == old) {
            break;
        }
    }
    if (it == end()) {
        throw InstructionNotFoundError();
    }

    replace(it, neu);
}

Module* BasicBlock::module() const {
    return parent()->parent();
}

/**@brief Returns a clone of this block.
 * Each instruction will be a new object instead of pointing to the old object.
 *
 * @return
 */
BasicBlock* BasicBlock::clone() {
    // callinst_list and inst_list need deep copy
    BasicBlock* bb = new BasicBlock(*this);
    bb->callinst_list().clear();
    bb->instruction_list().clear();
    for (auto I: instruction_list()) {
        bb->append_instruction(I->clone());
    }
//    for (auto it = bb->begin(); it != bb->end(); ++it) {
//        Instruction* old = *it;
//        if (old->raw_text().find("%89 = call i64 (i8*, i8*, ...) %88(i8* %86, i8* %87), !dbg !1777") != string::npos) {
//            zpl("gocha");
//        }
//        Instruction* neu = (*it)->clone();
//        *it = neu;
//        neu->set_parent(bb);
//
//        /* if instruction is CallInst, it changes the call graph upon insertion */
//        if (auto ci = dynamic_cast<CallInstFamily*>(neu)) {
//            bb->callinst_list().push_back(ci);
//        }
//    }

    return bb;
}

void BasicBlock::print_to_stream(FILE *fp) {
    if (!is_entry())
        fprintf(fp, "%s\n", raw_text().c_str());

    auto& l = _instruction_list;
    for (auto i = l.begin(); i != l.end(); ++i) {
        (*i)->print_to_stream(fp);
    }
}

void BasicBlock::print_to_stream(std::ostream &os) {
    //if (!is_entry())
        os << raw_text() << '\n';

    for (auto i: instruction_list()) {
        i->print_to_stream(os);
    }

    if (!is_exit())
        os << '\n';
}