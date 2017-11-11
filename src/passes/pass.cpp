//
// Created by GentlyGuitar on 6/8/2017.
//

#include <utilities/macros.h>
#include <utilities/strings.h>
#include "pass.h"
#include <utilities/flags.h>

#define ARG_DEFAULT_VALUE "1"

Pass::Pass() {
    _is_dynamic = false;
    _priority = 0;
    _is_global_pass = false;
    _is_module_pass = false;
    _is_function_pass = false;
    _is_basic_block_pass = false;
    _is_instruction_pass = false;

    _is_parse_time = false;
    _unloader = NULL;
}

void Pass::unload() {
    guarantee(_unloader, " ");
    _unloader(this);
}

void Pass::parse_arguments(string args) {
    if (PrintPassArguments) {
        printf("---- Print Arguments ----\n");
    }
    for (auto option: Strings::split(args, ':')) {
        auto pair = Strings::split(option, '=');
        if (pair.size() == 1) {
            _args[pair[0]] = ARG_DEFAULT_VALUE;
        }
        else if (pair.size() == 2) {
            _args[pair[0]] = pair[1];
        }
        else {
            guarantee(0, " ");
        }
        if (PrintPassArguments) {
            printf("arg: %s, value: %s\n", pair[0].c_str(), _args[pair[0]].c_str());
        }
    }
    if (PrintPassArguments) {
        printf("---- End Arguments ----\n");
    }
}

bool Pass::has_argument(string key) {
    if (_args.find(key) != _args.end()) {
        return true;
    }
    else {
        return false;
    }
}

string Pass::get_argument(string key) {
    if (!has_argument(key)) {
        return ARG_DEFAULT_VALUE;  // default value for options that have no value
    }
    else {
        return _args[key];
    }
}

void Pass::print_arguments() {
    for (auto it: _args) {
        std::cout << "---- Print Arguments ----\n";
        std::cout << "options: " << it.first << ", value: " << it.second << '\n';
        std::cout << "---- End Arguments ----\n";
    }
}