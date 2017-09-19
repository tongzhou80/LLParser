//
// Created by tzhou on 9/19/17.
//

#include <passes/pass.h>
#include <ir/irEssential.h>
#include <asmParser/sysDict.h>
#include <thread>

class SplitModulePass: public Pass {
    string _output_dir;
public:
    SplitModulePass() {
        set_is_module_pass();
    }

    bool run_on_module(Module* module) {
        auto ncores = std::thread::hardware_concurrency();
        if (ncores < 4) {
            fprintf(stderr, "The number of cores should at least be 4, but got %d", ncores);
            exit(1);
        }

        if (has_argument("dir")) {
            _output_dir = get_argument("dir");
        }

        print_other_data(module);
        print_functions(module);
        print_debug_info(module);
    }

    void print_other_data(Module* m) {
        std::ofstream ofs(_output_dir+"/head");

        ofs << "; ModuleID = '" << m->module_id() << "'\n";
        for (auto pair: m->headers()) {
            ofs << pair.first << " = \"" << pair.second << "\"\n";
        }
        ofs << '\n';

        if (!m->module_level_inline_asms().empty()) {
            for (auto s: m->module_level_inline_asms()) {
                ofs << s << '\n';
            }
            ofs << '\n';
        }

        for (auto i: m->struct_list()) { ofs << i; }           ofs << '\n';
        for (auto i: m->comdat_list()) { ofs << i; }           ofs << '\n';
        for (auto i: m->global_list()) { ofs << i; }           ofs << '\n';
        for (auto i: m->alias_map()) { ofs << i.second; }      ofs << '\n';

        for (auto i: m->attribute_list()) { ofs << i; }        ofs << '\n';
        for (auto i: m->named_metadata_map()) { ofs << i.second; }        ofs << '\n';

        ofs.close();
    }

    void print_functions(Module* m) {
        std::ofstream ofs;
        auto ncores = std::thread::hardware_concurrency();
        int nfiles = (ncores-1) / 3 * 2;
        auto nfuncs = m->function_list().size();
        std::size_t nfunc_per_file;
        if (nfuncs % nfiles == 0) {
            nfunc_per_file = nfuncs / nfiles;
        }
        else {
            nfunc_per_file = nfuncs / nfiles + 1;
        }

        for (int i = 0; i < nfuncs; ++i) {
            if (i % nfunc_per_file == 0) {
                if (ofs.is_open()) {
                    ofs.close();
                }
                ofs.open(_output_dir+"/func"+std::to_string(i/nfunc_per_file));

                ofs << "; ModuleID = '" << m->module_id() << "'\n";
                for (auto pair: m->headers()) {
                    ofs << pair.first << " = \"" << pair.second << "\"\n";
                }
                ofs << '\n';

            }
            ofs << m->function_list()[i];
        }
    }

    void print_debug_info(Module* m) {
        std::ofstream ofs;
        auto ncores = std::thread::hardware_concurrency();
        int nfiles = (ncores-1) / 3 * 1;
        auto nfuncs = m->unnamed_metadata_list().size();
        std::size_t nfunc_per_file;
        if (nfuncs % nfiles == 0) {
            nfunc_per_file = nfuncs / nfiles;
        }
        else {
            nfunc_per_file = nfuncs / nfiles + 1;
        }

        for (int i = 0; i < nfuncs; ++i) {
            if (i % nfunc_per_file == 0) {
                if (ofs.is_open()) {
                    ofs.close();
                }
                ofs.open(_output_dir+"/debug"+std::to_string(i/nfunc_per_file));

                ofs << "; ModuleID = '" << m->module_id() << "'\n";
                for (auto pair: m->headers()) {
                    ofs << pair.first << " = \"" << pair.second << "\"\n";
                }
                ofs << '\n';
            }
            ofs << m->unnamed_metadata_list()[i];
        }
    }
};

REGISTER_PASS(SplitModulePass);

