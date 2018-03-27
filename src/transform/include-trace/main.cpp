//
// Created by tzhou on 12/6/17.
//

#include <passes/pass.h>
#include <ir/irEssential.h>
#include <di/diEssential.h>
#include <utilities/strings.h>

// This pass finds out what files are included by the
// current module, which is used for some fortran
// benchmarks to ensure that every individual source
// file will be compiled to an ir file.

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

    // FIXME: This assumes the first DIfile is the
    // current file, which I am not sure if is correct.
    bool run_on_module(Module* module) override {
        string name = module->name();
        if (Strings::endswith(name, ".c")
            || Strings::endswith(name, ".cc")
            || Strings::endswith(name, ".cpp")
            ) {
            return false;
        }
        string parent = "";
        for (auto md: module->unnamed_metadata_list()) {
            if (DIFile* difile = dynamic_cast<DIFile*>(md)) {
                if (parent.empty()) {
                    parent = difile->filename();
                    if (Strings::endswith(parent, ".c")) {
                        std::cout << "P: " << parent << "\n";
                        return false;
                    }
                }
                else {
                    _ofs << difile->filename()
                         << " " << parent
                         << std::endl;
                }
            }
        }
        return false;
    }
};

REGISTER_PASS(IncludeTracePass);
