//
// Created by GentlyGuitar on 6/6/2017.
//

#include <algorithm>
#include <utilities/strings.h>
#include <asmParser/sysDict.h>
#include <utilities/flags.h>
#include <di/diSubprogram.h>

Function::Function(): Value() {
    _is_external = false;
    _is_defined = false;
    _parent = NULL;
    _entry_block = NULL;
    _dbg_id = -1;
    _di_subprogram = NULL;
    _is_clone = false;
}

BasicBlock* Function::create_basic_block(string label) {
    BasicBlock* bb = create_basic_block();
    bb->set_name(label);
    return bb;
}

BasicBlock* Function::create_basic_block() {
    BasicBlock* bb = new BasicBlock();
    bb->set_parent(this);
    append_basic_block(bb);
    return bb;
}

int Function::get_basic_block_index(BasicBlock *bb) {
    auto& l = _basic_block_list;
    if (l.empty()) {
        return -1;
    }
    else {
        return (int)std::distance(l.begin(), std::find(l.begin(), l.end(), bb));
    }
}

Instruction* Function::get_instruction(int bi, int ii) {
    BasicBlock* bb = _basic_block_list.at(bi);
    return bb->instruction_list().at(ii);
}

Instruction* Function::get_instruction(Point2D<int> &pos) {
    BasicBlock* bb = _basic_block_list.at(pos.x);
    return bb->instruction_list().at(pos.y);
}

std::vector<CallInstFamily*> Function::caller_list() {
    std::vector<CallInstFamily*> callers;
    for (auto I: user_set()) {
        if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(I)) {
            callers.push_back(ci);
        }
    }
    return callers;
}

std::size_t Function::instruction_count() {
    std::size_t cnt = 0;
    for (auto B: basic_block_list()) {
        cnt += B->instruction_list().size();
    }
    return cnt;
}

/**@brief the copy is exactly the same as "this", except that
 * 1. it has no parent (not in the module yet)
 * 2. it has no users
 * 3. the name of the copy would be the old name + "." + a number, indicated by _copy_cnt
 * 4. the subprogram debug info is stripped, if any (llvm 4 does not allow two functions have the same DISubproggram)
 * 5. some other info are also stripped to avoid collision
 *
 * A pseudo debug info will be created for the new function, that is,
 * the new function's debug_id will be set to -1, but its _di_subprogram will be set
 * to an object that only has "name" and "file" fields.
 *
 * All instruction's debug info stay the same, so they will share a same debug info entry
 * with the old instructions in the old function.
 *
 * Function cloning is a process that could have multiple side effects. The cloned function
 * may or may not be inserted to the module immediately after creation. It will not affect
 * the call graph until it is inserted.
 *
 * Since the cloned function is new function, it can't be used before, so any calls
 * to the cloned function must happen after the cloning.
 *
 * The side effects of function's calls to other functions should happen in register time
 * (when it is inserted to the module).
 *
 * @return
 */
//todo: may need to create a Function from scratch
Function* Function::clone(string new_name) {
    Function* copy = new Function(*this);
    copy->set_parent(NULL);
    copy->user_set().clear();
    if (is_defined()) {
        for (auto it = copy->begin(); it != copy->end(); ++it) {
            // don't delete the old basic block, it is still used by the original function
            BasicBlock* old = *it;
            BasicBlock* neu = old->clone();
            *it = neu;
            neu->set_parent(copy);
        }
        _entry_block = *(copy->begin());
    }

    if (new_name.empty()) {
        new_name = name()+'.'+std::to_string(++_copy_cnt);
        //new_name = name()+".c."+std::to_string(++_copy_cnt);  // ".c" is inserted
    }
    copy->rename(new_name);

    //zpl("cloned %s to %s", name_as_c_str(), copy->name_as_c_str())

    /* strip DISubprogram info */
    int dipos = copy->raw_text().find("!dbg");
    if (dipos != string::npos) {
        string new_header = copy->raw_text().substr(0, dipos);
        copy->set_raw_text(new_header);
        copy->set_dbg_id(-1);
    }

    Strings::ireplace(copy->raw_text(), " comdat ", " ");  // todo

    /* create new debug info for the cloned function
     * except the name, other fields remain the same for now
     */
    if (!this->di_subprogram()) {
        this->dump();
        guarantee(0, "");
    }
    
    auto dbg_copy = new DISubprogram(*(this->di_subprogram()));
    dbg_copy->set_name(copy->name());
    copy->set_di_subprogram(dbg_copy);

    copy->set_is_clone();
    copy->set_copy_cnt(0);
    copy->set_copy_prototype(this);

    return copy;
}


/**@brief Change the name of a new created function
 *
 * Will not check name collision because the function is not inserted yet
 * @param name
 */
void Function::rename(string name) {
    if (parent() == NULL) {
        string& raw = raw_text();
        string old = '@' + _name;
        string neu = '@' + name;
        Strings::ireplace(raw, old, neu);
        _name = name;
    }
    else {
        guarantee(0, " unimplemented");
    }
}

void Function::print_to_stream(FILE *fp) {
    fprintf(fp, ";\n");
    if (is_external()) {
        fprintf(fp, "%s\n", raw_text().c_str());
    }
    else {
        fprintf(fp, "%s {\n", raw_text().c_str());
        //fprintf(fp, "%s\n", raw_text().c_str());
        auto l = _basic_block_list;
        for (int i = 0; i < l.size(); ++i) {
            auto bb = l[i];
            (bb)->print_to_stream(fp);
            if (i + 1 != l.size()) {
                fprintf(fp, "\n");
            }
        }
        fprintf(fp, "}\n");
    }
}

void Function::print_to_stream(std::ostream& os) {
    os << ";\n";
    if (is_external()) {
        os << raw_text() << '\n';
    }
    else {
        os << raw_text() << " {\n";
        for (auto i: basic_block_list()) {
            i->print_to_stream(os);
        }
        os << "}\n";
    }
}


DISubprogram* Function::di_subprogram() {
    /* Cloned functions cloned the _di_subprogram from their prototype,
     * but the dbg_id is set to -1
     */
    if (is_clone()) {
        return _di_subprogram;
    }
    
    if (dbg_id() < 0) {
        return NULL;
    }

    if (!_di_subprogram) {
        MetaData* md = module()->get_debug_info(_dbg_id);
        _di_subprogram = dynamic_cast<DISubprogram*>(md);
    }

    return _di_subprogram;
}
