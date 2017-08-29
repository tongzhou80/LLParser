//
// Created by tlaber on 6/14/17.
//

#ifndef LLPARSER_OPTPARSER_H
#define LLPARSER_OPTPARSER_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <cassert>
#include <vector>
#include <string>
#include <map>

// struct option {
//     const char *name;
//     int         has_arg;
//     int        *flag;
//     int         val;
// };

enum class OptionArgType {
    BOOL,
    CHARS,
    INT,
    FLOAT
};


struct OptionStruct {
    const char* long_name;
    char short_name;
    const char* desc;
    int has_arg;
    void* store;
    OptionArgType type;
    OptionStruct() {}
    OptionStruct(const char* lname, char sname, const char* d, int arg, void* st, OptionArgType t):
            long_name(lname), short_name(sname), desc(d), has_arg(arg), store(st), type(t) {}
};


/* a getopt.h based argument parser */
class GetOpt {
private:
    int _argc;
    char** _argv;
    int _required_arg_num;
    std::string _doc;
    std::map<int, OptionStruct> _option_map;
    std::string _option_string;

    bool _parse_verbose;
public:

    GetOpt(int agrc, char** argv);
    void add_option(OptionStruct opt);
    void add_option(const char* lname, char sname, const char* d, int arg, void* st, OptionArgType t);
    void add_doc(const char*);
    void verbose_on() { _parse_verbose = true; }
    void verbose_off() { _parse_verbose = false; }
    option* option_array();
    void parse();
    void print_help();
    void require_arg_num(int num) { _required_arg_num = num; } // the program requires at least num arguments
    int required_arg_num() { return _required_arg_num; }

    void test();
};

#endif //LLPARSER_OPTPARSER_H
