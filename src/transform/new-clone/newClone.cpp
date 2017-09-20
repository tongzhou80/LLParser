//
// Created by tlaber on 8/18/17.
//

#include <algorithm>
#include <set>
#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <di/diEssential.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>
#include <cstring>
#include <iomanip>

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

// TODO: cloning the whole path one by one might make alter other paths if they overlap, switch to a different algorithm

class NewClonePass: public Pass {
    bool PrintCloning;
    bool MatchVerbose;
    bool TracingVerbose;

    std::ofstream _ofs;
    std::vector<Instruction*> _stack;
    string _caller;
    string _callee;
    int _path_counter;
    int _cxt_counter;
    bool _skip;
    //const int FUNC_MAX_LEN = 1024;
public:
    NewClonePass() {
        set_is_module_pass();

        PrintCloning = false;
        MatchVerbose = 1;
        TracingVerbose = false;
        _path_counter = 1;
        _cxt_counter = 0;
        _skip = false;
    }

    ~NewClonePass() {

    }

    int match_header(string& line) {
        char hotness[11];  // max hold 0xffffffff + '\0'
        int apid;

        int matched = sscanf(line.c_str(), "%d %s", &apid, hotness);
        guarantee(matched == 2, "Matched: %d, Bad hotset file format: %s", matched, line.c_str());
    }

    Instruction* match_callsite(string & line) {
        guarantee(Strings::contains(line, "("), " ");
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
            //return approximately_match(file, line_num);
        }
        else {
            ret = approximately_match(file, line_num);
        }
        _callee = _caller;
        zpl("new callee: %s", _callee.c_str())
        return ret;
    }
