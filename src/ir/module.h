//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_MODULE_H
#define LLPARSER_MODULE_H

#include <map>
#include <vector>
#include <fstream>
#include "value.h"
#include "../utilities/macros.h"
#include "comdat.h"

class StructType;
class Comdat;
class GlobalVariable;
class Alias;
class Function;
class Attribute;
class MetaData;

class DILocation;

// todo: to add a routine to destroy a Module
class Module: public Value {
public:
    enum Language {
        c,
        cpp,
        f90
    };
private:
    Language _lang;
    string _input_file;
    string _module_id;
    bool _is_fully_resolved;
    std::map<string, string> _headers;
    std::vector<string> _module_level_inline_asms;
    std::vector<StructType*> _struct_list;
    std::vector<Comdat*> _comdat_list;
    std::vector<GlobalVariable*> _global_list;
    std::map<string, Alias*> _alias_map;
    std::vector<Function*> _function_list;  // guaranteed in the original order
    std::map<string, Function*> _function_map;  // for symbol resolving
    std::vector<Attribute*> _attribute_list;
    //std::vector<MetaData*> _metadata_list;
    std::map<string, MetaData*> _named_metadata_map;
    std::vector<MetaData*> _unnamed_metadata_list;
public:

    Module(): _lang(Language::c), _is_fully_resolved(false) {}

    Module::Language language()                            { return _lang; }
    void set_language(Language l)                          { _lang = l; }

    const string &input_file() const {
        return _input_file;
    }

    void set_input_file(const string &_input_file) {
        Module::_input_file = _input_file;
    }

    std::map<string, string> &headers() {
        return _headers;
    }

    void set_headers(const std::map<string, string> &_headers) {
        Module::_headers = _headers;
    }

    bool is_fully_resolved()                                   { return _is_fully_resolved; }
    void set_is_fully_resolved(bool v=1)                       { _is_fully_resolved = v; }

    std::vector<string> &module_level_inline_asms() {
        return _module_level_inline_asms;
    }

    void set_module_level_inline_asms(const std::vector<string> &_module_level_inline_asms) {
        Module::_module_level_inline_asms = _module_level_inline_asms;
    }

    std::vector<StructType *> &struct_list() {
        return _struct_list;
    }

    void set_struct_list(const std::vector<StructType *> &_struct_list) {
        Module::_struct_list = _struct_list;
    }

    std::vector<Comdat *> &comdat_list() {
        return _comdat_list;
    }

    void set_comdat_list(const std::vector<Comdat *> &_comdat_list) {
        Module::_comdat_list = _comdat_list;
    }

    std::vector<GlobalVariable*>& global_list()            { return _global_list; }
    std::map<string, Alias*>& alias_map()                  { return _alias_map; }
    std::vector<Function*>& function_list()                { return _function_list; }
    std::map<string, Function*>& function_map()            { return _function_map; };
    std::vector<Attribute*>& attribute_list()              { return _attribute_list; }
    std::map<string, MetaData*>& named_metadata_map()      { return _named_metadata_map; };
    std::vector<MetaData*>& unnamed_metadata_list()        { return _unnamed_metadata_list; }

    string module_id()                                     { return _module_id; }
    void set_module_id(string id)                          { _module_id = id; }

    string get_header(string key);
    void set_header(string key, string value)              { _headers[key] = value; }

    void add_module_level_asm(string& s)                   { _module_level_inline_asms.push_back(s); }
    void add_struct_type(StructType* st)                   { _struct_list.push_back(st); }
    void add_comdat(Comdat* cd)                            { _comdat_list.push_back(cd); }
    void add_global_variable(GlobalVariable* gv)           { _global_list.push_back(gv); }
    void add_alias(string key, Alias* value)               { _alias_map[key] = value; }
    Alias* get_alias(string key)                           { if (_alias_map.find(key) == _alias_map.end()) { return NULL; } else { return _alias_map[key]; } }
    void append_new_function(Function* f)                  { insert_new_function(_function_list.size(), f); }
    void append_attribute(Attribute* att)                  { _attribute_list.push_back(att); }
    void set_named_metadata(string key, MetaData* md)      { _named_metadata_map[key] = md; }
    void append_unnamed_metadata(MetaData* md)             { _unnamed_metadata_list.push_back(md); }

    MetaData* get_debug_info(int i);

    Function* get_function(string key);
    Function* get_function_by_orig_name(string key);
    Function* create_child_function_symbol(string name);
    Function* create_child_function(string name);


    void insert_new_function(int pos, Function* inserted);
    void insert_function_after(Function* old, Function* inserted);
    void insert_function_before(Function* old, Function* inserted);

    void set_as_resolved(Function* f);

    void print_to_stream(std::ostream& ofs);
    void print_to_stream(FILE* fp);

    // check
    void check_after_parse();

    void resolve_after_parse();
    void resolve_callinsts();
    void resolve_debug_info();
    void resolve_aliases();
    void resolve_callinsts_and_aliases();  // just for parallel purpose
};



#endif //LLPARSER_MODULE_H
