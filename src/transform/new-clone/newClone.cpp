//
// Created by tlaber on 8/18/17.
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

class NewClonePass: public Pass {
    bool PrintCloning;
    bool MatchVerbose;
    bool TracingVerbose;
    std::set<string> _black;
    bool _has_overlapped_path;
    std::ofstream _ofs;
    std::map<Function*, std::set<Instruction*> > _callers;  // partial callers
    std::vector<Instruction*> _stack;
    int _path_counter;
    //const int FUNC_MAX_LEN = 1024;
public:
    NewClonePass() {
        set_is_module_pass();

        _has_overlapped_path = false;

        PrintCloning = false;
        MatchVerbose = 1;
        TracingVerbose = false;
        _path_counter = 1;
    }

    ~NewClonePass() {
        printf("pass unloading is not yet implemented! Do stuff in do_finalization!\n");
    }

    int match_header(string& line) {
        char hotness[11];  // max hold 0xffffffff + '\0'
        int apid;

        int matched = sscanf(line.c_str(), "%d %s", &apid, hotness);
        guarantee(matched == 2, "Matched: %d, Bad hotset file format: %s", matched, line.c_str());
    }

    Instruction* match_callsite(string & line) {
        char caller_callee[FUNC_MAX_LEN] = {0};
        //char callee_name[FUNC_MAX_LEN] = {0};
        char file[FUNC_MAX_LEN] = {0};
        char line_num[64];

        int matched = sscanf(line.c_str(), "(%[^)])%[^:]:%s", caller_callee, file, line_num);

        guarantee(matched == 3, "Matched: %d, Bad hotset file format: %s", matched, line.c_str());


        string s = string(caller_callee);
        int pos = s.find(' ');
        guarantee(pos != s.npos, " ");
        string caller = s.substr(0, pos);
        string callee = s.substr(pos+1);
        if (!callee.empty()) {
            return approximately_match(caller, callee, file, atoi(line_num));
        }
        else {
            return approximately_match_alloc(caller, file, atoi(line_num));
        }

//
//        Function *callee = SysDict::module()->get_function(callee_name);
//        guarantee(callee, "Callee %s not found", callee_name);
//        auto &users = callee->user_list();
//        for (auto uit = users.begin(); uit != users.end(); ++uit) {
//            Instruction* I = *uit;
//            DILocation *loc = I->debug_loc();
//            guarantee(loc, "This pass needs full debug info, please compile with -g");
//            if (loc->filename() == string(file) &&
//                loc->line() == line_num &&
//                loc->column() == column) {
//                //I->dump();
//                add_partial_caller(callee, I);
//            }
//        }

    }

    Instruction* approximately_match_alloc(string caller, string filename, int line) {
        Function* callerf = SysDict::module()->get_function(caller);
        std::map<Instruction*, int> offsets;
        for (auto bit = callerf->begin(); bit != callerf->end(); ++bit) {
            BasicBlock* B = *bit;
            for (auto iit = B->callinst_list().begin(); iit != B->callinst_list().end(); ++iit) {
                Instruction* I = *iit;
                //CallInst* ci = (CallInst*)I;
                CallInst* ci = static_cast<CallInst*>(I);
                if (ci->called_function()) {
                    string callee = ci->called_function()->name();
                    if (callee == "malloc" || callee == "calloc" || callee == "realloc") {
                        DILocation* loc = ci->debug_loc();
                        guarantee(loc, " ");
                        offsets[I] = std::abs(loc->line()-line);
                    }
                }

            }
        }

        if (offsets.size() == 0) {
            return NULL;
        }
        int closest = offsets.begin()->second;
        Instruction* closest_I = offsets.begin()->first;

        for (auto it = offsets.begin(); it != offsets.end(); ++it) {
            if (it->second < closest) {
                closest = it->second;
                closest_I = it->first;
            }
        }
        return closest_I;
    }


    Instruction* approximately_match(string caller, string callee, string filename, int line) {
        Function* callerf = SysDict::module()->get_function(caller);
        Function* calleef = SysDict::module()->get_function(callee);
        Instruction* final = NULL;
        guarantee(calleef && callerf, " ");
        auto users = calleef->user_list();

        if (users.size() == 0) {
            return NULL;
        }

        // level 0
        if (users.size() == 1) {
            final = users[0];
        }

        if (caller == "BZ2_bzReadOpen") {
            //zpl("kkk")
        }

        // level 1
        std::map<Instruction*, int> users_offsets;
        if (!final) {
            for (auto uit = users.begin(); uit != users.end(); ++uit) {
                Instruction* I = *uit;
                DILocation *loc = I->debug_loc();
                guarantee(loc, "This pass needs full debug info, please compile with -g");
                if (I->parent()->parent() == callerf && Strings::conatins(filename, loc->filename())) {
                    users_offsets[I] = std::abs(line-loc->line());
                }
            }

            if (users_offsets.size() == 0) {
                //guarantee(0, "call for statistics");
                return NULL;
            }
            else if (users_offsets.size() == 1) {
                final = users_offsets.begin()->first;
            }
        }

        // level 2
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

        if (MatchVerbose) {
            DILocation *loc = final->debug_loc();
            guarantee(loc, "This pass needs full debug info, please compile with -g");
            printf("(%s, %s, %s, %d) => (%s, %s, %s, %d)\n", caller.c_str(), callee.c_str(), filename.c_str(), line,
                   loc->function().c_str(), callee.c_str(), loc->filename().c_str(), loc->line());
        }
        return final;
    }

    // todo: the hot_aps_file must have an appending new line to be correctly parsed for now
    void load_hot_aps_file(string filename) {
        std::ifstream ifs;
        ifs.open(filename);
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
                    zpl("has all: %d", has_all)
                    if (has_all) {
                        recognized++;
                        clone_call_path(_stack);
                        _path_counter++;
                    }
                    _stack.clear();
                }
            }
            else {
                if (is_header) {
                    match_header(line);
                    zpl("header: %s", line.c_str())
                    is_header = false;
                }
                else {
                    Instruction* ret = match_callsite(line);
                    _stack.push_back(ret);
                }
            }
        }
        zpl("recog: %d", recognized);
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
                cloned_caller = caller->clone(caller->name()+'.'+std::to_string(_path_counter));
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
                cloned_ci->replace_callee(callee->name()+'.'+std::to_string(_path_counter));
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
        
        string out = SysDict::filename();
        if (out.empty()) {
            out = "new";
        }
        out += ".newClone";
        if (SysArgs::has_property("output")) {
            out = SysArgs::get_property("output");
        }

        zpl("counter: %d", _path_counter);
        SysDict::module()->print_to_file(out);
    }

    //bool do_finalization(Module* module);
};

REGISTER_PASS(NewClonePass);