//
//    Instruction* match_callsite(string & line) {
//        char caller_callee[FUNC_MAX_LEN] = {0};
//        //char callee_name[FUNC_MAX_LEN] = {0};
//        char file[FUNC_MAX_LEN] = {0};
//        char line_num[64];
//
//        int matched = sscanf(line.c_str(), "(%[^)])%[^:]:%s", caller_callee, file, line_num);
//
//        guarantee(matched == 3, "Matched: %d, Bad hotset file format: %s", matched, line.c_str());
//
//
//        string s = string(caller_callee);
//        int pos = s.find(' ');
//        guarantee(pos != s.npos, " ");
//        string caller = s.substr(0, pos);
//        string callee = s.substr(pos+1);
//        if (!callee.empty()) {
//            return approximately_match(caller, callee, file, atoi(line_num));
//        }
//        else {
//            return approximately_match_alloc(caller, file, atoi(line_num));
//        }
//
//    }

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
                    for (auto I: alloc->user_set()) {
                        if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(I)) {
                            DILocation *loc = ci->debug_loc();
                            guarantee(loc, "This pass needs full debug info, please compile with -g");
                            if (Strings::contains(filename, loc->filename())) {
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

//        guarantee(!_caller.empty(), " ");
//        Function* callerf = SysDict::module()->get_function(_caller);
//        std::map<Instruction*, int> offsets;
//        for (auto bit = callerf->begin(); bit != callerf->end(); ++bit) {
//            BasicBlock* B = *bit;
//            for (auto iit = B->callinst_list().begin(); iit != B->callinst_list().end(); ++iit) {
//                Instruction* I = *iit;
//                //CallInst* ci = (CallInst*)I;
//                CallInst* ci = static_cast<CallInst*>(I);
//                if (ci->called_function()) {
//                    string callee = ci->called_function()->name();
//                    if (callee == "malloc" || callee == "calloc" || callee == "realloc") {
//                        DILocation* loc = ci->debug_loc();
//                        guarantee(loc, " ");
//                        offsets[I] = std::abs(loc->line()-line);
//                    }
//                }
//
//            }
//        }
//
//        if (offsets.size() == 0) {
//            return NULL;
//        }
//        int closest = offsets.begin()->second;
//        Instruction* closest_I = offsets.begin()->first;
//
//        for (auto it = offsets.begin(); it != offsets.end(); ++it) {
//            if (it->second < closest) {
//                closest = it->second;
//                closest_I = it->first;
//            }
//        }
//        return closest_I;
    }

    // todo: use CallInstFamily
    Instruction* approximately_match(string filename, int line) {

        Function* calleef = SysDict::module()->get_function(_callee);
        Instruction* final = NULL;
        guarantee(calleef, " ");
        auto users = calleef->user_set();

        // level 0
        if (users.size() == 1) {
            final = users[0];
        }

        // level 1
        std::map<Instruction*, int> users_offsets;
        std::vector<Instruction*> other_callers;
        if (!final) {
            for (auto I: calleef->user_set()) {
                if (CallInst* ci = dynamic_cast<CallInst*>(I)) {
                    DILocation *loc = ci->debug_loc();
                    guarantee(loc, "This pass needs full debug info, please compile with -g");
                    if (Strings::contains(filename, loc->filename())) {
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
                        clone_call_path(_stack);
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
                    if (Strings::contains(line, "__libc_start_main")) {
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

    void clone_call_path(std::vector<Instruction*>& stack) {
        string new_callee;
        for (int i = 0; i < stack.size(); ++i) {
            CallInst* ci = dynamic_cast<CallInst*>(stack[i]);
            guarantee(ci, " ");
            Function* caller = ci->function();
            Function* callee = ci->called_function();
            int i_index = ci->get_index_in_block();
            int b_index = ci->parent()->get_index_in_function();
            Function* cloned_caller;

            if (caller->name() == "main") {
                cloned_caller = caller;
            }
            else {
                cloned_caller = caller->clone();
                SysDict::module()->append_new_function(cloned_caller);
            }
            CallInst* cloned_ci = dynamic_cast<CallInst*>(cloned_caller->get_instruction(b_index, i_index));

            if (i == 0) {
                string callee_name = callee->name();
                guarantee(callee_name == "malloc" || callee_name == "calloc" || callee_name == "realloc", " ");

                modify_callinst(cloned_ci, "ben_malloc", true);
                //Instruction* neu = create_new_call(cloned_ci, "ben_malloc", true);
                //cloned_ci->parent()->replace(cloned_ci, neu);  // will not delete I; I will become an orphan
            }
            else {
                cloned_ci->replace_callee(new_callee);
            }
            new_callee = cloned_caller->name();
        }

        string top_caller = stack[stack.size()-1]->owner();
        if (top_caller != "main") {
            update_top_caller(new_callee, stack);
        }
    }

    void update_top_caller(string new_name, std::vector<Instruction*>& stack) {
        Function* caller = stack[stack.size()-1]->function();
        auto users = caller->user_set();
        for (auto I: users) {
            if (CallInst* ci = dynamic_cast<CallInst*>(I)) {
                if (std::find(stack.begin(), stack.end(), ci) == stack.end()) {
                    zpl("replaced callee %s to %s in %s", ci->called_function()->name_as_c_str(), new_name.c_str(), ci->owner().c_str())
                    ci->replace_callee(new_name);
                }
            }
        }
    }

    void modify_callinst(CallInstFamily* ci, string new_callee, bool add_id=true) {
        zpl("old inst %s", ci->raw_text().c_str());

        // only when add_id is on will it be an allocation site
        if (add_id) {
            _ofs << std::setiosflags(std::ios::left);
            _ofs << std::setw(10) << _path_counter;

            int default_tier = 0;
            _ofs << std::setw(10) << default_tier;

            /* disable debug info for a while, I want to rewrite the debug info parsing system */
            DILocation* loc = ci->debug_loc();
            if (loc) {
                printf("  in %s:%d (%s)\n", loc->filename().c_str(), loc->line(), loc->function().c_str());
                string loc_string = loc->filename() + ':' + std::to_string((long long)loc->line());
                loc_string += '(' + loc->function() + ')';
                _ofs << std::setw(20) << loc_string;
            }
            _ofs << std::endl;
        }
        else {
            DILocation* loc = ci->debug_loc();
            if (loc) {
                printf("  in %s:%d (%s)\n", loc->filename().c_str(), loc->line(), loc->function().c_str());
            }
        }

        /* not consider indirect call here */
        ci->replace_callee(new_callee);

        string new_args;
        if (add_id) {
            new_args = "i32 " + std::to_string(_path_counter) + ", " + ci->get_raw_field("args");
        }
        else {
            new_args = ci->get_raw_field("args");
        }
        ci->replace_args(new_args);


        if (ci->is_varargs()) {
            if (add_id) {
                string args_sig = ci->get_raw_field("fnty");
                guarantee(!args_sig.empty(), "problematic line: %s", ci->raw_c_str());
                string new_sig = args_sig;
//            zpl("old sig: %s", args_sig.c_str());
//            zpl("new sig: %s", new_sig.c_str());

                new_sig.insert(1, "i32, ");
                Strings::replace(ci->raw_text(), args_sig, new_sig);
            }
        }

        zpl("new inst %s\n", ci->raw_text().c_str());
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
        
        string out = SysDict::filename() + '.' + name();

        zpl("counter: %d", _path_counter);
        SysDict::module()->print_to_file(out);
    }

    //bool do_finalization(Module* module);
};

REGISTER_PASS(NewClonePass);
