//
// Created by tzhou on 8/10/17.
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

class HotCallClonePass: public Pass {
    bool PrintCloning;
    bool TracingVerbose;
    std::set<string> _black;
    bool _has_overlapped_path;
    std::ofstream _ofs;
    std::map<Function*, std::set<Instruction*> > _callers;  // partial callers
    //const int FUNC_MAX_LEN = 1024;
public:
    HotCallClonePass() {
        set_is_module_pass();

        _has_overlapped_path = false;

        PrintCloning = false;
        TracingVerbose = false;
    }
            
    ~HotCallClonePass() {
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

        Function* malloc = module->get_function("malloc");
        int cnt = 0;

        do {
            if (TracingVerbose) {
                printf("round: %d", cnt);
            }

            cnt++;
            _black.clear();
            _has_overlapped_path = false;  // always assume this round is the last round
            //auto& users = _callers[malloc];
            auto& users = malloc->user_list();
            auto users_copy = users; // user_list might change during the iteration since new functions may be created
            for (auto uit = users_copy.begin(); uit != users_copy.end(); ++uit) {
                Function* func = (*uit)->function();
                do_clone(func, module);

            }
        } while (_has_overlapped_path && cnt < 10);


        string out = SysArgs::filename();
        if (out.empty()) {
            out = "new";
        }
        out += ".hotCallClone";

        if (SysArgs::has_property("output")) {
            out = SysArgs::get_property("output");
        }

        SysDict::module()->print_to_file(out);
    }

    void do_clone(Function* f, Module *module) {
        if (TracingVerbose) {
            printf("inpsect func: %s\n", f->name_as_c_str());
        }

        // not allow loop in the call graph
        if (_black.find(f->name()) != _black.end()) {
            return;
            guarantee(0, "should not recheck a node: %s", f->name().c_str());
        }
        else {
            _black.insert(f->name());
        }

        auto& users = f->user_list();
        //auto& users = _callers[f];
        auto users_copy = users;
//    if (users_copy.size() > 1) {
//        if (f->name() == "BZ2_bzReadOpen") {
//            zpl("%s is called in ", f->name().c_str());
//            for (auto u: users) {
//                zpl("  %s", u->function()->name().c_str());
//            }
//        }
//    }
        int num = 0;
        for (auto it = users_copy.begin(); it != users_copy.end(); ++it, ++num) {
            CallInst* ci = dynamic_cast<CallInst*>(*it);
            guarantee(ci, " ");
            if (num > 0) {
                _has_overlapped_path = true;  // set the flag to scan again
                Function* fclone = f->clone();
                module->append_new_function(fclone);

                if (PrintCloning) {
                    printf("cloned %s to %s\n", f->name_as_c_str(), fclone->name_as_c_str());
                }
                ci->replace_callee(fclone->name());
            }

            if (_black.find(ci->function()->name()) == _black.end()) {
                do_clone(ci->function(), module);
            }

        }
    }

    //bool do_finalization(Module* module);
};


REGISTER_PASS(HotCallClonePass);
