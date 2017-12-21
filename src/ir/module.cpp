//
// Created by GentlyGuitar on 6/6/2017.
//

#include <thread>
#include "irEssential.h"
#include <inst/instEssential.h>
#include <di/diEssential.h>
#include <utilities/flags.h>
#include <asmParser/sysDict.h>
#include <asmParser/llParser.h>

string Module::get_header(string key) {
     if (_headers.find(key) == _headers.end()) {
         return "";
     }
     else {
         return _headers[key];
     }
}

// Globals
void Module::append_new_global(string &text) {
    LLParser* parser = SysDict::parser;
    parser->set_line(text);
    parser->parse_globals(this);
}

Function* Module::get_function(string key) {
    // todo: here it assumes an aliasee must be a Function, which seems
    // ok for now but may not always be true
    if (Alias* alias = get_alias(key)) {
        return dynamic_cast<Function*>(alias->aliasee());
    }
//
//    if (_function_map.find(key) == _function_map.end()) {
//        return NULL;
//    } else {
//        return _function_map[key];
//    }

    if (_value_map.find(key) == _value_map.end()) {
        return NULL;
    } else {
        return dynamic_cast<Function*>(_value_map[key]);
    }
}

Function* Module::get_function_by_orig_name(string key) {
    // todo: here it assumes an aliasee must be a Function, which seems
    // ok for now but may not always be true
    if (Alias* alias = get_alias(key)) {
        return dynamic_cast<Function*>(alias->aliasee());
    }

    for (auto F: _function_list) {
        if (DISubprogram* sp = F->di_subprogram()) { // only consider those that have debug id            
            if (sp->linkageName() == key) {
                return F;
            }
        }
    }
    throw FunctionNotFoundError(key);
}

void Module::add_global_variable(GlobalVariable* gv) {
    _global_list.push_back(gv);
    guarantee(!gv->name().empty(), "");
    if (_value_map.find(gv->name()) != _value_map.end()) {
        throw SymbolRedefinitionError(gv->name());
    }
    _value_map[gv->name()] = gv;
}

GlobalVariable* Module::get_global_variable(string name) {
    if (_value_map.find(name) != _value_map.end()) {
        return dynamic_cast<GlobalVariable*>(_value_map[name]);
    }
    else {
        return NULL;
    }
}

void Module::set_as_resolved(Function *f) {
    guarantee(get_function(f->name()) != NULL, "function %s not in the symbol table", f->name().c_str());
    _function_list.push_back(f);
}

/**@brief This function will be called both during parsing and during executing passes.
 * The difference is that at parsing time, the current module's is_full_resolved is not
 * set while this flag will be set after parsing and resolving is done.
 *
 * @param pos
 * @param inserted
 */
void Module::insert_new_function(int pos, Function *inserted) {
    // todo: should throw an exception
    guarantee(pos < _function_list.size()+1, "inserted position out of range");
    guarantee(inserted != NULL, "inserted function is NULL");
    guarantee(inserted->parent() == NULL, "inserted function already belong to a module");

    if (get_function(inserted->name())) {
        string msg = "invalid redefinition of function "+ inserted->name();
        throw SymbolRedefinitionError(msg);
    }

    // iterate all the CallInstFamily of the inserted function
    if (is_fully_resolved()) {
        if (inserted->is_defined()) {
            for (auto bb: inserted->basic_block_list()) {
                for (auto ci: bb->callinst_list()) {
                    bb->check_insertion_side_effects_on_module(ci);
                }
            }
        }
    }

    auto& l = _function_list;
    l.insert(l.begin()+pos, inserted);
    inserted->set_parent(this);
    //_function_map[inserted->name()] = inserted;
    _value_map[inserted->name()] = inserted;
}

void Module::insert_function_before(Function *old, Function *inserted) {
    int i = 0;
    auto& l = _function_list;
    for (; i < l.size(); ++i) {
        if (l[i] == old) {
            break;
        }
    }

    // todo: should throw an exception
    guarantee(i != l.size(), "Insert position not found");
    insert_new_function(i, inserted);
}

void Module::insert_function_after(Function *old, Function *inserted) {
    int i = 0;
    auto& l = _function_list;
    for (; i < l.size(); ++i) {
        if (l[i] == old) {
            break;
        }
    }

    // todo: should throw an exception
    guarantee(i != l.size(), "Insert position not found");
    insert_new_function(i+1, inserted);
}


MetaData* Module::get_debug_info(int i) {
    return _unnamed_metadata_list.at(i);
//
//    if (i < 0 || i > _unnamed_metadata_list.size()) {
//        return NULL;
//    }
//    else {
//        return _unnamed_metadata_list[i];
//    }
}

