//
// Created by tlaber on 6/26/17.
//

#include <unistd.h>
#include <sys/wait.h>
#include <set>
#include <iomanip>

#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>
#include <di/diEssential.h>
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <asmParser/sysDict.h>


class CallgraphPass: public Pass {
    bool PrintCaller;
    std::ofstream _dot_ofs;
    std::ofstream _caller_ofs;
    std::vector<string> _bottoms;
    std::set<string> _visited;
public:
    CallgraphPass() {
        set_is_module_pass();

        PrintCaller = true;
    }

    bool run_on_module(Module* module);

    void traverse(Function* f, Module* module);

    void print_dot_line(string caller, string callee);

    //void do_finalization(Module* module);
};

bool CallgraphPass::run_on_module(Module *module) {
    if (has_argument("bottom")) {
        auto bottoms = Strings::split(get_argument("bottom"), ',');
        _bottoms.insert(_bottoms.end(), bottoms.begin(), bottoms.end());
    }

    string out1 = SysDict::filename() + ".dot";
    string out2 = SysDict::filename() + ".callers";

    _dot_ofs.open(out1);
    _caller_ofs.open(out2);
    _dot_ofs << "digraph {\n";

    if (_bottoms.empty()) {
        for (auto F: module->function_list()) {
            if (F->is_clone()) {
                continue;
            }

            if (F->caller_list().empty()) {
                continue;
            }

            _caller_ofs << std::left << std::setw(50) << F->name() << F->caller_list().size() << std::endl;
            for (auto I: F->caller_list()) {
                if (CallInstFamily* cif = dynamic_cast<CallInstFamily*>(I)) {
                    print_dot_line(I->function()->name(), F->name());
                    _caller_ofs << "  - " << I->function()->name() << std::endl;
                }
            }
        }
    }
    else {
        for (auto it = _bottoms.begin(); it != _bottoms.end(); ++it) {
            string name = *it;
            traverse(module->get_function(name), module);
        }
    }

    _dot_ofs << "}\n";
    _dot_ofs.close();
    _caller_ofs.close();

    if (has_argument("dot")) {
        // call dot as well
        pid_t pid = fork();
        if (pid == -1) {
            fprintf(stderr, "fork process (dot) failed\n");
        }
        else if (pid == 0) {
            execlp("dot", "dot", "-Tpng", "-O", out1.c_str(), NULL);
        }
        else {
            wait(NULL);
//        The wait() system call suspends execution of the calling process
//        until one of its children terminates.  The call wait(&wstatus) is
//        equivalent to:
//
//        waitpid(-1, &wstatus, 0);
        }
    }
}

void CallgraphPass::traverse(Function *F, Module* module) {
    guarantee(F, "function %s not found", F->name_as_c_str());

    _visited.insert(F->name());
    for (auto I: F->caller_list()) {
        string caller_name = I->function()->name();
        print_dot_line(caller_name, F->name());
        //_dot_ofs << '"' << caller_name << "\" -> \"" << F->name() << "\";\n";

        if (_visited.find(caller_name) == _visited.end()) {
            traverse(module->get_function(caller_name), module);
        }
    }

}

void CallgraphPass::print_dot_line(string caller, string callee) {
    _dot_ofs << '"' << caller << "\" -> \"" << callee << "\";\n";
}

REGISTER_PASS(CallgraphPass);