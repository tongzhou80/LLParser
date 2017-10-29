//
// Created by tzhou on 10/28/17.
//

#include <cstdio>
#include <ir/function.h>
#include <passes/pass.h>
#include <asmParser/sysDict.h>
#include <di/diSubprogram.h>
#include <utilities/strings.h>

class GuidedClonePass: public Pass {
    std::ofstream _ofs;
    string _log_dir;
    string _src_dir;
public:
    GuidedClonePass() {
        set_is_global_pass();
        _log_dir = ".";
        _src_dir = ".";
    }

    Module* get_module(string& src_filename) {
        
    }

    bool run_on_global() override {
        /* set source file dir */
        if (has_argument("log_dir")) {
            _log_dir = get_argument("log_dir");
        }
        if (has_argument("src_dir")) {
            _src_dir = get_argument("src_dir");
        }


        std::ifstream ifs(_log_dir+"/clone.log");
        string line;
        while (std::getline(ifs, line)) {
            auto fields = Strings::split(line, ' ');
            string callee = fields[0];  // name of the function that's to be cloned
            string user = fields[2];
            string callee_file = Strings::split(callee, ':').at(0);

            Module* callee_m = get_module(callee_file);

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