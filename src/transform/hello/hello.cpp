//
// Created by GentlyGuitar on 6/8/2017.
//



#include <cstdio>

#include <ir/function.h>
#include <passes/pass.h>

class HelloPass: public Pass {
public:
    HelloPass() {
        set_is_function_pass();
    }

    bool do_initialization(Module*)  {}

    bool run_on_function(Function* func) {
        printf("hello function: %s\n", func->name().c_str());
    }

    bool do_finalization(Module* m)  {
        printf("function count: %d\n", m->function_list().size());
    }
};

REGISTER_PASS(HelloPass)
