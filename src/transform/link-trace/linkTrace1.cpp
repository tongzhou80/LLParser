//
// Created by tzhou on 10/28/17.
//

#include <cstdio>
#include <ir/function.h>
#include <passes/pass.h>
#include <asmParser/sysDict.h>
#include <di/diSubprogram.h>

class LinkTracePass: public Pass {
    std::ofstream _ofs;
public:
    LinkTracePass() {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) override {
        _ofs.open("function_to_file.txt");
        for (MetaData* md: module->unnamed_metadata_list()) {
            if (DISubprogram* sp = dynamic_cast<DISubprogram*>(md)) {
                _ofs << sp->name() << ": " << sp->filename() << '\n';
            }
        }
        _ofs.close();
    }
};

REGISTER_PASS(LinkTracePass)