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
    bool MatchVerbose;
    std::set<string> _black;
    bool _has_overlapped_path;
    std::ofstream _ofs;
    std::map<Function*, std::set<Instruction*> > _callers;  // partial callers

    std::vector<Instruction*> _stack;
    string _caller;
    string _callee;
    int _path_counter;
    int _cxt_counter;
    bool _skip;
    //const int FUNC_MAX_LEN = 1024;
public:
    HotCallClonePass() {
        set_is_module_pass();

        _has_overlapped_path = false;

        PrintCloning = false;
        TracingVerbose = false;
        _path_counter = 1;
        _cxt_counter = 0;
        _skip = false;
        MatchVerbose = 1;
    }
            
    ~HotCallClonePass() {
        printf("pass unloading is not yet implemented! Do stuff in do_finalization!\n");
    }


    // todo: the hot_aps_file must have an appending new line to be correctly parsed for now
    void load_hot_aps_file(string filename) {
        std::ifstream ifs;
        ifs.open(filename);
        if (!ifs.is_open()) {
            fprintf(stderr, "open file %s failed.\n", filename.c_str());
        }
        string line;
        std::vector<XPS_CallSite*> callstack;
        bool is_header = true;
        int recognized = 0;

        while (std::getline(ifs, line)) {
            // got a whole call stack
            if (line.empty()) {
                is_header = true;

                bool has_all = true;
                if (!_stack.empty()) {
                    for (int i = 0; i < _stack.size(); ++i) {
                        if (!_stack[i]) {
                            has_all = false;
                        }
                    }
                    zpl("has all: %d\n", has_all)
                    if (has_all) {
                        recognized++;
                        //clone_call_path(_stack);
                        _path_counter++;
                    }
                    _cxt_counter++;
                    _stack.clear();
                    _caller.clear();
                    _callee.clear();
                    _skip = false;
                }
            }
            else {
                if (is_header) {
                    match_header(line);
                    zpl("header: %s", line.c_str())
                    is_header = false;
                }
                else {
                    if (Strings::conatins(line, "__libc_start_main")) {
                        _skip = true;
                    }

                    if (!_skip) {
                        Instruction* ret = match_callsite(line);
                        _stack.push_back(ret);
                    }
                }
            }
        }
        zpl("recog: %d, path: %d, cxt: %d", recognized, _path_counter, _cxt_counter);
    }

    int match_header(string& line) {
        char hotness[11];  // max hold 0xffffffff + '\0'
        int apid;

        int matched = sscanf(line.c_str(), "%d %s", &apid, hotness);
        guarantee(matched == 2, "Matched: %d, Bad hotset file format: %s", matched, line.c_str());
    }

    Instruction* match_callsite(string & line) {
        guarantee(Strings::conatins(line, "("), " ");
        line = line.substr(line.find('('));  // strip the exe name before '('

        int pos1 = line.find(' ');
        string bt_symbol = line.substr(0, pos1);
        string fileline = line.substr(pos1+1);

        if (bt_symbol != "()") {
            int pos2 = bt_symbol.find('+');
            _caller = bt_symbol.substr(1, pos2-1);
            string offset = bt_symbol.substr(pos2+1);
        }
        else {
            _caller = "";
        }

        /* remove (discriminator x) */
        int pos4 = fileline.find(' ');
        if (pos4 != fileline.npos) {
            fileline = fileline.substr(0, pos4);
        }

        int pos3 = fileline.find(':');
        string file = fileline.substr(0, pos3);
        string lino_str = fileline.substr(pos3+1);
        int line_num = -1;
        if (Strings::is_number(lino_str)) {
            line_num = std::stoi(lino_str);
        }

        Instruction* ret = NULL;

        if (_callee.empty()) {
            ret = approximately_match_alloc(file, line_num);
            if (!ret) {
                _skip = true;
            }
        }
        else {
            ret = approximately_match(file, line_num);
        }
        _callee = _caller;
        zpl("new callee: %s", _callee.c_str())
        return ret;
    }

    Instruction* approximately_match_alloc(string filename, int line) {
        CallInstFamily* final = NULL;

        // level 1
        std::map<CallInstFamily*, int> users_offsets;
        std::vector<CallInstFamily*> other_callers;
        if (!final) {
            std::vector<string> candidates = {"malloc", "calloc", "realloc"};
            for (auto c: candidates) {
                Function* alloc = SysDict::module()->get_function(c);
                if (alloc) {
                    for (auto I: alloc->user_list()) {
                        if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(I)) {
                            DILocation *loc = ci->debug_loc();
                            guarantee(loc, "This pass needs full debug info, please compile with -g");
                            if (Strings::conatins(filename, loc->filename())) {
                                users_offsets[ci] = std::abs(line-loc->line());
                            }
                            else {
                                other_callers.push_back(ci);
                            }
                        }
                    }
                }
            }

            if (users_offsets.size() == 0) {
                final = NULL;
            }
            else if (users_offsets.size() == 1) {
                final = users_offsets.begin()->first;
            }
            else {
                int closest = users_offsets.begin()->second;
                CallInstFamily* closest_I = users_offsets.begin()->first;
                if (!final) {
                    for (auto it = users_offsets.begin(); it != users_offsets.end(); ++it) {
                        if (it->second < closest) {
                            closest = it->second;
                            closest_I = it->first;
                        }
                    }
                    final = closest_I;
                }
            }
        }

        if (final) {
            zpl("infer alloc: %s", final->called_function()->name_as_c_str())
        }

        return final;
    }

    // todo: use CallInstFamily
    Instruction* approximately_match(string filename, int line) {

        Function* calleef = SysDict::module()->get_function(_callee);
        Instruction* final = NULL;
        guarantee(calleef, " ");
        auto users = calleef->user_list();

        // level 0
        if (users.size() == 1) {
            final = users[0];
        }

        // level 1
        std::map<Instruction*, int> users_offsets;
        std::vector<Instruction*> other_callers;
        if (!final) {
            for (auto I: calleef->user_list()) {
                if (CallInst* ci = dynamic_cast<CallInst*>(I)) {
                    DILocation *loc = ci->debug_loc();
                    guarantee(loc, "This pass needs full debug info, please compile with -g");
                    if (Strings::conatins(filename, loc->filename())) {
                        users_offsets[I] = std::abs(line-loc->line());
                    }
                    else {
                        other_callers.push_back(I);
                    }
                }
            }

            if (users_offsets.size() == 0) {
                final = NULL;
            }
            else if (users_offsets.size() == 1) {
                final = users_offsets.begin()->first;
            }
            else {
                int closest = users_offsets.begin()->second;
                Instruction* closest_I = users_offsets.begin()->first;
                if (!final) {
                    for (auto it = users_offsets.begin(); it != users_offsets.end(); ++it) {
                        if (it->second < closest) {
                            closest = it->second;
                            closest_I = it->first;
                        }
                    }
                    final = closest_I;
                }
            }
        }

        if (!final && !other_callers.empty()) {
            final = other_callers[0];
        }

        if (_caller.empty() && final) {
            _caller = final->owner();
            zpl("infer caller: %s", _caller.c_str())
        }

        if (MatchVerbose) {
            if (final) {
                DILocation *loc = final->debug_loc();
                guarantee(loc, "This pass needs full debug info, please compile with -g");
                printf("(%s, %s, %s, %d) => (%s, %s, %s, %d)\n", _caller.c_str(), _callee.c_str(), filename.c_str(), line,
                       loc->function().c_str(), _callee.c_str(), loc->filename().c_str(), loc->line());

            } else {
                printf("(%s, %s, %s, %d) => None\n", _caller.c_str(), _callee.c_str(), filename.c_str(), line);
            }
        }
        return final;
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
//        prune_call_graph();
//
//        Function* malloc = module->get_function("malloc");
//        int cnt = 0;
//
//        do {
//            if (TracingVerbose) {
//                printf("round: %d", cnt);
//            }
//
//            cnt++;
//            _black.clear();
//            _has_overlapped_path = false;  // always assume this round is the last round
//            //auto& users = _callers[malloc];
//            auto& users = malloc->user_list();
//            auto users_copy = users; // user_list might change during the iteration since new functions may be created
//            for (auto uit = users_copy.begin(); uit != users_copy.end(); ++uit) {
//                Function* func = (*uit)->function();
//                do_clone(func, module);
//
//            }
//        } while (_has_overlapped_path && cnt < 10);
//
//
//        string out = SysArgs::filename();
//        if (out.empty()) {
//            out = "new";
//        }
//        out += ".hotCallClone";
//
//        if (SysArgs::has_property("output")) {
//            out = SysArgs::get_property("output");
//        }
//
//        SysDict::module()->print_to_file(out);
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
