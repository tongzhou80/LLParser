//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_SHADOW_H
#define LLPARSER_SHADOW_H

#include <string>
#include <map>
#include "../utilities/macros.h"

class Shadow {
protected:
    string _raw_text;
    bool _has_raw_text;
    bool _fully_parsed;
    std::map<string, string> _raw_fields;
public:
    Shadow(): _has_raw_text(false), _fully_parsed(false) {}

    string& raw_text()           { return _raw_text; }
    const char* raw_c_str()      { return _raw_text.c_str(); }
    bool has_raw_text()          { return _has_raw_text; }
    bool fully_parsed()          { return _fully_parsed; }

    virtual void init_raw_field()                                 {}
    void set_raw_field(string field, string value)                { _raw_fields[field] = value; }
    bool has_raw_field(string field)                              { return _raw_fields.find(field) != _raw_fields.end(); }
    string get_raw_field(string field)                            { return _raw_fields[field]; }
    void update_raw_field(string field, string new_value);

    void set_raw_text(string text)      { _raw_text = text; }
    void append_raw_text(string text)   { _raw_text += text; }
    void set_has_raw_text(bool has)     { _has_raw_text = has; }
    void set_fully_parsed(bool fully)   { _fully_parsed = fully; }


    virtual void dump()                 { std::cout << raw_text() << std::endl; }
};

#endif //LLPARSER_SHADOW_H
