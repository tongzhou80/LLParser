//
// Created by tlaber on 6/11/17.
//

#ifndef LLPARSER_INSTFLAG_H
#define LLPARSER_INSTFLAG_H

#include <set>
#include <unordered_set>
#include <bits/unordered_set.h>
#include "../utilities/macros.h"

class InstFlags {
    static std::set<string> _fastmaths;
    static std::set<string> _linkages;
    static std::set<string> _cconvs;  // calling conventions
    static std::set<string> _visibilities;
    static std::set<string> _dll_storage_classes;
    static std::set<string> _param_attrs;  // attrs for return type and function parameter type
    static std::set<string> _tails;
    static std::set<string> _terminator_insts;

public:
    static void init();
    static std::set<string> fastmaths()                                   { return _fastmaths; }
    static bool is_fastmath_flag(const string& key)                       { return _fastmaths.find(key) != _fastmaths.end(); }
    //static bool is_fastmath_flag(string key);

    static std::set<string> linkages()                                    { return _linkages; }
    static bool is_linkage_flag(const string& key)                        { return _linkages.find(key) != _linkages.end(); }

    static std::set<string> cconvs()                                      { return _cconvs; }
    static bool is_cconv_flag(const string& key);

    static std::set<string> visibilities()                                { return _visibilities; }
    static bool is_visibility_flag(const string& key)                     { return _visibilities.find(key) != _visibilities.end(); }

    static bool is_dll_storage_class_flag(const string& key)              { return _dll_storage_classes.find(key) != _dll_storage_classes.end(); }

    static bool is_param_attr_flag(const string& key);
    static bool is_tail_flag(const string& key)                           { return _tails.find(key) != _tails.end(); }
    //static bool is_tail_flag1(string key);

    static bool is_terminator_inst(const string& key)                     { return _terminator_insts.find(key) != _terminator_insts.end(); }
};

#endif //LLPARSER_INSTFLAG_H
