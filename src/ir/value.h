//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_VALUE_H
#define LLPARSER_VALUE_H

#include <map>
#include "shadow.h"
#include "../utilities/macros.h"

class Instruction;

class Value: public Shadow {
protected:
    string _name;
    //std::map<string, string> _properties;
    int _copy_cnt;
    std::vector<Instruction*> _users; 
public:
    Value();
    virtual string name()                                { return _name; }
    virtual const char* name_as_c_str()                  { return _name.c_str(); };
    virtual void set_name(string name)                   { _name = name; }

    /* caller/user interfaces */
    void append_user(Instruction* user)                    { _users.push_back(user); }
    typedef std::vector<Instruction*> InstList;
    typedef InstList::iterator inst_iterator;
    InstList& user_list()                                  { return _users; }
    inst_iterator user_begin()                             { return _users.begin(); }
    inst_iterator user_end()                               { return _users.end(); }
    void remove_user(Instruction* user);


//    std::map<string, string> properties()                { return _properties; };
//    bool has_property(string key);
//    string get_properties(string key);
//    void set_property(string key, string value);

    virtual void print_to_file(const char* file);
    virtual void print_to_file(string file);
    virtual void print_to_stream(std::ofstream& ofs);
    virtual void print_to_stream(FILE* fp);
};

#endif //LLPARSER_VALUE_H
