//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_VALUE_H
#define LLPARSER_VALUE_H

#include <map>
#include <set>
#include "shadow.h"
#include "../utilities/macros.h"

class Instruction;

class Value: public Shadow {
public:
    typedef std::vector<Instruction*> InstList;
    typedef InstList::iterator inst_iterator;
    typedef std::set<Instruction*> InstSet;
protected:
    string _name;
    //std::map<string, string> _properties;
    int _copy_cnt;
    Value* _copy_prototype;
    InstSet _users;
public:
    Value();
    virtual string name() const                          { return _name; }
    virtual const char* name_as_c_str() const            { return _name.c_str(); };
    virtual void set_name(string name)                   { _name = name; }

    int copy_cnt()                                       { return _copy_cnt; }
    void set_copy_cnt(int cnt)                           { _copy_cnt = cnt; }

    Value *copy_prototype() const {
        return _copy_prototype;
    }

    void set_copy_prototype(Value *_copy_prototype) {
        Value::_copy_prototype = _copy_prototype;
    }

    string prototype_name();

    /* caller/user interfaces */
    void append_user(Instruction* user);

    InstSet& user_set()                                   { return _users; }
    void remove_user(Instruction* user);


//    std::map<string, string> properties()                { return _properties; };
//    bool has_property(string key);
//    string get_properties(string key);
//    void set_property(string key, string value);

    virtual void print_to_file(const char* file);
    virtual void print_to_file(string file);
    virtual void print_to_stream(std::ostream& ofs);
    virtual void print_to_stream(FILE* fp);
    friend std::ostream& operator<<(std::ostream&, Value*);
};

#endif //LLPARSER_VALUE_H
