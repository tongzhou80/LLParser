//
// Created by tlaber on 6/14/17.
//

#ifndef LLPARSER_SysArgs_H
#define LLPARSER_SysArgs_H

#include <vector>
#include <map>
#include <set>
#include "utilities/macros.h"

struct SoptInitArgs {
    int version;
    int argc;
    char** argv;
};


class SysArgs {
    static int _file_id;
    static std::vector<string> _filenames;
    static std::map<string, string> _properties;
    static std::set<string> _flags;

public:
    static string cur_target;

    static void init(SoptInitArgs* initArgs);
    static void add_target_file(string name);

    static bool get_next_file();
    static string& filename();
    static std::vector<string>& filenames()                { return _filenames; }

    static void set_property(string key, string value)     { _properties[key] = value; }
    static string get_property(string key);
    static bool has_property(string key);

    static void set_flag(string f)                           { _flags.insert(f); }
    static bool get_flag(string f)                           { return _flags.find(f) != _flags.end(); }

    static void print_help();
};

#endif //LLPARSER_SysArgs_H
