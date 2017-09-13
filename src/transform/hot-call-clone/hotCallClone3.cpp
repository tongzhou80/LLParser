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
    bool PathCheck;
    bool MatchVerbose;
    bool CloneVerbose;
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
    int _min_cxt;
    int _hot_counter;
    int _path_counter;
    int _cxt_counter;
    bool _skip;
    int _recursive;
    int _recognized;
    int _ben_num;
    int _distinct_num;
    bool _done;
    //const int FUNC_MAX_LEN = 1024;
public:
    HotCallClonePass() {
        set_is_module_pass();

        CloneVerbose = false;
        MatchVerbose = false;
        PathCheck = false;
        _min_cxt = 2;
        _path_counter = 1;
        _cxt_counter = 0;
        _skip = false;
        _recursive = 0;
        _cloned = 0;
        _ben_num = 0;
        _recognized = 0;
        _hot_counter = 0;
        _distinct_num = 0;
    }

    ~HotCallClonePass() {

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
                bool existed = false;
                for (auto xps_caller: v) {
                    if (xps_caller->caller == new_element->caller) {
                        existed = true;
                    }
                }
                if (!existed)
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

    CallInstFamily* get_element_in_call_path(std::vector<CallInstFamily*>& path, int pos) {
        if (pos < path.size()) {
            return path[pos];
        }
        else {
            return NULL;
        }
    }

    void do_one() {
        std::map<CallInstFamily*, XPS_Path*> hot_dr_callers;
        CallInstFamily* ret = NULL;
        for (auto xps_path: _all_paths) {
            if (xps_path->hotness == 1)
                hot_dr_callers[xps_path->path[0]] = xps_path;
        }
        //check_all_paths();

        bool in_cold_path = false;
        for (auto xps_path: _all_paths) {
            if (xps_path->hotness == 0) {
                auto root = xps_path->path[0];
                if (hot_dr_callers.find(root) != hot_dr_callers.end()) {
                    in_cold_path = true;

                    /* let the cold path call the clone */
                    auto& hot_path = hot_dr_callers[root];
                    auto& cold_path = xps_path;
                    if (PathCheck) {
                        zpl("before clone:")
                        check_path(hot_path->path);
                        check_path(cold_path->path);
                    }
                    int i = 1; // go upwards until find two different callers
                    while (get_element_in_call_path(hot_path->path, i) == get_element_in_call_path(cold_path->path, i)) {
                        if (CloneVerbose) {
                            zpl("go up: %s => %s", root->function()->name_as_c_str(), hot_path->path[i]->function()->name_as_c_str())
                        }

                        root = hot_path->path[i];
                        i++;
                    }
                    if (CloneVerbose) {
                        zpl("root: %p %s %s", root, root->function()->name_as_c_str(), root->get_position_in_function().c_str());
                    }

                    auto hot_parent = get_element_in_call_path(hot_path->path, i);
                    auto cold_parent = get_element_in_call_path(cold_path->path, i);

                    Function* cloned_callee = root->function()->clone();
                    SysDict::module()->append_new_function(cloned_callee);
                    _cloned++;
                    //update_callee_in_path(callers[i], cloned_callee);

                    if (hot_parent) {
                        update_callee_in_all_paths(hot_parent, cloned_callee);
                    }
                    else {
                        update_callee_in_all_paths(cold_parent, cloned_callee);
                    }

                    // XPS_Caller* caller = new XPS_Caller();
                    // caller->caller = get_element_in_call_path(cold_path->path, i);
                    // caller->xps_path = cold_path;
                    // //update_callee_in_path(caller, cloned_callee);
                    if (PathCheck) {
                        zpl("after clone:")
                        check_path(hot_path->path);
                        check_path(cold_path->path);
                    }

                    break;
                }
            }
        }

        if (!in_cold_path) {
            _done = true;
        }

    }
//
    void update_callee_in_path(XPS_Caller* caller, Function* new_callee) {
        auto& stack = caller->xps_path->path;
        if (caller->caller == NULL && stack[stack.size()-1]->function() == new_callee->copy_prototype()) {
            auto callee_call = stack[stack.size()-1];
            int i_index = callee_call->get_index_in_block();
            int b_index = callee_call->parent()->get_index_in_function();
            //zpl("in %d path replace %p to %p", caller->xps_path->hotness, stack[i-1], new_callee->get_instruction(b_index, i_index));
            auto ci_in_new_callee = static_cast<CallInstFamily*>(new_callee->get_instruction(b_index, i_index));
            stack[stack.size()-1] = ci_in_new_callee;
            zpl("path pos %d to %s", stack.size()-1, ci_in_new_callee->function()->name_as_c_str())
            check_path(stack);
            return;
        }

        for (int i = 1; i < stack.size(); ++i) {
            CallInstFamily* I = stack[i];
            if (I == caller->caller) {
                zpl("in callinst %p repalce %s to %s", I, I->called_function()->name_as_c_str(), new_callee->name_as_c_str())
                I->replace_callee(new_callee->name());

                auto callee_call = stack[i-1];
                int i_index = callee_call->get_index_in_block();
                int b_index = callee_call->parent()->get_index_in_function();
                zpl("in %d path replace %p to %p", caller->xps_path->hotness, stack[i-1], new_callee->get_instruction(b_index, i_index));
                auto ci_in_new_callee = static_cast<CallInstFamily*>(new_callee->get_instruction(b_index, i_index));
                stack[i-1] = ci_in_new_callee;
                zpl("path pos %d to %s", i-1, ci_in_new_callee->function()->name_as_c_str())
                check_path(stack);
            }
        }
    }

    void update_callee_in_all_paths(CallInstFamily* caller, Function* new_callee) {
        bool is_replaced = false;
        for (auto& xps_path: _all_paths) {
            auto& stack = xps_path->path;
            guarantee(caller != NULL, " ");
            if (caller == NULL && stack[stack.size()-1]->function() == new_callee->copy_prototype()) {
                auto callee_call = stack[stack.size()-1];
                int i_index = callee_call->get_index_in_block();
                int b_index = callee_call->parent()->get_index_in_function();
                //zpl("in %d path replace %p to %p", caller->xps_path->hotness, stack[i-1], new_callee->get_instruction(b_index, i_index));
                auto ci_in_new_callee = static_cast<CallInstFamily*>(new_callee->get_instruction(b_index, i_index));
                stack[stack.size()-1] = ci_in_new_callee;
                zpl("path pos %d to %s", stack.size()-1, ci_in_new_callee->function()->name_as_c_str())
                    //check_path(stack);
                return;
            }

            for (int i = 1; i < stack.size(); ++i) {

                CallInstFamily* I = stack[i];
                if (I == caller) {
                    if (!is_replaced) {
                        zpl("in callinst repalce %s to %s in %p", I->called_function()->name_as_c_str(), new_callee->name_as_c_str(), I)
                        I->replace_callee(new_callee->name());
                        is_replaced = true;
                    }

                    auto callee_call = stack[i-1];
                    int i_index = callee_call->get_index_in_block();
                    int b_index = callee_call->parent()->get_index_in_function();
                    //zpl("in %d path replace %p to %p", caller->xps_path->hotness, stack[i-1], new_callee->get_instruction(b_index, i_index));
                    auto ci_in_new_callee = static_cast<CallInstFamily*>(new_callee->get_instruction(b_index, i_index));
                    stack[i-1] = ci_in_new_callee;
                    zpl("path pos %d to %s", i-1, ci_in_new_callee->function()->name_as_c_str())
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
        int hot = 0;

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
                            XPS_Path* path = new XPS_Path();
                            path->path = _stack;
                            path->hotness = hot;
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
                hot = false;
            }
            else {
                if (is_header) {
                    header = line;
                    hot = match_header(line);
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
        std::vector<XPS_Path*> distinct_set;
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
        guarantee(Strings::contains(line, "("), " ");
        line = line.substr(line.find('('));  // strip the exe name before '('

        int pos1 = line.find(' ');
        string bt_symbol = line.substr(0, pos1);
        string fileline = line.substr(pos1+1);

        if (bt_symbol != "()") {
            int pos2 = bt_symbol.find('+');
            _caller = bt_symbol.substr(1, pos2-1);
            string offset = bt_symbol.substr(pos2+1);

            if (Alias* alias = SysDict::module()->get_alias(_caller)) {
                _caller = dynamic_cast<Function*>(alias->aliasee())->name();
            }
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
        _callee = _caller;

        return ret;
    }

    CallInstFamily* approximately_match_alloc(string filename, int line) {
        CallInstFamily* final = NULL;

        // level 1
        std::map<CallInstFamily*, int> users_offsets;
        std::vector<CallInstFamily*> other_callers;

        //std::vector<string> candidates = {"malloc", "calloc", "realloc", "_Znam", "_Znwm", "_ZdaPv", "_ZdlPv"};
        std::vector<string> candidates = {"malloc", "calloc", "realloc"};
        for (auto c: candidates) {
            if (Function* alloc = SysDict::module()->get_function(c)) {
                for (auto I: alloc->user_list()) {
                    if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(I)) {
                        DILocation *loc = ci->debug_loc();
                        guarantee(loc, "This pass needs full debug info, please compile with -g");
                        if (Strings::contains(filename, loc->filename())
                            && std::abs(line-loc->line()) < 10) {
                            if (SysDict::module()->language() == Module::Language::cpp) {
                                if (loc->function_linkage_name() == _caller) {
                                    users_offsets[ci] = std::abs(line-loc->line());
                                }
                            }
                            else {
                                users_offsets[ci] = std::abs(line-loc->line());
                            }

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

        if (final) {
            _caller = final->function()->name();
            //zpl("alloc caller: %s, %p", _caller.c_str(), SysDict::module()->get_alias(_caller))
            if (Alias* alias = SysDict::module()->get_alias(_caller)) {
                _caller = dynamic_cast<Function*>(alias->aliasee())->name();
            }

            if (dynamic_cast<InvokeInst*>(final)) {
                zpl("got invoke")
            }
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
        if (_caller == "eo_fermion_force" && line == 979) {
            line = 977;
        }
        Function* calleef = SysDict::module()->get_function(_callee);

        CallInstFamily* final = NULL;
        guarantee(calleef, "Function %s not found", _callee.c_str());
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
                    if (Strings::contains(filename, loc->filename()) &&  std::abs(line-loc->line()) < 10) {
                        if (MatchVerbose) {
                            printf("loc->filename(): %s, loc->line(): %d, loc function: %s\n",
                                   loc->filename().c_str(), loc->line(), loc->function_linkage_name().c_str());
                            printf("filename: %s, line: %d, caller: %s\n\n",
                                   filename.c_str(), line, _caller.c_str());
                        }

                        if (SysDict::module()->language() == Module::Language::cpp) {
                            if (loc->function_linkage_name() == _caller) {
                                users_offsets[ci] = std::abs(line-loc->line());
                            }
                        }
                        else {
                            if (loc->function_linkage_name() == _caller) {
                                users_offsets[ci] = std::abs(line-loc->line());
                            }
                            //users_offsets[ci] = std::abs(line-loc->line());
                        }
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
                if (Alias* alias = SysDict::module()->get_alias(_caller)) {
                    _caller = dynamic_cast<Function*>(alias->aliasee())->name();
                }

                if (final->owner() != _caller) {
                    return NULL;
                }
            }
            else {
                _caller = final->owner();
                zpl("infer caller: %s", _caller.c_str())
            }

        }



        if (MatchVerbose) {
            if (final) {
                DILocation *loc = final->debug_loc();
                guarantee(loc, "This pass needs full debug info, please compile with -g");
                printf("(%s, %s, %s, %d) => (%p, %s, %s, %s, %d)\n", _caller.c_str(), _callee.c_str(), filename.c_str(), line,
                       final, loc->function().c_str(), _callee.c_str(), loc->filename().c_str(), loc->line());

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


    bool run_on_module(Module* module) {
        Timer timer;
        timer.start();
        if (has_argument("min-cxt")) {
            _min_cxt = std::stoi(get_argument("min-cxt"));
            zpd(_min_cxt)
        }
        insert_declaration("malloc", "ben_malloc", true);
        insert_declaration("calloc", "ben_calloc", true);
        insert_declaration("realloc", "ben_realloc", true);

        string arg_name = "hot_aps_file";
        if (has_argument(arg_name)) {
            string hot_aps_file = get_argument(arg_name);
            load_hot_aps_file(hot_aps_file);
        }
        get_distinct_all_paths();

        if (PathCheck)
            check_all_paths();
        int round = 0;
        while (!_done) {
            do_one();
            if (CloneVerbose) {
                zpl("one clone done.")
            }

            round++;
        }

        replace_malloc();

        string out = SysArgs::get_option("output");
        if (out.empty()) {
            out = SysDict::filename() + '.' + name();
        }
        zps(out.c_str())
        SysDict::module()->print_to_file(out);

        if (PathCheck) {
            check_all_paths();
        }


        timer.stop();
        std::ofstream stat_ofs;
        stat_ofs.open(out + ".timing");
        stat_ofs << timer.seconds() << " " << _cxt_counter << " " << _all_paths.size() << " " << _cloned;
        stat_ofs.close();

        zpl("======== Summary ======");
        zpl("recog: %d, cxt: %d, recursive: %d, distinct: %d, cloned: %d, round: %d, ben malloc: %d", _recognized, _cxt_counter, _recursive, _all_paths.size(), _cloned, round, _ben_num);
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
                _ben_num++;
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


REGISTER_PASS(HotCallClonePass);
