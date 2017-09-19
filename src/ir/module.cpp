//
// Created by GentlyGuitar on 6/6/2017.
//


#include "irEssential.h"
#include <inst/instEssential.h>
#include <di/diEssential.h>

string Module::get_header(string key) {
     if (_headers.find(key) == _headers.end()) {
         return "";
     }
     else {
         return _headers[key];
     }
}

Function* Module::get_function(string key) {
    if (Alias* alias = get_alias(key)) {
        return dynamic_cast<Function*>(alias->aliasee());
    }
    if (_function_map.find(key) == _function_map.end()) {
        return NULL;
    } else {
        return _function_map[key];
    }
}

Function* Module::create_child_function_symbol(string name) {
    Function* f = new Function();
    f->set_name(name);
    f->set_parent(this);
    _function_map[name] = f;

    return f;
}

Function* Module::create_child_function(string name) {
    Function* f = create_child_function_symbol(name);
    set_as_resolved(f);

    return f;
}

void Module::set_as_resolved(Function *f) {
    guarantee(get_function(f->name()) != NULL, "function %s not in the symbol table", f->name().c_str());
    _function_list.push_back(f);
}

/// new function mean it is not currently in the map
void Module::insert_new_function(int pos, Function *inserted) {
    // todo: should throw an exception
    guarantee(pos < _function_list.size()+1, "inserted position out of range");
    guarantee(inserted != NULL, "inserted function is NULL");
    guarantee(inserted->parent() == NULL, "inserted function already belong to a module");

    if (_function_map.find(inserted->name()) != _function_map.end()) {
        string msg = "invalid redefinition of function "+ inserted->name();
        throw SymbolRedefinitionError(msg);
    }

    // iterate all the CallInst of the inserted function
    if (inserted->is_defined()) {
        for (auto bit = inserted->begin(); bit != inserted->end(); ++bit) {
            BasicBlock* bb = *bit;
            auto& l = bb->callinst_list();
            for (int i = 0; i < l.size(); ++i) {
                CallInst* ci = dynamic_cast<CallInst*>(l[i]);
                guarantee(ci, "Should be of CallInst class");
                Function* callee = ci->called_function();
                if (callee) {
                    callee->append_user(ci);
                }

            }
        }
    }
    auto& l = _function_list;
    l.insert(l.begin()+pos, inserted);
    inserted->set_parent(this);
    _function_map[inserted->name()] = inserted;

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
    auto& l = _unnamed_metadata_list;
    for (auto it = l.begin(); it != l.end(); ++it) {
        (*it)->resolve();
        DILocation* loc = dynamic_cast<DILocation*>(*it);
        if (loc) {
            more.push_back(loc);
        }
    }

    for (auto it = more.begin(); it != more.end(); ++it) {
        (*it)->second_resolve();
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

void Module::print_to_stream(std::ostream &ofs) {
    ofs << "; ModuleID = '" << module_id() << "'\n";
    for (auto pair: headers()) {
        ofs << pair.first << " = \"" << pair.second << "\"\n";
    }
    ofs << '\n';

    if (!module_level_inline_asms().empty()) {
        for (auto s: module_level_inline_asms()) {
            ofs << s << '\n';
        }
        ofs << '\n';
    }

    for (auto i: struct_list()) { ofs << i; }           ofs << '\n';
    for (auto i: global_list()) { ofs << i; }           ofs << '\n';
    for (auto i: comdat_list()) { ofs << i; }           ofs << '\n';
    for (auto i: alias_map()) { ofs << i.second; }      ofs << '\n';
    for (auto i: function_list()) { ofs << i; }         ofs << '\n';
    for (auto i: attribute_list()) { ofs << i; }        ofs << '\n';
    for (auto i: named_metadata_map()) { ofs << i.second; }        ofs << '\n';
    for (auto i: unnamed_metadata_list()) { ofs << i; }            ofs << '\n';
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

    auto& l2 = _global_list;
    for (auto it = l2.begin(); it != l2.end(); ++it) {
        (*it)->print_to_stream(fp);
    }
    fprintf(fp, "\n");

    auto& l2a = _comdat_list;
    for (auto it = l2a.begin(); it != l2a.end(); ++it) {
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
    if (_function_map.size() != _function_list.size()) {
        printf("map size: %d, list size: %d\n", _function_map.size(), _function_list.size());
    }
    bool all_resolved = 1;
    auto& m = _function_map;
    for (auto it = m.begin(); it != m.end(); ++it) {
        Function* f = it->second;
        if (!f->is_defined() && !f->is_external()) {
            printf("unresolved function: %s\n", f->name().c_str());
        }
    }
//
//    auto& l3 = _function_list;
//    for (auto f = l3.begin(); f != l3.end(); ++f) {
//        (*f)->dump();
//    }
}

void Module::check_after_pass() {
    for (auto F: function_list()) {

    }
}