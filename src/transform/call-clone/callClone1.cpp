//
// Created by tzhou on 8/10/17.
//

#include <set>
#include <algorithm>
#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <di/diEssential.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>

#include "contextGenerator.h"

#define FUNC_MAX_LEN 1024


struct XPS_Caller {
    CallInstFamily* caller;
    XPath* xps_path;
};

class CallClonePass: public Pass {
    bool PrintCloning;
    bool PathCheck;
    bool MatchVerbose;
    bool CloneVerbose;
    std::set<string> _black;
    bool _has_overlapped_path;
    int _cloned;
    std::ofstream _ofs;
    //std::map<Function*, std::set<CallInstFamily*> > _callers;  // partial callers
    std::vector<XPath*> _all_paths;
    //std::vector<std::vector<CallInstFamily*>> _cold_paths;
    //std::vector<std::vector<CallInstFamily*>> _hot_paths;
    std::map<CallInstFamily*, std::vector<XPS_Caller*>> _callers_map;
    std::map<CallInstFamily*, int> _dr_caller_freq;
    CallInstFamily* _search_root;

    std::vector<CallInstFamily*> _stack;
    string _caller;
    string _callee;
    int _min_cxt;
    bool _done;
    bool _skip;

    /* statistics */
    int _hot_counter;
    int _path_counter;
    int _cxt_counter;
    int _recursive;
    int _recognized;
    int _ben_num;
    int _hot_cxt_counter;
    int _used_hot_cxt;
    Timer _timer;

    //const int FUNC_MAX_LEN = 1024;
public:
    CallClonePass() {
        _timer.start();

        set_is_module_pass();

        CloneVerbose = false;
        MatchVerbose = false;
        PathCheck = false;
        _min_cxt = 1;
        _path_counter = 1;
        _cxt_counter = 0;
        _skip = false;
        _recursive = 0;
        _cloned = 0;

        _ben_num = 0;
        _recognized = 0;
        _hot_counter = 0;
        _hot_cxt_counter = 0;
        _used_hot_cxt = 0;
    }

    ~CallClonePass() {

    }

    CallInstFamily* get_element_in_call_path(std::vector<CallInstFamily*>& path, int pos) {
        if (pos < path.size()) {
            return path[pos];
        }
        else {
            return NULL;
        }
    }

    bool check_common_edge(std::vector<XPath*>& paths, int pos) {
        guarantee(pos < paths[0]->path.size(), "pos: %d, size: %lu", pos, paths.size());
        auto sentinel = paths[0]->path[pos];
        for (int i = 1; i < paths.size(); ++i) {
            if (paths[i]->path[pos] != sentinel) {
                return false;
            }
        }
        return true;
    }

    void scan() {
        bool replaced = false;
        std::map<CallInstFamily*, std::vector<XPath*>> dr_map;
        for (auto xpath: _all_paths) {
            auto dr = xpath->path[0];
            if (dr_map.find(dr) == dr_map.end()) {
                dr_map[dr] = std::vector<XPath*>();
            }
            dr_map[dr].push_back(xpath);
        }

        for (auto dr: dr_map) {
            auto& paths = dr.second;
            if (paths.size() > 1) {
                replaced = true;
                int i = 0;
                for (; check_common_edge(paths, i) ; ++i) {}
                if (i == 0) {
                    for (auto p: paths) {
                        check_path(p->path, true);
                    }
                }
                guarantee(i > 0, "");
                /* i is the position that divergence starts */
                auto root = paths[0]->path[i-1];  // the last shared ins
                auto callee = root->function();
                for (int j = 1; j < paths.size(); ++j) {
                    auto& this_path = paths[j]->path;
                    auto cloned_callee = callee->clone();
                    SysDict::module()->append_new_function(cloned_callee);
                    _cloned++;
                    update_callee_in_all_paths(this_path[i], cloned_callee);
                }
                break;
            }
        }

        if (!replaced) {
            _done = true;
        }
    }

    void update_callee_in_all_paths(CallInstFamily* caller, Function* new_callee) {
        bool is_replaced = false;
        for (auto& xps_path: _all_paths) {

            auto& stack = xps_path->path;
            guarantee(caller != NULL, " ");
            for (int i = 1; i < stack.size(); ++i) {
                CallInstFamily* I = stack[i];
                if (I == caller) {
                    if (!is_replaced) {
                        if (CloneVerbose) {
                            zpl("in callinst repalce %s to %s in %p", I->called_function()->name_as_c_str(), new_callee->name_as_c_str(), I)
                        }

                        I->replace_callee(new_callee->name());
                        is_replaced = true;
                    }
                    auto callee_call = stack[i-1];
                    int i_index = callee_call->get_index_in_block();
                    int b_index = callee_call->parent()->get_index_in_function();
                    //zpl("in %d path replace %p to %p", caller->xps_path->hotness, stack[i-1], new_callee->get_instruction(b_index, i_index));
                    auto ci_in_new_callee = static_cast<CallInstFamily*>(new_callee->get_instruction(b_index, i_index));
                    stack[i-1] = ci_in_new_callee;
                    if (CloneVerbose) {
                        zpl("path pos %d to %s", i - 1, ci_in_new_callee->function()->name_as_c_str())
                    }
                    //check_path(stack);
                }
            }

        }
    }

