//
// Created by GentlyGuitar on 6/6/2017.
//

#include <algorithm>
#include <utilities/strings.h>
#include <asmParser/sysDict.h>
#include "irEssential.h"

Function::Function(): Value() {
    _is_external = false;
    _is_defined = false;
    _parent = NULL;
    _entry_block = NULL;
    _dbg_id = -1;
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

/**@brief the copy is exactly the same as "this", except that
 * 1. it has no parent (not in the module yet)
 * 2. it has no users
 * 3. the name of the copy would be the old name + "." + a number, indicated by _copy_cnt
 * 4. the subprogram debug info is stripped, if any (llvm 4 does not allow two functions have the same DISubproggram)
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
Function* Function::clone(string new_name) {
    Function* f = new Function(*this);
    f->set_parent(NULL);
    f->user_list().clear();
    if (is_defined()) {
        for (iterator it = f->begin(); it != f->end(); ++it) {
            // don't delete the old basic block, it is still used by the original function
            BasicBlock* old = *it;
            BasicBlock* neu = old->clone();
            *it = neu;
            neu->set_parent(f);
        }
        _entry_block = *(f->begin());
    }

    if (new_name.empty()) {
        new_name = name()+'.'+std::to_string((long long)++_copy_cnt);
    }
    f->rename(new_name);

    zpl("cloned %s to %s", name_as_c_str(), f->name_as_c_str())

    /* strip DISubprogram info */
    int dipos = raw_text().find("!dbg");
    if (dipos != string::npos) {
        string new_header = raw_text().substr(0, dipos) + '{';
        set_raw_text(new_header);
    }
    return f;
}


/**@brief change the name of a new created function
 *
 * @param name
 */
void Function::rename(string name) {
    if (parent() == NULL) {
        if (SysDict::module()->get_function(name)) {
            throw SymbolTableError("Cannot rename function '"+_name+"' to '"+name+"', this name already exists in the symbol table!");
        }
        string raw = raw_text();
        string old = '@' + _name;
        string neu = '@' + name;
        Strings::replace(raw, old, neu);
        set_raw_text(raw);
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
        //fprintf(fp, "%s {\n", raw_text().c_str());
        fprintf(fp, "%s\n", raw_text().c_str());
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
