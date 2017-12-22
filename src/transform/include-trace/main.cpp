//
// Created by tzhou on 12/6/17.
//

#include <passes/pass.h>
#include <ir/irEssential.h>
#include <di/diEssential.h>
#include <utilities/strings.h>

class IncludeTracePass: public Pass {
    std::ofstream _ofs;
public:
    IncludeTracePass() {
        set_is_module_pass();
    }

    void do_initialization() override {
        _ofs.open("inclusion.txt");
    }

    void do_finalization() override {
        _ofs.close();
    }

    bool run_on_module(Module* module) override {
        string parent = "";
        for (auto md: module->unnamed_metadata_list()) {
            if (DIFile* difile = dynamic_cast<DIFile*>(md)) {
                if (parent.empty()) {
                    parent = difile->filename();
                    if (Strings::endswith(parent, ".c")) {
                        return false;
                    }
                }
                else {
                    _ofs << difile->filename() << " " << parent << std::endl;
                }
            }
        }
        return false;
    }
};

REGISTER_PASS(IncludeTracePass);
