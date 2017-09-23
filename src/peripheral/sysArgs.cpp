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
std::map<string, string> SysArgs::_options;
std::set<string> SysArgs::_flags;
std::vector<string> SysArgs::_passes;
string SysArgs::cur_target;

void SysArgs::init(SoptInitArgs* initArgs) {
    PassManager* pm = PassManager::pass_manager;

    ArgsParser ap;
    ap.parse_args(initArgs);

    if (DebugRun) {
        std::vector<string> benches;
        benches.push_back("450.soplex");
        //benches.push_back("433.milc");
        //benches.push_back("453.povray");
        benches.push_back("401.bzip2");
        //benches.push_back("456.hmmer");
        //benches.push_back("astar");
        //benches.push_back("429.mcf");
        //benches.push_back("gobmk");
        //benches.push_back("h264ref");
        //benches.push_back("462.libquantum");
        //benches.push_back("403.gcc");
        for (int i = 0; i < benches.size(); ++i) {
            string path = "../../benchmarks/cpu2006/" + benches[i] + "/src/" + benches[i].substr(4) +".ll";
            //parser->parse("../../test/fortran/a.ll");
            //parser->parse("../../test/clone/clone3.ll");
            //path = "../../test/fortran/spec_backtrace.o.ll";
            //path = "../../test/clone/clone3.ll";
            add_target_file(path);
        }
    }
}

void SysArgs::add_target_file(string name) {
    _filenames.push_back(name);
}

void SysArgs::use_split_files() {
    string dir = SysDict::filedir() + "split-module/";
}

string SysArgs::get_option(string key) {
    if (_options.find(key) == _options.end()) {
        return "";
    } else {
        return _options[key];
    }
}

bool SysArgs::has_option(string key) {
    if (_options.find(key) == _options.end()) {
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
//              << std::setw(30) << "  -g" << "tell the parser debug info is available, which enables some optimizations;\n"
//              << std::setw(30) << "  " << "debug info is always parsed if present, even without -g\n"

              << std::setw(30) << "  -load, --load=<PASSNAME>" << "load a set of passes specified by the argument. Use ',' or '+' as delimiter for multiple passes\n"

              << std::setw(30) << "  -path, --ld-pass-path" << "specify the path to load passes from. Defaults to the path configured in $HOME/.sopt/config\n"
              << std::setw(30) << "  " << "if $HOME/.sopt/config is not found, the load path defaults to '../passes/'\n"

              << std::setw(30) << "  -o, --output" << "specify the output file name. Usually passes need to produce a file after transformation\n"

              << "\nA pass may also accept arguments; the arguments should be appended to the pass name using ':' as the delimiter. \n"
              << "The current syntax is --load=<pass>:arg1=xxx:arg2=xxx, or -load <pass>:arg1=xxx:arg2=xxx etc.\n\n"
              << "Examples:\n"
              << "  # produces a call graph in dot format for a.ll. The call Graph only contains paths to 'malloc'. The output is a.dot\n"
              << "  sopt --load=callGraph?bottom=malloc a.ll --output=a.dot\n"
              << "  # run a HelloWorld pass that prints a hello when each function is examined\n"
              << "  sopt -load hello input.ll  # or sopt -load /xxx/xxx/pass_dir/libHello.so input.ll\n"
              << "  \n"
              << "  # run pass1, pass2, pass3 respectively on input.ll\n"
              << "  sopt -load pass1,pass2,pass3 input.ll\n"
              << "  # run pass1, pass2, pass3 respectively on input1.ll, input2.ll\n"
              << "  sopt -load pass1,pass2,pass3 input1.ll input2.ll  # When -XX:+ParallelModule is on, the input files are parsed/analyzed/transformed in parallel \n"

            ;
//              << "  # run a pass that replaces every occurrence of malloc call with a custom call\n"
//              << "  sopt -g -load malloc a.ll -o new.ll\n";
    std::cout << std::endl;
}