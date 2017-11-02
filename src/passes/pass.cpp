//
// Created by GentlyGuitar on 6/8/2017.
//

#include <utilities/macros.h>
#include <utilities/strings.h>
#include "pass.h"

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
    auto pairs = Strings::split(args, ':');
    for (int i = 0; i < pairs.size(); ++i) {
        auto pair = Strings::split(pairs[i], '=');
        if (pair.size() == 1) {
            _args[pair[0]] = ARG_DEFAULT_VALUE;
        }
        else if (pair.size() == 2) {
            zps(pair[0])
            _args[pair[0]] = pair[1];
        }
        else {
            guarantee(0, " ");
        }
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
