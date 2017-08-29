//
// Created by tlaber on 6/26/17.
//

#include <unistd.h>
#include <sys/wait.h>
#include <set>

#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>
#include <ir/di/diEssential.h>
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>


class CallGraphPass: public Pass {
    bool PrintCloning;
    bool TracingVerbose;
    std::ofstream _ofs;

    std::vector<string> _bottoms;
    std::set<string> _visited;
public:
    CallGraphPass() {
        set_is_module_pass();

        PrintCloning = true;
        TracingVerbose = true;
    }

    ~CallGraphPass() {
        printf("pass unloading is not yet implemented! Do stuff in do_finalization!\n");
    }

    bool run_on_module(Module* module);

    void traverse(Function* f, Module* module);

    void print_dot_line(string caller, string callee);

    //bool do_finalization(Module* module);
};

bool CallGraphPass::run_on_module(Module *module) {
    if (has_argument("bottom")) {
        auto bottoms = Strings::split(get_argument("bottom"), ',');
        _bottoms.insert(_bottoms.end(), bottoms.begin(), bottoms.end());
    }

    string out = SysArgs::cur_target;
    if (out == "") {
        out = "callGraph";
    }
    out += ".dot";

    if (SysArgs::has_property("output")) {
        out = SysArgs::get_property("output");
    }

    _ofs.open(out);
    _ofs << "digraph {\n";

    if (_bottoms.empty()) {
        auto& functions = module->function_list();
        for (auto fi = functions.begin(); fi != functions.end(); ++fi) {
            Function* F = *fi;
            for (auto ui = F->user_begin(); ui != F->user_end(); ++ui) {
                Instruction* I = *ui;
                print_dot_line(I->function()->name(), F->name());
                //_ofs << '"' << I->function()->name() << "\" -> \"" << F->name() << "\";\n";
            }
        }
    }
    else {
        for (auto it = _bottoms.begin(); it != _bottoms.end(); ++it) {
            string name = *it;
            traverse(module->get_function(name), module);
        }
    }

    _ofs << "}\n";
    _ofs.close();

    // call dot as well
    pid_t pid = fork();
    if (pid == -1) {
        fprintf(stderr, "fork process (dot) failed\n");
    }
    else if (pid == 0) {
        execlp("dot", "dot", "-Tpng", "-O", out.c_str(), NULL);
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

void CallGraphPass::traverse(Function *F, Module* module) {
    guarantee(F, "function %s not found", F->name_as_c_str());

    _visited.insert(F->name());
    for (auto ui = F->user_begin(); ui != F->user_end(); ++ui) {
        Instruction* I = *ui;
        string caller_name = I->function()->name();
        print_dot_line(caller_name, F->name());
        //_ofs << '"' << caller_name << "\" -> \"" << F->name() << "\";\n";

        if (_visited.find(caller_name) == _visited.end()) {
            traverse(module->get_function(caller_name), module);
        }
    }

}

void CallGraphPass::print_dot_line(string caller, string callee) {
    _ofs << '"' << caller << "\" -> \"" << callee << "\";\n";
}

REGISTER_PASS(CallGraphPass);