//
// Created by tzhou on 12/6/17.
//

#include <passes/pass.h>
#include <ir/irEssential.h>
#include <di/diEssential.h>

class IncludeTracePass: public Pass {
    std::ofstream _ofs;
public:
    IncludeTracePass() {
        set_is_module_pass();
    }

    bool do_initialization() override {
        _ofs.open("inclusion.txt");
    }

    bool do_finalization() override {
        _ofs.close();
    }

    bool run_on_module(Module* module) override {
        string parent = "";
        for (auto md: module->unnamed_metadata_list()) {
            if (DIFile* difile = dynamic_cast<DIFile*>(md)) {
                zps(difile->filename())
            }
        }
    }
};

REGISTER_PASS(IncludeTracePass);
