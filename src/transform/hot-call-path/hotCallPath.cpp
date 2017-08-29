//
// Created by tzhou on 8/14/17.
//

#include <set>
#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <ir/di/diEssential.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>

struct XPS_CallSite {
    string func;
    string file;
    int line;
    int column;

    XPS_CallSite(const string &func, const string &file, int line) : func(func), file(file), line(line) {}

    XPS_CallSite(const string &func, const string &file, int line, int column) : func(func), file(file), line(line),
                                                                                 column(column) {}
};

#define FUNC_MAX_LEN 1024

class HotCallPathPass: public Pass {
    bool PrintCloning;
    bool TracingVerbose;
    std::set<string> _black;
    bool _has_overlapped_path;
    std::ofstream _ofs;
    std::map<Function*, std::set<Instruction*> > _callers;  // partial callers
    //const int FUNC_MAX_LEN = 1024;
public:
    HotCallPathPass() {
        set_is_module_pass();

        _has_overlapped_path = false;

        PrintCloning = false;
        TracingVerbose = false;
    }

    ~HotCallPathPass() {
        printf("pass unloading is not yet implemented! Do stuff in do_finalization!\n");
    }

    void load_hot_aps_file(string filename) {
        std::ifstream ifs;
        ifs.open(filename);
        string line;
        std::vector<XPS_CallSite*> callstack;
        while (std::getline(ifs, line)) {
//            // got a whole call stack
            if (line.empty()) {

            }
            else {
                char callee_name[FUNC_MAX_LEN] = {0};
                char file[FUNC_MAX_LEN] = {0};
                int line_num, column;

                int matched = sscanf(line.c_str(), "%s %[^:]:%d:%d", callee_name, file, &line_num, &column);
                guarantee(matched == 4, "Matched: %d, Bad hotset file format: %s", matched, line.c_str());

                Function *callee = SysDict::module()->get_function(callee_name);
                guarantee(callee, "Callee %s not found", callee_name);
                auto &users = callee->user_list();
                for (auto uit = users.begin(); uit != users.end(); ++uit) {
                    Instruction* I = *uit;
                    DILocation *loc = I->debug_loc();
                    guarantee(loc, "This pass needs full debug info, please compile with -g");
                    if (loc->filename() == string(file) &&
                        loc->line() == line_num &&
                        loc->column() == column) {
                        //I->dump();
                        add_partial_caller(callee, I);
                    }
                }
                //XPS_CallSite* cs = new XPS_CallSite(func, file, line_num);
                //callstack.push_back(cs);
            }
        }
    }

    void add_partial_caller(Function* callee, Instruction* user) {
        if (_callers.find(callee) == _callers.end()) {
            std::set<Instruction*> callers;
            _callers[callee] = callers;
        }
        _callers[callee].insert(user);
    }

    void prune_call_graph() {
        auto l = SysDict::module()->function_list();
        for (auto fi = l.begin(); fi != l.end(); ++fi) {
            Function* F = *fi;
            if (_callers.find(F) != _callers.end()) {
                F->user_list() = Function::InstList(_callers[F].begin(), _callers[F].end());
            }
            else {
                F->user_list().clear();
            }
        }
    }

    bool run_on_module(Module* module) {
        string arg_name = "hot_aps_file";
        if (has_argument(arg_name)) {
            string hot_aps_file = get_argument(arg_name);
            load_hot_aps_file(hot_aps_file);
        }
        prune_call_graph();
    }

    //bool do_finalization(Module* module);
};


REGISTER_PASS(HotCallPathPass);
