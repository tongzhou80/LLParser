//
// Created by GentlyGuitar on 6/8/2017.
//

#ifndef LLPARSER_PASS_H
#define LLPARSER_PASS_H

#include <string>
#include <cstdio>
#include <map>

class Module;
class Function;
class BasicBlock;
class Instruction;
class Pass;


typedef Pass* (*pass_loader)();
typedef void (*pass_unloader)(Pass*);


/* represent an immutable pass */
class Pass {
    bool _is_dynamic;
    int _priority;
    bool _is_global_pass;
    bool _is_module_pass;
    bool _is_function_pass;
    bool _is_basic_block_pass;
    bool _is_instruction_pass;

    bool _is_parse_time;
    std::map<string, string> _args;
    pass_unloader _unloader;
    string _name;
public:
    Pass();

    int priority()                  { return _priority; }
    void set_priority(int p)        { _priority = p; }

    bool is_dynamic() const {
        return _is_dynamic;
    }

    void set_is_dynamic(bool _is_dynamic=true) {
        Pass::_is_dynamic = _is_dynamic;
    }

    const string &name() const {
        return _name;
    }

    void set_name(const string &_name) {
        Pass::_name = _name;
    }

    void parse_arguments(string args);
    string get_argument(string key);
    string set_argument(string key, string value)          { _args[key] = value; }
    bool has_argument(string key);

    void set_unloader(pass_unloader v)                     { _unloader = v; }
    void unload();

    bool is_global_pass()                          { return _is_global_pass; }
    bool is_module_pass()                          { return _is_module_pass; }
    bool is_function_pass()                        { return _is_function_pass; }
    bool is_basic_block_pass()                     { return _is_basic_block_pass; }
    bool is_instruction_pass()                     { return _is_instruction_pass; }

    void set_is_global_pass(bool v=1)        { _is_global_pass = v; }
    void set_is_module_pass(bool v=1)        { _is_module_pass = v; }
    void set_is_function_pass(bool v=1)      { _is_function_pass = v; }
    void set_is_basic_block_pass(bool v=1)   { _is_basic_block_pass = v; }
    void set_is_instruction_pass(bool v=1)   { _is_instruction_pass = v; }

    bool is_run_at_parse_time()              { return  _is_parse_time; }
    void set_run_at_parse_time(bool v=1)     { _is_parse_time = v; }


    virtual bool run_on_global() {
        printf("Pass.run_on_global called: do nothing\n");
        return false; // not mutate
    }

    virtual bool run_on_module(Module* module) {
        printf("Pass.run_on_module called: do nothing\n");
        return false; // not mutate
    }

    virtual bool run_on_function(Function* func) {
        printf("Pass.run_on_function called: do nothing\n");
        return false; // not mutate
    }

    virtual bool run_on_basic_block(BasicBlock* bb) {
        printf("Pass.run_on_basic_block called: do nothing\n");
        return false; // not mutate
    }

    virtual bool run_on_instruction(Instruction* inst) {
        printf("Pass.run_on_instruction called: do nothing\n");
        return false; // not mutate
    }

    virtual bool parse_epilogue() {
        printf("Pass.epilogue called: used for parse time passes, execute after all the parsing is done\n");
    }

//    virtual bool do_initialization() {
//        printf("Pass.do_initialization called: do nothing\n");
//    }
//
//    virtual bool do_finalization() {
//        printf("Pass.do_finalization called: do nothing\n");
//    }

    virtual bool do_initialization(Module* M) {
        printf("Pass.do_initialization on module called: do nothing\n");
    }

    virtual bool do_finalization(Module* M) {
        printf("Pass.do_finalization on module called: do nothing\n");
    }

    virtual bool do_initialization(Function* F) {
        printf("Pass.do_initialization on module called: do nothing\n");
    }

    virtual bool do_finalization(Function* F) {
        printf("Pass.do_finalization on module called: do nothing\n");
    }

};



#define REGISTER_PASS(classname) \
    extern "C" Pass* __load_pass_##classname() { \
        printf("dynamically load pass " #classname "!\n"); \
        Pass* p = new classname(); \
        p->set_name(#classname); \
        return p; \
    } \
    extern "C" Pass* __unload_pass_##classname(classname* p) { \
        printf("dynamically unload pass " #classname "!\n"); \
        delete p; \
    } \


#endif //LLPARSER_PASS_H
