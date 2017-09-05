//
// Created by tzhou on 8/10/17.
//

#include <set>
#include <algorithm>
#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <ir/di/diEssential.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>

#define FUNC_MAX_LEN 1024

struct XPS_Path {
    std::vector<CallInstFamily*> path;
    int hotness;
};

struct XPS_Caller {
    CallInstFamily* caller;
    XPS_Path* xps_path;
};

class HotCallClonePass: public Pass {
    bool PrintCloning;
    bool TracingVerbose;
    bool MatchVerbose;
    std::set<string> _black;
    bool _has_overlapped_path;
    int _cloned;
    std::ofstream _ofs;
    //std::map<Function*, std::set<CallInstFamily*> > _callers;  // partial callers
    std::vector<XPS_Path*> _all_paths;
    //std::vector<std::vector<CallInstFamily*>> _cold_paths;
    //std::vector<std::vector<CallInstFamily*>> _hot_paths;
    std::map<CallInstFamily*, std::vector<XPS_Caller*>> _callers_map;
    std::map<CallInstFamily*, int> _dr_caller_freq;
    CallInstFamily* _search_root;

    std::vector<CallInstFamily*> _stack;
    string _caller;
    string _callee;
    int _hot_counter;
    int _path_counter;
    int _cxt_counter;
    bool _skip;
    int _recursive;
    bool _done;
    //const int FUNC_MAX_LEN = 1024;
public:
    HotCallClonePass() {
        set_is_module_pass();

        _has_overlapped_path = false;

        PrintCloning = true;
        TracingVerbose = true;
        _path_counter = 1;
        _cxt_counter = 0;
        _skip = false;
        MatchVerbose = 0;
        _recursive = 0;
        _cloned = 0;
        _hot_counter = 0;
    }

    ~HotCallClonePass() {
        printf("pass unloading is not yet implemented! Do stuff in do_finalization!\n");
    }
    void construct_callers_map() {
        _callers_map.clear();
        _dr_caller_freq.clear();
        _search_root = NULL;
        int max_freq = 0;
        for (auto xps_path: _all_paths) {
            auto context = xps_path->path;
            for (int i = 0; i < context.size()-1; ++i) {
                auto I = context[i];

                // for each direct caller of malloc/..
                if (i == 0) {
                    if (_dr_caller_freq.find(I) == _dr_caller_freq.end()) {
                        _dr_caller_freq[I] = 0;
                    }
                    _dr_caller_freq[I]++;
                    if (_dr_caller_freq[I] > max_freq) {
                        max_freq = _dr_caller_freq[I];
                        _search_root = I;
                    }
                }

                if (_callers_map.find(I) == _callers_map.end()) {
                    _callers_map[I] = std::vector<XPS_Caller*>();
                }

                auto& v = _callers_map[I];
                auto new_element = new XPS_Caller();
                new_element->caller = context[i+1];
                new_element->xps_path = xps_path;
                v.push_back(new_element);
            }
        }
    }

    CallInstFamily* get_search_root() {
        std::set<CallInstFamily*> hot_dr_callers;
        CallInstFamily* ret = NULL;
        for (auto xps_path: _all_paths) {
            if (xps_path->hotness == 1)
                hot_dr_callers.insert(xps_path->path[0]);
        }

        for (auto xps_path: _all_paths) {
            if (xps_path->hotness == 0) {
                if (hot_dr_callers.find(xps_path->path[0]) != hot_dr_callers.end()) {
                    ret = xps_path->path[0];
                    return ret;
                }
            }
        }
        return NULL;
    }


