//
// Created by tzhou on 8/19/17.
//

#include <cstdio>

#include <ir/function.h>
#include <passes/pass.h>
#include <asmParser/sysDict.h>

class GlobalHelloPass: public Pass {
public:
    GlobalHelloPass() {
        set_is_global_pass();
    }

    bool run_on_global() {
        printf("number of modules: %lu\n", SysDict::module_table().size());
    }
};

REGISTER_PASS(GlobalHelloPass)