    // todo: the hot_aps_file must have an appending new line to be correctly parsed for now
    void load_hot_aps_file(string filename) {
        std::ifstream ifs;
        ifs.open(filename);
        if (!ifs.is_open()) {
            fprintf(stderr, "open file %s failed.\n", filename.c_str());
        }
        string line;
        bool is_header = true;
        string header;
        int is_hot = 0;

        while (std::getline(ifs, line)) {
            // got a whole call stack
            if (line.empty()) {
                is_header = true;

                bool has_all = true;
                if (!_stack.empty()) {
                    std::vector<CallInstFamily*> usable_stack;
                    for (auto I:_stack) {
                        if (I) {
                            usable_stack.push_back(I);
                        }
                        else {
                            break;
                        }
                    }
                    _stack = usable_stack;
                    if (_stack.size() < _min_cxt) {
                        has_all = false;
                    }
                    for (int i = 0; i < _stack.size(); ++i) {
                        if (!_stack[i]) {
                            has_all = false;
                        }
                    }

                    if (has_all) {
                        _recognized++;
                        //if (!has_direct_recursion()) {
                        if (!has_continuous_recursion()) {
                            XPath* path = new XPath();
                            path->path = _stack;
                            path->hotness = is_hot;
                            _all_paths.push_back(path);
                        }
                        _path_counter++;
                    }
                    else {
                        if (MatchVerbose)
                        zpl("not match: %s", header.c_str())
                    }
                }
                _cxt_counter++;
                _stack.clear();
                _caller.clear();
                _callee.clear();
                _skip = false;
                is_hot = false;
            }
            else {
                if (is_header) {
                    header = line;
                    is_hot = match_header(line);
                    is_header = false;
                }
                else {
                    if (Strings::contains(line, "__libc_start_main")) {
                        _skip = true;
                    }

                    if (!_skip) {
                        CallInstFamily* ret = match_callsite(line);
                        _stack.push_back(ret);
                    }
                }
            }
        }

    }
//
//    void check_all_paths() {
//        for (auto v: _hot_paths) {
//            string callee = v[0]->called_function()->name();
//            printf("%s", callee.c_str());
//            for (auto I: v) {
//                guarantee(I->called_function()->name() == callee, "%s, %s", I->called_function()->name_as_c_str(), callee.c_str());
//                printf(" <- %s(%p)", I->function()->name_as_c_str(), I);
//                callee = I->function()->name();
//                // printf("%s > %s, ", I->called_function()->name_as_c_str(), I->function()->name_as_c_str());
//            }
//            printf("\n");
//        }
//    }

    void get_distinct_all_paths() {
        std::vector<XPath*> distinct_set;
        std::set<string> contexts;

        // for (auto v: _all_paths) {
        //     if (1) {
        //         string context;
        //         for (auto I: v->path) {
        //             char buf[128];
        //             sprintf(buf, "%p", I);
        //             context += string(buf) + ' ';
        //         }

        //         if (contexts.find(context) != contexts.end()) {
        //             zpd(v->hotness);
        //         }
        //         else {
        //             contexts.insert(context);
        //             distinct_set.push_back(v);
        //         }

        //     }
        // }

        for (auto v: _all_paths) {
            if (v->hotness == 1) {
                string context;
                for (auto I: v->path) {
                    char buf[128];
                    sprintf(buf, "%p", I);
                    context += string(buf) + ' ';
                }

                if (contexts.find(context) != contexts.end()) {

                }
                else {
                    contexts.insert(context);
                    distinct_set.push_back(v);
                }

            }
        }

        for (auto v: _all_paths) {
            if (v->hotness == 0) {
                string context;
                for (auto I: v->path) {
                    char buf[128];
                    sprintf(buf, "%p", I);
                    context += string(buf) + ' ';
                }

                if (contexts.find(context) != contexts.end()) {

                }
                else {
                    contexts.insert(context);
                    distinct_set.push_back(v);
                }

            }
        }


        _all_paths = distinct_set;
    }

