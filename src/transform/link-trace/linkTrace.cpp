//
// Created by tzhou on 10/28/17.
//

#include <cstdio>
#include <ir/function.h>
#include <passes/pass.h>
#include <asmParser/sysDict.h>

class LinkTracePass: public Pass {
    string _target;
    std::map<string, Module*> _functions;
public:
    LinkTracePass() {
        set_is_global_pass();
    }

    bool run_on_global() override {
        if (!has_argument("target")) {
            std::cerr << "LinkTrace pass has a required argument `target` that should be the name of the linked file! Exit...\n";
            exit(1);
        }
        _target = get_argument("target");

        Module* target = SysDict::get_module(_target);

        /* construct a map between function name and the module */
        for (auto it: SysDict::module_table()) {
            if (it.first == _target) {
                continue;
            }

            Module* M = it.second;
            for (auto F: M->function_list()) {
                if (!F->is_defined()) {
                    continue;
                }
                
                if (_functions.find(F->name()) != _functions.end()) {
                    fprintf(stderr, "name collision:\n");
                    fprintf(stderr, "function %s appears in both %s and %s! Exit.",
                            F->name_as_c_str(), _functions[F->name()]->input_file().c_str(), M->input_file().c_str());
                    exit(1);
                }
                else {
                    _functions[F->name()] = M;
                }
            }
        }

        for (auto F: target->function_list()) {
            F->dump();
        }

        //printf("number of modules: %lu\n", SysDict::module_table().size());
    }
};

REGISTER_PASS(LinkTracePass)