    void do_one() {
        construct_callers_map();
        _search_root = get_search_root();
        /* if all direct callers of malloc are distinct, terminates */
        if (!_search_root) {
            _done = true;
            return;
        }

        while (_callers_map[_search_root].size() == 1) {
            _search_root = _callers_map[_search_root][0]->caller;
        }

        auto& callers = _callers_map[_search_root];
        if (callers.size() == 0)
            guarantee(callers.size() != 0, "root: %p", _search_root);
        zpl("root: %p, callers: %d", _search_root, callers.size())
        for (auto i = 1; i < callers.size(); ++i) {
            Function* cloned_callee = _search_root->function()->clone();
            SysDict::module()->append_new_function(cloned_callee);
            _cloned++;
            update_callee_in_path(callers[i], cloned_callee);
        }
    }

    void update_callee_in_path(XPS_Caller* caller, Function* new_callee) {
        auto& stack = caller->xps_path->path;
        for (int i = 1; i < stack.size(); ++i) {
            CallInstFamily* I = stack[i];
            if (I == caller->caller) {
                zpl("in callinst %p repalce %s to %s", I, I->called_function()->name_as_c_str(), new_callee->name_as_c_str())
                I->replace_callee(new_callee->name());

                auto callee_call = stack[i-1];
                int i_index = callee_call->get_index_in_block();
                int b_index = callee_call->parent()->get_index_in_function();
                zpl("in %d path replace %p to %p", caller->xps_path->hotness, stack[i-1], new_callee->get_instruction(b_index, i_index));
                stack[i-1] = static_cast<CallInstFamily*>(new_callee->get_instruction(b_index, i_index));
            }
        }
    }

//    void update_callee_in_all_paths(CallInstFamily* caller, Function* new_callee) {
//        bool is_replaced = false;
//        for (auto& xps_path: _all_paths) {
//            auto stack = xps_path->path;
//            for (int i = 1; i < stack.size(); ++i) {
//                CallInstFamily* I = stack[i];
//                if (I == caller) {
//                    if (!is_replaced) {
//                        zpl("in callinst repalce %s to %s in %p", I->called_function()->name_as_c_str(), new_callee->name_as_c_str(), I)
//                        I->replace_callee(new_callee->name());
//                        is_replaced = true;
//                    }
//
//                    auto callee_call = stack[i-1];
//                    int i_index = callee_call->get_index_in_block();
//                    int b_index = callee_call->parent()->get_index_in_function();
//                    //zpl("in path replace %p to %p", stack[i-1], new_callee->get_instruction(b_index, i_index));
//                    stack[i-1] = static_cast<CallInstFamily*>(new_callee->get_instruction(b_index, i_index));
//                }
//            }
//        }
//
//    }

    // todo: the hot_aps_file must have an appending new line to be correctly parsed for now
    void load_hot_aps_file(string filename) {
        std::ifstream ifs;
        ifs.open(filename);
        if (!ifs.is_open()) {
            fprintf(stderr, "open file %s failed.\n", filename.c_str());
        }
        string line;
        bool is_header = true;
        int hot = 0;
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
                        if (!has_direct_recursion()) {
                            XPS_Path* path = new XPS_Path();
                            path->path = _stack;
                            path->hotness = hot;
                            _all_paths.push_back(path);
                        }
                        _path_counter++;
                    }
                }
                _cxt_counter++;
                _stack.clear();
                _caller.clear();
                _callee.clear();
                _skip = false;
                hot = false;
            }
            else {
                if (is_header) {
                    hot = match_header(line);
                    zpl("header: %s", line.c_str())
                    is_header = false;
                }
                else {
                    if (Strings::conatins(line, "__libc_start_main")) {
                        _skip = true;
                    }

                    if (!_skip) {
                        CallInstFamily* ret = match_callsite(line);
                        _stack.push_back(ret);
                    }
                }
            }
        }
        zpl("recog: %d, cxt: %d, recursive: %d", recognized, _cxt_counter, _recursive);
    }
