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

class Module: public Value {
public:
    enum Language {
        c,
        cpp,
        f90
    };
private:
    Language _lang;
    string _module_id;
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
    Module(): _lang(Language::c) {}

    Module::Language language()                            { return _lang; }
    void set_language(Language l)                          { _lang = l; }
    std::vector<GlobalVariable*>& global_list()            { return _global_list; }
    std::map<string, Alias*>& alias_map()                  { return _alias_map; }
    std::vector<Function*>& function_list()                { return _function_list; }
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
    Function* create_child_function_symbol(string name);
    Function* create_child_function(string name);


    void insert_new_function(int pos, Function* inserted);
    void insert_function_after(Function* old, Function* inserted);
    void insert_function_before(Function* old, Function* inserted);

    void set_as_resolved(Function* f);

    //void print_to_stream(std::ofstream& ofs);
    void print_to_stream(FILE* fp);

    // check
    void check_after_parse();
    void check_after_pass();

    void resolve_debug_info();
    void resolve_aliases();
};



#endif //LLPARSER_MODULE_H
