//
// Created by tlaber on 6/14/17.
//

#include "optParser.h"

#include "optParser.h"


GetOpt::GetOpt(int argc, char** argv) {
    _argc = argc;
    _argv = argv;
    _option_string = "";
    _doc = "";
    _required_arg_num = 0;
    _parse_verbose = false;

    OptionStruct opt("help", 1, "print help message", no_argument, 0, OptionArgType::CHARS); // 1 is reserved as the value for "help" option
    add_option(opt);
}

void GetOpt::add_doc(const char* doc) {
    _doc += doc;
    _doc += "\n";
}

void GetOpt::add_option(const char* lname, char sname, const char* d, int arg, void* st, OptionArgType t) {
    OptionStruct opt(lname, sname, d, arg, st, t);
    add_option(opt);
}

void GetOpt::add_option(OptionStruct opt_) {
    _option_map[(int)opt_.short_name] = opt_;

    if (opt_.long_name != NULL) {
        _option_string += opt_.short_name;
        if (opt_.has_arg == required_argument) {
            _option_string += ':';
        }
        else if (opt_.has_arg == optional_argument) {
            _option_string += ':';
        }
    }
}

option* GetOpt::option_array() {
    int array_size = _option_map.size() + 1; // last item is (0, 0, 0, 0)
    option* options = new option[array_size];
    int i = 0;
    std::map<int, OptionStruct>::iterator it;
    for (it = _option_map.begin(); it != _option_map.end(); it++) {
        //int short_name = (int)it->first;
        OptionStruct opt_ = it->second;
        option opt;
        opt.name = opt_.long_name;
        opt.has_arg = opt_.has_arg;
        opt.flag = 0;
        opt.val = (int)opt_.short_name;

        options[i++] = opt;
    }

    option opt;
    opt.name = 0;
    opt.has_arg = 0;
    opt.flag = 0;
    opt.val = 0;
    options[i] = opt;

    return options;
}

void GetOpt::print_help() {
    std::string opt_msg = "Options:\n";
    std::map<int, OptionStruct>::iterator it;
    for (it = _option_map.begin(); it != _option_map.end(); it++) {
        OptionStruct opt = it->second;
        char name[128];
        char line[1024];
        if (opt.short_name < 64) {
            sprintf(name, "--%s", opt.long_name);
        }
        else {
            sprintf(name, "--%s, -%c", opt.long_name, opt.short_name);
        }
        sprintf(line, "  %-32s %s\n", name, opt.desc);
        opt_msg += line;
    }

    printf("%s\n%s", _doc.c_str(), opt_msg.c_str());
}

void GetOpt::parse() {
    if (_argc-1 < required_arg_num()) {
        print_help();
        exit(0);
    }


    int c;
    int option_index = 0;

    if (_parse_verbose) {
        printf("option string: %s\n", _option_string.c_str());
    }

    while (1) {
        c = getopt_long (_argc, _argv, _option_string.c_str(), //"t:i:r:b:c:v::h",
                         option_array(), &option_index);

        /* Detect the end of the options. */
        if (c == -1)
            break;

        OptionStruct opt = _option_map[c];
        if (_parse_verbose) {
            printf ("option -%c with value '%s'\n", (char)c, optarg);
        }

        if (opt.long_name == "help") {
            print_help();
            exit(0);
        }

        if (opt.store) {
            switch (opt.type) {
                case OptionArgType::BOOL: {
                    *((bool*)opt.store) = true;
                    break;
                }

                case OptionArgType::CHARS: {
                    strcpy((char*)opt.store, optarg);
                    break;
                }
                case OptionArgType::INT: {
                    *((int*)opt.store) = std::stoi(optarg);
                    break;
                }
                case OptionArgType::FLOAT: {
                    *((float*)opt.store) = std::stof(optarg);
                    break;
                }
            }
        }


    }

}

void GetOpt::test() {
    int id;
    float score;
    char name[100];
    char passes[4096];


    // OptionStruct o1("id", 'i', "id of xxx", required_argument, &id, OptionArgType::INT);
    // add_option(o1);
    verbose_on();
    //require_arg_num(3);
    add_option("id", 'i', "id of xxx", required_argument, &id, OptionArgType::INT);
    add_option("name", 'n', "name of xxx", required_argument, name, OptionArgType::CHARS);
    add_option("score", 's', "score of xxx", required_argument, &score, OptionArgType::FLOAT);

    parse();

    printf("id: %d\n", id);
    printf("name: %s\n", name);
    printf("score: %.3f\n", score);
}
