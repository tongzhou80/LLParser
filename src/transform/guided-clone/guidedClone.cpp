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
#include <transform/lsda/lsda.cpp>

class GuidedClonePass: public Pass {
    std::ofstream _ofs;
    string _log_dir;
    string _src_dir;
    bool _use_indi;
    LSDAPass* _lsda;

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
        _use_indi = true;
        _lsda = new LSDAPass();

        _clone_num = 0;
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

            guarantee(m, "");
        }
        _lsda->run_on_module(m);
        return m;
    }

    bool run_on_global() override {
        /* set source file dir */
        if (has_argument("log_dir")) {
            _log_dir = get_argument("log_dir");
        }
        if (has_argument("src_dir")) {
            _src_dir = get_argument("src_dir");
        }
        if (has_argument("use_indi")) {
            _use_indi = (bool)std::stoi(get_argument("use_indi"));
        }

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
            Function* callee_f = callee_m->get_function(callee);
            Function* callee_clone = callee_f->clone();
            _clone_num++;
            callee_m->append_new_function(callee_clone);
            Module* user_m = get_module(user_file);
            Function* user_f = user_m->get_function(user);
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

        for (auto it: SysDict::module_table()) {
            Module* m = it.second;
            _lsda->replace_alloc(m);
            _lsda->replace_free(m);
            if (_use_indi) {
                _lsda->replace_indi(m);
            }
            m->print_to_file(Strings::replace(m->input_file(), ".ll", ".clone.ll"));
        }
    }
};

REGISTER_PASS(GuidedClonePass)