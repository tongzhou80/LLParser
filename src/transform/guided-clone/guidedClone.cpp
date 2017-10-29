//
// Created by tzhou on 10/28/17.
//

#include <cstdio>
#include <ir/function.h>
#include <passes/pass.h>
#include <asmParser/sysDict.h>
#include <di/diSubprogram.h>

class GuidedClonePass: public Pass {
    std::ofstream _ofs;
    string _dir;
public:
    GuidedClonePass() {
        set_is_global_pass();
        _dir = ".";
    }

    bool run_on_global() override {
        /* set source file dir */
        if (has_argument("dir")) {
            _dir = get_argument("dir");
        }


//        _ofs.open("function_to_file.txt");
//        for (MetaData* md: module->unnamed_metadata_list()) {
//            if (DISubprogram* sp = dynamic_cast<DISubprogram*>(md)) {
//                _ofs << sp->name() << ": " << sp->filename() << '\n';
//            }
//        }
//        _ofs.close();
    }
};

REGISTER_PASS(GuidedClonePass)