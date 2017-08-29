//
// Created by tlaber on 6/14/17.
//

#include <asmParser/llParser.h>
#include <utilities/flags.h>
#include <asmParser/sysDict.h>
#include <asmParser/instParser.h>
#include <peripheral/timer.h>
#include <iomanip>
#include "argsParser.h"
#include "sysArgs.h"
#include "passes/passManager.h"
#include "utilities/strings.h"

int SysArgs::_file_id = -1;
std::vector<string> SysArgs::_filenames;
std::map<string, string> SysArgs::_properties;
std::set<string> SysArgs::_flags;
string SysArgs::cur_target;

void SysArgs::init(SoptInitArgs* initArgs) {
    PassManager* pm = PassManager::pass_manager;

    ArgsParser ap;
    ap.parse_args(initArgs);

    for (int i = 0; i < ap.passes().size(); ++i) {
        string pass = ap.passes()[i];
        pm->add_pass(ap.passes()[i]);
    }

    if (DebugRun) {
        std::vector<string> benches;
        benches.push_back("401.bzip2");
        //benches.push_back("astar");
        //benches.push_back("mcf");
        //benches.push_back("gobmk");
        //benches.push_back("h264ref");
        //benches.push_back("libquantum");
        //benches.push_back("403.gcc");
        for (int i = 0; i < benches.size(); ++i) {
            string path = "../../benchmarks/cpu2006/" + benches[i] + "/src/" + benches[i].substr(4);
            //parser->parse("../../test/fortran/a.ll");
            //parser->parse("../../test/clone/clone3.ll");
            //string p = "../../test/fortran/shell_lam.fppized.o.ll";
            //path = "../../test/clone/clone3.ll";
            add_target_file(path);
        }
    }
}

bool SysArgs::get_next_file() {
    if (_file_id < (int)(_filenames.size()-1)) {
        _file_id++;
        return true;
    }
    else {
        return false;
    }
}

string& SysArgs::filename() {
    guarantee(_file_id < (int)_filenames.size() && _file_id > -1, "_file_id: %d, _filename.size(): %d", _file_id, _filenames.size());
    return _filenames[_file_id];
//    if (_filenames.size() > 0) {
//        return _filenames[0];
//    }
//    else {
//        return "";
//    }
}

void SysArgs::add_target_file(string name) {
    _filenames.push_back(name);
}

string SysArgs::get_property(string key) {
    if (_properties.find(key) == _properties.end()) {
        return "";
    } else {
        return _properties[key];
    }
}

bool SysArgs::has_property(string key) {
    if (_properties.find(key) == _properties.end()) {
        return false;
    } else {
        return true;
    }
}

void SysArgs::print_help() {
    std::cout << std::setiosflags(std::ios::left);
    std::cout << "Usage: sopt [OPTION] <file>\n"
              "Parse a .ll file and perform some analysis or transformations on the IR\n"
              "\nOptions:\n"
              << std::setw(30) << "  -g" << "Tell the parser debug info is available, which enables some optimizations;\n"
              << std::setw(30) << "  " << "  Debug info is always parsed if present, even without -g\n"

              << std::setw(30) << "  -load, --load" << "Name of the pass loaded. Use ',' or '+' as delimiter for multiple passes\n"

              << std::setw(30) << "  -path, --ld-pass-path" << "Specify the path to load passes from. Defaults to the path configured in $HOME/.sopt/config\n"
              << std::setw(30) << "  " << "  If $HOME/.sopt/config is not found, the load path defaults to '../passes/'\n"

              << std::setw(30) << "  -o, --output" << "Specify the output file name. Usually passes need to produce a file after transformation\n"

              << "\nA pass may also accept arguments; the arguments should be appended to the pass name using '?' as the delimiter. \n"
              << "The current syntax is --load=<pass>?arg1=xxx?arg2=xxx, or -load <pass>?arg1=xxx?arg2=xxx etc.\n\n"
              << "Examples:\n"
              << "  # produces a call graph in dot format for a.ll. The call Graph only contains paths to 'malloc'. The output is a.dot\n"
              << "  sopt -g --load=callGraph?bottom=malloc a.ll --output=a.dot\n"
              << "  # run a HelloWorld pass that prints a hello when each function is examined\n"
              << "  sopt -g -load hello a.ll\n"
              << "  # run a pass that replaces every occurrence of malloc call with a custom call\n"
              << "  sopt -g -load malloc a.ll -o new.ll\n";
    std::cout << std::endl;
}