    int match_header(string& line) {
        char hotness[11];  // max hold 0xffffffff + '\0'
        int apid;
        char alloc[64];

        int matched = sscanf(line.c_str(), "%d %s %s", &apid, hotness, alloc);
        guarantee(matched == 3, "Matched: %d, Bad hotset file format: %s", matched, line.c_str());

        _callee = string(alloc);
        if (string(hotness) == "0xffffffff") {
            _hot_cxt_counter++;
            return 1;
        }
        else {
            return 0;
        }
    }

    CallInstFamily* match_callsite(string & line) {
        guarantee(Strings::contains(line, "("), " ");
        line = line.substr(line.find('('));  // strip the exe name before '('

        int pos1 = line.find(' ');
        string bt_symbol = line.substr(0, pos1);
        string fileline = line.substr(pos1+1);


        int pos2 = bt_symbol.find('+');
        _caller = bt_symbol.substr(1, pos2-1);
        if (Alias* alias = SysDict::module()->get_alias(_caller)) {
            _caller = dynamic_cast<Function*>(alias->aliasee())->name();
        }
        string offset = bt_symbol.substr(pos2+1);
        int bi, ii;
        sscanf(offset.c_str(), "%d_%d", &bi, &ii);


        int pos3 = fileline.find(':');
        string file = fileline.substr(0, pos3);
        string lino_str = fileline.substr(pos3+1);
        int line_num = -1;
        if (Strings::is_number(lino_str)) {
            line_num = std::stoi(lino_str);
        }

        CallInstFamily* ret = dynamic_cast<CallInstFamily*>(SysDict::module()->get_function(_caller)->get_instruction(bi, ii));
        DILocation* loc = ret->debug_loc();
        guarantee(ret && loc->filename() == file && loc->line() == line_num, "");
        _callee = _caller;

        return ret;
    }

    bool has_direct_recursion() {
        std::map<CallInstFamily*, int> counters;
        for (auto I: _stack) {
            counters[I]++;
        }
        for (auto i: counters) {
            if (i.second > 1) {
                _recursive++;

                auto& v = _stack;
                string callee = v[0]->called_function()->name();
                printf("skip: %s", callee.c_str());
                for (auto I: v) {
                    guarantee(I->called_function()->name() == callee, "%s, %s", I->called_function()->name_as_c_str(), callee.c_str());
                    printf(" <- %s(%p)", I->function()->name_as_c_str(), I);
                    callee = I->function()->name();
                    // printf("%s > %s, ", I->called_function()->name_as_c_str(), I->function()->name_as_c_str());
                }
                return true;
            }
        }

        return false;
    }

    bool has_continuous_recursion() {
        std::map<CallInstFamily*, int> counters;
        for (int i = 1; i < _stack.size(); i++) {
            if (_stack[i] == _stack[i-1]) {
                return true;
            }
        }

        return false;
    }

    bool run_on_module(Module* module) override {
        int nlevel = 2;
        if (has_argument("nlevel")) {
            nlevel = std::stoi(get_argument("nlevel"));
        }

        ContextGenerator cg;

        cg.generate(module, "malloc", nlevel); // todo other allocs
        cg.generate(module, "calloc", nlevel); // todo other allocs
        cg.generate(module, "realloc", nlevel); // todo other allocs

        if (has_argument("min-cxt")) {
            _min_cxt = std::stoi(get_argument("min-cxt"));
            zpd(_min_cxt)
        }
        insert_declaration("malloc", "ben_malloc", true);
        insert_declaration("calloc", "ben_calloc", true);
        insert_declaration("realloc", "ben_realloc", true);
        insert_declaration("free", "ben_free", false);

        string arg_name = "hot_aps_file";
        if (has_argument(arg_name)) {
            string hot_aps_file = get_argument(arg_name);
            load_hot_aps_file(hot_aps_file);
        }
        else {
            load_hot_aps_file(SysDict::filedir() + "contexts.txt");
        }
//
        get_distinct_all_paths();
        check_all_paths();

//        if (PathCheck)
//            check_all_paths();
        int round = 0;
        while (!_done) {
            scan();
            if (CloneVerbose) {
                zpl("one clone done.")
            }

            round++;
        }

        replace_malloc();
        replace_free();

        string out = SysArgs::get_option("output");
        if (out.empty()) {
            out = SysDict::filename();
            Strings::replace(out, ".ll", ".clone.ll");
            //out = SysDict::filename() + '.' + name();
        }
        SysDict::module()->print_to_file(out);

        check_all_paths();

//
//        _timer.stop();
//        std::ofstream stat_ofs;
//        stat_ofs.open(out + ".timing");
//        stat_ofs << _timer.seconds() << " " << _hot_cxt_counter << " " << _used_hot_cxt << " " << _cloned << " " << _ben_num;
//        stat_ofs.close();

        zpl("======== Summary ======");
        zpl("recog: %d, cxt: %d, recursive: %d, distinct: %d, cloned: %d, round: %d, ben malloc: %d", _recognized, _cxt_counter, _recursive, _all_paths.size(), _cloned, round, _ben_num);
    }

