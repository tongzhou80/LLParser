//
// Created by tlaber on 6/14/17.
//

#ifndef LLPARSER_ARGSPARSER_H
#define LLPARSER_ARGSPARSER_H

#include "stringParser.h"

struct SoptInitArgs;

class ArgsParser: public StringParser {
    SoptInitArgs* _args;
    std::vector<string> _passes;
    bool _eoa;
    int _p;
public:
    ArgsParser();
    void parse_args(SoptInitArgs* initArgs);
    void get_arg();
    void parse_long_option();
    void parse_short_flag();
    void parse_config_files();
    void set_bool_flag(string flag, int v);

    std::vector<string> passes()                    { return _passes; }

};

#endif //LLPARSER_ARGSPARSER_H
