//
// Created by tlaber on 6/14/17.
//

#include <fstream>
#include <utilities/strings.h>
#include <cstring>
#include <utilities/flags.h>
#include "argsParser.h"
#include "sysArgs.h"

ArgsParser::ArgsParser() {
    _p = 1;  // skip the first argument
    _eoa = false;
}

void ArgsParser::parse_config_files() {
    SysArgs::set_property("ld-pass-path", "../passes/");

    const char* home = std::getenv("HOME");
    guarantee(home != NULL, "$HOME is not set");

    string pass_ld_config = home;
    pass_ld_config += "/.sopt/config";
    std::ifstream ifs;
    ifs.open(pass_ld_config);
    if (ifs.good()) {
        string line;
        while (std::getline(ifs, line)) {
            if (line[0] == ';') {
                continue;
            }
            set_text(line);
            if (Strings::startswith(_text, "ld-pass-path")) {
                inc_intext_pos(strlen("ld-pass-path"));
                get_word('=');
                get_word(','); // todo: ',' is just a randomly chosen delimiter
                SysArgs::set_property("ld-pass-path", _word);
            }
        }
    }
}

void ArgsParser::get_arg() {
    if (_p >= _args->argc) {
        _eoa = true;
        if (ArgParsingVerbose) {
            printf("arg ends\n");
        }
        return;
    }
    else {
        string cur = _args->argv[_p];
        set_text(cur);
        ++_p;
    }
}

void ArgsParser::parse_args(SoptInitArgs* init_args) {
    _args = init_args;

    parse_config_files();

    if (init_args->argc == 1) {
        return;
    }

    string args;
    for (int i = 1; i < init_args->argc; ++i) {
        args += string(init_args->argv[i]) + ' ';
    }
    set_text(args);

    while (!_eol) {
        get_word_until(" =");
        if (Strings::startswith(_word, "-XX:")) {
            int xx_len = strlen("-XX:");
            if (_word[xx_len] == '+') {
                Flags::set_flag(_word.substr(xx_len+1), true);
            }
            else if (_word[xx_len] == '-') {
                Flags::set_flag(_text.substr(xx_len+1), false);
            }
            else {
                guarantee(0, "Bad formatted option: %s (miss a '+' or '-'?)", _text.c_str());
            }
        }
        else if (Strings::startswith(_word, "-")) {
            parse_long_option();
        }
        else {
            SysArgs::add_target_file(_word);
        }
    }
}

void ArgsParser::parse_short_flag() {
    if (_word == "-g") {
        SysArgs::set_flag("debug-info");
    }
    else if (_word == "-load") {
        get_arg();
        guarantee(!_eoa, " ");

        auto passes = Strings::split(_text, ',');
        if (passes.size() == 0) {
            passes = Strings::split(_text, '+');
        }
        guarantee(passes.size() != 0, ".");
        for (auto p: passes) {
            SysArgs::passes().push_back(p);
        }
    }
    else if (_word == "-path") {
        get_arg();
        guarantee(!_eoa, " ");
        SysArgs::set_property("ld-pass-path", _text);
    }
    else if (_word == "-o") {
        get_arg();
        guarantee(!_eoa, " ");
        SysArgs::set_property("output", _text);
    }
    else {
        std::cout << "ignored short option: " << _word << std::endl;
    }
}

void ArgsParser::parse_long_option() {
    string opt = _word;

    if (opt == "--help") {
        SysArgs::print_help();
        exit(0);
    }
    else if (opt == "--save-dbg") {
        SysArgs::set_property("save-dbg", "");
    }
    else if (opt == "--load-dbg") {
        SysArgs::set_property("load-dbg", "");
    }
    else if (opt == "--load" || opt == "-load") {
        if (_eoa) {
            fprintf(stderr, "--load requires an argument!\n");
            exit(0);
        }

        get_word();

        auto passes = Strings::split(_word, ',');
        if (passes.size() == 0) {
            passes = Strings::split(_word, '+');
        }
        guarantee(passes.size() != 0, ".");
        for (auto p: passes) {
            SysArgs::passes().push_back(p);
        }
    } else if (opt == "--ld-pass-path") {
        if (_eoa) {
            fprintf(stderr, "--ld-pass-path requires an argument!\n");
            exit(0);
        }
        get_word();
        SysArgs::set_property("ld-pass-path", _word);
    }
    else if (opt == "--output" || opt == "-o") {
        if (_eoa) {
            fprintf(stderr, "--output requires an argument!\n");
            exit(0);
        }
        get_word();
        SysArgs::set_property("output", _word);
    }
    else {
        std::cout << "ignored long option: " << opt << std::endl;
    }
}


//void ArgsParser::parse_long_option() {
//    get_word('=');
//    string opt = _word;
//    string value;
//    if (!_eol) {
//        get_word();
//        value = _word;
//    }
//    else {
//        get_arg();
//        value = _text;
//    }
//
//    if (opt == "--help") {
//        SysArgs::print_help();
//        exit(0);
//    }
//    else if (opt == "--load") {
//        if (_eoa) {
//            fprintf(stderr, "--load requires an argument!\n");
//            exit(0);
//        }
//
//        auto passes = Strings::split(value, ',');
//        if (passes.size() == 0) {
//            passes = Strings::split(value, '+');
//        }
//        guarantee(passes.size() != 0, ".");
//        for (auto p: passes) {
//            SysArgs::passes().push_back(p);
//        }
//    } else if (opt == "--ld-pass-path") {
//        if (_eoa) {
//            fprintf(stderr, "--ld-pass-path requires an argument!\n");
//            exit(0);
//        }
//        SysArgs::set_property("ld-pass-path", value);
//    }
//    else if (opt == "--output") {
//        if (_eoa) {
//            fprintf(stderr, "--output requires an argument!\n");
//            exit(0);
//        }
//        SysArgs::set_property("output", value);
//    }
//    else {
//        std::cout << "ignored long option: " << opt << std::endl;
//    }
//}