    void replace_malloc() {
        int id = 1;
        for (auto p: _all_paths) {
            auto path = p->path;
            CallInstFamily* ci = path[0];
            string old_callee = ci->called_function()->name();
            //zps(old_callee)

            guarantee(old_callee == "malloc" || old_callee == "calloc" || old_callee == "realloc", " ");
            ci->replace_callee("ben_"+old_callee);
            _ben_num++;
            string new_args = "i32 " + std::to_string(id++) + ", " + ci->get_raw_field("args");
            ci->replace_args(new_args);
        }
    }

    void replace_free() {
        Function* free_fp = SysDict::module()->get_function("free");
        for (auto ci: free_fp->caller_list()) {
            ci->replace_callee("ben_free");
        }

        //todo: non-dirty way
        string suffixes[3] = {" ", ",", ")"};
        for (auto F: SysDict::module()->function_list()) {
            for (auto B: F->basic_block_list()) {
                for (auto I: B->callinst_list()) {
                    for (auto& suf: suffixes) {
                        string targets[4] = {"malloc", "calloc", "realloc", "free"};
                        for (auto& t: targets) {
                            string old = "@"+t+suf;
                            if (I->raw_text().find(old) != string::npos) {
                                Strings::replace(I->raw_text(), old, "@indi_"+t+suf);
                            }
                        }
                    }
                }
            }
        }
    }

    bool insert_declaration(string oldname, string newname, bool add_id=true) {
        Function* func = SysDict::module()->get_function(oldname);

        if (func == NULL) {
            return 0;
        }
        guarantee(func->is_external(), "malloc family should be external");
        //Function* newfunc = SysDict::module()->create_child_function(_new_malloc);

        Function* existed = SysDict::module()->get_function(newname);
        if (existed) {
            return 0; // return if already inserted
        }

        /* manipulate the text */
        string text = func->raw_text();
        string old_call = oldname + '(';
        string new_call = newname + "(i32, ";
        if (!add_id || Strings::startswith(oldname, "f90_")) {  // a bit funky
            new_call = newname + '(';
        }

        Strings::replace(text, old_call, new_call);
        Function* newfunc = IRBuilder::create_function_declaration(text);
        SysDict::module()->insert_function_after(func, newfunc);
    }

    void check_all_paths(bool do_print=false) {
        for (auto v: _all_paths) {
            string callee = v->path[0]->called_function()->name();
            if (do_print)
                printf("%d: %s", v->hotness, callee.c_str());
            for (auto I: v->path) {
                guarantee(I->called_function()->name() == callee, "%s, %s", I->called_function()->name_as_c_str(), callee.c_str());
                if (do_print)
                    printf(" <- %p %s(%d, %d)", I, I->function()->name_as_c_str(), I->parent()->get_index_in_function(), I->get_index_in_block());
                callee = I->function()->name();
                // printf("%s > %s, ", I->called_function()->name_as_c_str(), I->function()->name_as_c_str());
            }
            if (do_print)
                printf("\n");
        }
    }

    void check_path(std::vector<CallInstFamily*>& path, bool do_print=false) {
        string callee = path[0]->called_function()->name();
        bool terminate = false;
        string msg;
        for (auto I: path) {
            if (I->called_function()->name() != callee) {
                terminate = true;
                msg = "I called: " + I->called_function()->name() + " but should call " + callee + " at " + I->function()->name_as_c_str() + ":" + I->get_position_in_function().c_str();
                if (do_print) {
                    zpl("wrong %p called %p: %s", I, I->called_function(), msg.c_str())
                }
            }
            if (do_print)
                printf("%s <- %s(%p, %s)\n", callee.c_str(), I->function()->name_as_c_str(), I, I->get_position_in_function().c_str());
            callee = I->function()->name();

            //zpl("set callee to %s", callee.c_str())

            //guarantee(I->called_function()->name() == callee, "%s, %s", I->called_function()->name_as_c_str(), callee.c_str());
            // printf("%s > %s, ", I->called_function()->name_as_c_str(), I->function()->name_as_c_str());
        }
        if (terminate) {
            guarantee(0, " ");
        }

        if (do_print)
            printf("\n");
    }

    //bool do_finalization(Module* module);
};


REGISTER_PASS(CallClonePass);
