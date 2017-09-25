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
    static std::set<string> _visibility;
    static std::set<string> _dll_storage_class;
    static std::set<string> _param_attrs;  // attrs for return type and function parameter type
    static std::set<string> _tails;
    static std::set<string> _terminator_insts;

public:
    static void init();
    static std::set<string> fastmaths()                        { return _fastmaths; }
    static bool in_fastmaths(const string& key)                       { return _fastmaths.find(key) != _fastmaths.end(); }
    //static bool in_fastmaths(string key);

    static std::set<string> linkages()                         { return _linkages; }
    static bool in_linkages(const string& key)                        { return _linkages.find(key) != _linkages.end(); }

    static std::set<string> cconvs()                           { return _cconvs; }
    static bool in_cconvs(const string& key);

    static std::set<string> visibility()                       { return _visibility; }
    static bool in_visibility(const string& key)                     { return _visibility.find(key) != _visibility.end(); }

    static bool in_param_attrs(const string& key);
    static bool in_tails(const string& key)                           { return _tails.find(key) != _tails.end(); }
    //static bool in_tails1(string key);

    static bool in_terminator_insts(const string& key)                { return _terminator_insts.find(key) != _terminator_insts.end(); }
};

#endif //LLPARSER_INSTFLAG_H