void Module::resolve_debug_info() {
    std::vector<DILocation*> more;
    for (auto i: unnamed_metadata_list()) {
        i->resolve_refs();
        if (DILocation* loc = dynamic_cast<DILocation*>(i)) {
            more.push_back(loc);
        }
    }

    for (auto i: more) {
        i->second_resolve();
    }

//    for (auto i: unnamed_metadata_list()) {
//        i->resolve();
//    }
//
//    for (auto i: unnamed_metadata_list()) {
//        if (DILocation* loc = dynamic_cast<DILocation*>(i)) {
//            loc->second_resolve();
//        }
//    }
}

void Module::resolve_callinsts() {
    for (auto F: function_list()) {
        for (auto B: F->basic_block_list()) {
            B->resolve_callinsts();
//            for (auto I: B->callinst_list()) {
//                if (I->is_indirect_call()) {
//                    I->try_resolve_indirect_call();
//                }
//                else {
//                    I->resolve_direct_call();
//                }
//            }
        }
    }
}

void Module::resolve_aliases() {
    for (auto it: _alias_map) {
        Alias* a = it.second;
        Function* f = get_function(a->get_raw_field("aliasee"));
        guarantee(f, " ");
        a->set_aliasee(f);
    }
}

void Module::resolve_callinsts_and_aliases() {
    resolve_callinsts();
    resolve_aliases();
}

void Module::resolve_after_parse() {
    if (1) {
    //if (ParallelModule) {
        std::thread t1(&Module::resolve_debug_info, this);
        //std::thread t1([=] { resolve_debug_info();});
        resolve_callinsts();
        resolve_aliases();
        t1.join();
    }
    else {
        resolve_callinsts();
        resolve_aliases();
        resolve_debug_info();
    }
    set_is_fully_resolved();
}

void Module::print_to_stream(std::ostream &os) {
    os << "; ModuleID = '" << module_id() << "'\n";
    for (auto pair: headers()) {
        os << pair.first << " = \"" << pair.second << "\"\n";
    }
    os << '\n';

    if (!module_level_inline_asms().empty()) {
        for (auto s: module_level_inline_asms()) {
            os << s << '\n';
        }
        os << '\n';
    }

    for (auto i: struct_list()) { os << i; }           os << '\n';
    for (auto i: comdat_list()) { os << i; }           os << '\n';
    for (auto i: global_list()) { os << i; }           os << '\n';
    for (auto i: alias_map()) { os << i.second; }      os << '\n';
    for (auto i: function_list()) { os << i; }         os << '\n';
    for (auto i: attribute_list()) { os << i; }        os << '\n';
    for (auto i: named_metadata_map()) { os << i.second; }        os << '\n';
    for (auto i: unnamed_metadata_list()) { os << i; }            os << '\n';
}

void Module::print_to_stream(FILE *fp) {
    fprintf(fp, "; ModuleID = '%s'\n", module_id().c_str());

    for (auto pair: headers()) {
        fprintf(fp, "%s = \"%s\"\n", pair.first.c_str(), pair.second.c_str());
    }
    fprintf(fp, "\n");

    for (auto s: module_level_inline_asms()) {
        fprintf(fp, "%s\n", s.c_str());
    }
    fprintf(fp, "\n");

    for (auto st: struct_list()) {
        st->print_to_stream(fp);
    }
    fprintf(fp, "\n");

    auto& l2a = _comdat_list;
    for (auto it = l2a.begin(); it != l2a.end(); ++it) {
        (*it)->print_to_stream(fp);
    }
    fprintf(fp, "\n");

    auto& l2 = _global_list;
    for (auto it = l2.begin(); it != l2.end(); ++it) {
        (*it)->print_to_stream(fp);
    }
    fprintf(fp, "\n");

    for (auto it: _alias_map) {
        it.second->print_to_stream(fp);
    }
    fprintf(fp, "\n");

    auto& l3 = _function_list;
    for (auto f = l3.begin(); f != l3.end(); ++f) {
        (*f)->print_to_stream(fp);
        fprintf(fp, "\n");
    }

    auto& l4 = _attribute_list;
    for (auto a = l4.begin(); a != l4.end(); ++a) {
        (*a)->print_to_stream(fp);
    }
    fprintf(fp, "\n");

    auto& mm = _named_metadata_map;
    for (auto md = mm.begin(); md != mm.end(); ++md) {
        md->second->print_to_stream(fp);
    }

    auto& uml = _unnamed_metadata_list;
    for (auto md = uml.begin(); md != uml.end(); ++md) {
        (*md)->print_to_stream(fp);
    }
}

void Module::check_after_parse() {
//    guarantee(_function_map.size() == _function_list.size(),
//              "map size: %d, list size: %d\n", _function_map.size(), _function_list.size());

    for (auto F: _function_list) {
//        if (!F->is_defined() && !F->is_external()) {
//            printf("unresolved function: %s\n", F->name().c_str());
//        }
        if (CheckDebugInfo) {
//            if (F->is_defined()) {
//                guarantee(F->di_subprogram(), "Function %s does not have debug info", F->name_as_c_str());  // wrong assumption
//            }
        }
    }

}
