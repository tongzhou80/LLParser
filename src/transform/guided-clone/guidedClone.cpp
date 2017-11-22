//
// Created by tzhou on 10/28/17.
//

#include <cstdio>
#include <ir/irEssential.h>
#include <passes/pass.h>
#include <asmParser/sysDict.h>
#include <asmParser/llParser.h>
#include <di/diSubprogram.h>
#include <utilities/strings.h>
#include <transform/ben-alloc/benAlloc.cpp>

class GuidedClonePass: public Pass {
    std::ofstream _ofs;
    BenAllocPass* _lsda;

    /* command line args */
    string _log_dir;
    string _src_dir;
    string _lang;
    bool _use_indi;
    bool _noclone;  // only does ben replacement
    bool _noben;  // only does clone and no replacement


    /* flags */
    bool _load_verbose;
    bool _replace_verbose;

    /* statistics */
    int _clone_num;
public:
    GuidedClonePass() {
        set_is_global_pass();
        _log_dir = "./";
        _src_dir = "./";
        _lang = "all";
        _use_indi = false;
        _noclone = false;
        _noben = false;
        _load_verbose = false;
        _replace_verbose = false;
        _clone_num = 0;
    }

    void init() {
        /* set source file dir */
        if (has_argument("log_dir")) {
            _log_dir = get_argument("log_dir");
        }
        if (has_argument("src_dir")) {
            _src_dir = get_argument("src_dir");
        }
        if (has_argument("lang")) {
            _lang = get_argument("lang");
        }
        if (has_argument("indi")) {
            _use_indi = (bool)std::stoi(get_argument("indi"));
        }
        if (has_argument("noben")) {
            _noben = (bool)std::stoi(get_argument("noben"));
        }
        if (has_argument("noclone")) {
            _noclone = (bool)std::stoi(get_argument("noclone"));
        }
        _lsda = new BenAllocPass(_lang);
        _lsda->do_initialization();
        _lsda->set_use_indi(_use_indi);
    }

    Module* get_module(string& src_filename) {
        int pos = src_filename.rfind('.');
        guarantee(pos != string::npos, "");
        string ir_name = _src_dir+src_filename.substr(0, pos) + ".o.ll";
        Module* m = SysDict::get_module(ir_name);
        if (!m) {
            m = SysDict::parser->parse(ir_name);
            if (_load_verbose) {
                printf("parsed %s\n", m->name_as_c_str());
            }
            //guarantee(m, "");

//            if (m) {
//                _lsda->run_on_module(m);
//            }
        }

        return m;
    }

    void do_clone() {
        std::ifstream ifs(_log_dir+"/clone.log");
        string line;
        while (std::getline(ifs, line)) {
            auto fields = Strings::split(line, ' ');
            string callee_file = fields.at(0);
            string callee = fields.at(1);  // name of the function that's to be cloned
            string user_file = fields.at(3);
            string user = fields.at(4);
            string use_loc = fields.at(5);
            Point2D<int> point(use_loc);

            Module* callee_m = get_module(callee_file);
            if (!callee_m) {
                continue;
            }
            Function* callee_f = callee_m->get_function_by_orig_name(callee);
            Function* callee_clone = callee_f->clone();
            _clone_num++;
            callee_m->append_new_function(callee_clone);
            Module* user_m = get_module(user_file);
            if (!user_m) {
                continue;
            }
            //Function* user_f = user_m->get_function(user);
            Function* user_f = user_m->get_function_by_orig_name(user);
            guarantee(user_f, "Function %s not found", user.c_str());
            auto user_i = dynamic_cast<CallInstFamily*>(user_f->get_instruction(point));
            guarantee(user_i, "");

            /* need to insert declaration if inter-procedural */
            if (user_m != callee_m) {
                _lsda->insert_declaration(user_m, user_i->called_function()->name(), callee_clone->name());
            }
            user_i->replace_callee(callee_clone->name());
            if (_replace_verbose) {
                printf("replaced %s\n", callee_clone->name_as_c_str());
            }
        }
    }

    bool run_on_global() override {
        init();

        if (!_noclone) {
            do_clone();
        }

        if (!_noben) {
            use_ben_malloc();
        }

        for (auto it: SysDict::module_table()) {
            Module* m = it.second;
            //m->print_to_file(Strings::replace(m->input_file(), ".ll", ".clone.ll"));
            m->print_to_file(m->input_file());
        }
    }

    void use_ben_malloc() {
        std::ifstream ifs(_log_dir+"/ben.log");
        string line;
        while (std::getline(ifs, line)) {
            if (Module* m = get_module(line)) {
                _lsda->insert_lsd(m);
                _lsda->replace_alloc(m);
                _lsda->replace_free(m);
                if (_use_indi) {
                    _lsda->replace_indi(m);
                }
            }
        }
    }
};

REGISTER_PASS(GuidedClonePass)