//
//    void print_all_paths() {
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
        std::vector<XPS_Path*> distinct_set;
        std::set<string> contexts;
        int removed = 0;
        for (auto v: _all_paths) {
            string context;
            for (auto I: v->path) {
                char buf[128];
                sprintf(buf, "%p", I);
                context += string(buf) + ' ';
            }

            if (contexts.find(context) != contexts.end()) {
//                string callee = v[0]->called_function()->name();
//                printf("%s", callee.c_str());
//                for (auto I: v) {
//                    guarantee(I->called_function()->name() == callee, "%s, %s", I->called_function()->name_as_c_str(), callee.c_str());
//                    printf(" <- %s(%p)", I->function()->name_as_c_str(), I);
//                    callee = I->function()->name();
//                    // printf("%s > %s, ", I->called_function()->name_as_c_str(), I->function()->name_as_c_str());
//                }
//                guarantee(0, "re: %s", context.c_str());
                removed++;
            }
            else {
                contexts.insert(context);
                distinct_set.push_back(v);
            }
        }
        _all_paths = distinct_set;

        zpd(removed);
    }

    int match_header(string& line) {
        char hotness[11];  // max hold 0xffffffff + '\0'
        int apid;

        int matched = sscanf(line.c_str(), "%d %s", &apid, hotness);
        guarantee(matched == 2, "Matched: %d, Bad hotset file format: %s", matched, line.c_str());
        if (string(hotness) == "0xffffffff") {
            return 1;
        }
        else {
            return 0;
        }
    }

    CallInstFamily* match_callsite(string & line) {
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

        CallInstFamily* ret = NULL;

        if (_callee.empty()) {
            ret = approximately_match_alloc(file, line_num);
            if (!ret) {
                _skip = true;
            }
        }
        else {
            ret = approximately_match(file, line_num);
        }
        if (_callee == "gen_adddi3") {
            zpl("kkk: %s", _caller.c_str());
        }
        _callee = _caller;

        return ret;
    }

    CallInstFamily* approximately_match_alloc(string filename, int line) {
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
            _caller = final->function()->name();
        }

        if (MatchVerbose) {
            if (final) {
                DILocation *loc = final->debug_loc();
                guarantee(loc, "This pass needs full debug info, please compile with -g");
                printf("alloc: (%s, %s, %s, %d)\n",
                       final->function()->name_as_c_str(),
                       final->called_function()->name_as_c_str(),
                       filename.c_str(), line);

            } else {
                printf("(%s, %s, %s, %d) => None\n", _caller.c_str(), _callee.c_str(), filename.c_str(), line);
            }
        }

        return final;
    }

    CallInstFamily* approximately_match(string filename, int line) {
        Function* calleef = SysDict::module()->get_function(_callee);

        CallInstFamily* final = NULL;
        guarantee(calleef, " ");
        auto users = calleef->caller_list();

        // level 0
        if (users.size() == 1) {
            final = users[0];
        }

        // level 1
        std::map<CallInstFamily*, int> users_offsets;
        std::vector<CallInstFamily*> other_callers;
        if (!final) {
            for (auto I: calleef->user_list()) {
                if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(I)) {
                    DILocation *loc = ci->debug_loc();
                    guarantee(loc, "This pass needs full debug info, please compile with -g");
                    if (Strings::conatins(filename, loc->filename()) && std::abs(line-loc->line()) < 10) {
                        //if (ci->owner() == _caller) {
                        users_offsets[ci] = std::abs(line-loc->line());
                    }
                    else {
                        other_callers.push_back(ci);
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

        /* not do this for now */
//        if (!final && !other_callers.empty()) {
//            final = other_callers[0];
//        }

        if (final) {
            if (!_caller.empty()) {
                if (final->owner() != _caller) {
                    return NULL;
                }
            }
            else {
                _caller = final->owner();
                zpl("infer caller: %s", _caller.c_str())
            }


            if (_callee == "gen_adddi3") {
                zpl("kkk: %s, caller: %s", final->owner().c_str(), _caller.c_str());
            }
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



//    void add_partial_caller(Function* callee, CallInstFamily* user) {
//        if (_callers.find(callee) == _callers.end()) {
//            std::set<CallInstFamily*> callers;
//            _callers[callee] = callers;
//        }
//        _callers[callee].insert(user);
//    }

    bool has_recursion() {
        std::set<string> chain;
        string alloc = _stack[0]->called_function()->name();
        chain.insert(alloc);
        for (auto I: _stack) {
            string caller = I->function()->name();
            if (chain.find(caller) != chain.end()) {
                return true;
            }
            else {
                chain.insert(caller);
            }
        }
        return false;
    }

    bool has_direct_recursion() {
        std::map<CallInstFamily*, int> counters;
        for (auto I: _stack) {
            counters[I]++;
        }
        for (auto i: counters) {
            if (i.second > 2) {
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


    bool run_on_module(Module* module) {
        insert_declaration("malloc", "ben_malloc", true);
        insert_declaration("calloc", "ben_calloc", true);
        insert_declaration("realloc", "ben_realloc", true);

        string arg_name = "hot_aps_file";
        if (has_argument(arg_name)) {
            string hot_aps_file = get_argument(arg_name);
            load_hot_aps_file(hot_aps_file);
        }
        get_distinct_all_paths();
//        print_all_paths();
        int round = 0;
        while (!_done) {
            do_one();
            zpl("one clone")
            round++;
            //print_all_paths();
        }

        zpd(_cloned)
        zpd(round)
        replace_malloc();

        string out = SysArgs::get_option("output");
        if (out.empty()) {
            out = SysDict::filename() + '.' + name();
        }
        SysDict::module()->print_to_file(out);
    }

    void replace_malloc() {
        for (auto p: _all_paths) {
            auto path = p->path;
            if (p->hotness == 1) {
                CallInstFamily* ci = path[0];
                string old_callee = ci->called_function()->name();

                if (old_callee.find("ben_") == 0) {
                    continue;
                }

                guarantee(old_callee == "malloc" || old_callee == "calloc" || old_callee == "realloc", " ");
                ci->replace_callee("ben_"+old_callee);
                string new_args = "i32 " + std::to_string(_hot_counter) + ", " + ci->get_raw_field("args");
                ci->replace_args(new_args);
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

    void traverse(string bottom) {
        Function* malloc = SysDict::module()->get_function(bottom);
        int cnt = 0;

        do {
            if (TracingVerbose) {
                printf("round: %d\n", cnt);
            }

            cnt++;
            _black.clear();
            _has_overlapped_path = false;  // always assume this round is the last round
            //auto& users = _callers[malloc];
            auto& users = malloc->user_list();
            auto users_copy = users; // user_list might change during the iteration since new functions may be created
            for (auto uit = users_copy.begin(); uit != users_copy.end(); ++uit) {
                Function* func = (*uit)->function();
                do_clone(func);
            }
        } while (_has_overlapped_path && cnt < 10);

        string out = SysDict::filename() + name();
        SysDict::module()->print_to_file(out);
    }

    void do_clone(Function* f) {
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
        auto users_copy = users;

        int num = 0;
        for (auto it = users_copy.begin(); it != users_copy.end(); ++it, ++num) {
            CallInstFamily* ci = dynamic_cast<CallInstFamily*>(*it);
            guarantee(ci, " ");
            if (num > 0) {
                _has_overlapped_path = true;  // set the flag to scan again
                Function* fclone = f->clone();
                SysDict::module()->append_new_function(fclone);

                if (PrintCloning) {
                    printf("cloned %s to %s\n", f->name_as_c_str(), fclone->name_as_c_str());
                }
                ci->replace_callee(fclone->name());
            }

            if (_black.find(ci->function()->name()) == _black.end()) {
                do_clone(ci->function());
            }
        }
    }

    //bool do_finalization(Module* module);
};


REGISTER_PASS(HotCallClonePass);
