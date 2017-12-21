//
// Created by GentlyGuitar on 6/7/2017.
//

#ifndef LLPARSER_METADATA_H
#define LLPARSER_METADATA_H

#include "value.h"

class Module;

class MetaData: public Value {
protected:
    bool _is_resolved;
    int _number; //  for unnamed metadata
    Module* _parent;
public:
    MetaData(): _is_resolved(false), _number(-1), _parent(NULL) {}

    void set_number(int n)                                       { _number = n; }
    int number()                                                 { return _number; }

    Module* parent()                                             { return _parent; }
    void set_parent(Module* m)                                   { _parent = m; }

    MetaData* get_reference_by_value(string md_id);
    MetaData* get_reference_by_key(string field);

    virtual void resolve_non_refs() {}
    virtual void resolve_refs() {}
    bool is_resolved()                                       { return _is_resolved; }
    void set_is_resolved(bool v=1)                           { _is_resolved = v; }
};


#define DI_SET_REF_FIELD(field, type) \
if (has_raw_field(#field)) { \
    _##field = dynamic_cast<type*>(get_reference_by_key(#field)); \
    guarantee(_##field, "%s: %s is not of type " #type, #field, get_raw_field(#field).c_str()); \
}

#define DI_SET_INT_FIELD(field) \
if (has_raw_field(#field)) { \
    _##field = std::stoi(get_raw_field(#field)); \
}

#define DI_SET_STR_FIELD(field) \
if (has_raw_field(#field)) { \
    std::string s = get_raw_field(#field); \
    _##field = s.substr(1, s.size()-2); \
}

#endif //LLPARSER_METADATA_H
