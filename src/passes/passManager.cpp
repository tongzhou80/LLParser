//
// Created by GentlyGuitar on 6/8/2017.
//


#include <dlfcn.h>
#include <unistd.h>

#include <algorithm>
#include <vector>
#include <peripheral/argsParser.h>
#include <peripheral/sysArgs.h>
#include <utilities/strings.h>
#include <utilities/flags.h>
#include "passManager.h"
#include "pass.h"
#include "../ir/instruction.h"


// PassList

PassList::PassList() {

}

PassList::~PassList() {

}

void PassList::add_pass(Pass *p) {
    if (p->is_module_pass()) {
        insert_with_priority(_module_passes, p);
        //_module_passes.push_back(p);
    }

    if (p->is_function_pass()) {
        insert_with_priority(_function_passes, p);
        //_function_passes.push_back(p);
    }

    if (p->is_basic_block_pass()) {
        insert_with_priority(_basic_block_passes, p);
        //_basic_block_passes.push_back(p);
    }

    if (p->is_instruction_pass()) {
        insert_with_priority(_instruction_passes, p);
        //_instruction_passes.push_back(p);
    }
}

int PassList::insert_with_priority(std::vector<Pass *>& list, Pass *p) {
    int i;
    for (i = 0; i < list.size(); ++i) {
        if (list[i]->priority() < p->priority()) {
            break;
        }
    }
    list.insert(list.begin()+i, p);
}

void PassList::apply_passes(Function *func) {
    std::vector<Pass*>& passes = _function_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_function(func);
    }
}

void PassList::apply_passes(Instruction *inst) {
    std::vector<Pass*>& passes = _instruction_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_instruction(inst);
    }
}

void PassList::apply_passes(Module *module) {
    std::vector<Pass*>& passes = _module_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_module(module);
    }
}

void PassList::apply_passes(BasicBlock *unit) {
    std::vector<Pass*>& passes = _basic_block_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_basic_block(unit);
    }
}

void PassList::apply_epilogue() {
    std::vector<Pass*>& passes = _instruction_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->parse_epilogue();
    }
}
//
//template <typename T>
//void PassList::apply_passes(T *unit) {
//    if (std::is_same<T, Function*>::value) {
//        std::vector<Pass*>& passes = _function_passes;
//        for (int i = 0; i < passes.size(); ++i) {
//            int mutated = passes[i]->run_on_module(unit);
//        }
//    }
//
//    if (std::is_same<T, Instruction*>::value) {
//        std::vector<Pass*>& passes = _instruction_passes;
//        for (int i = 0; i < passes.size(); ++i) {
//            int mutated = passes[i]->run_on_instruction(unit);
//        }
//    }
//}
//
//// Explicit template instantiation to hide the template definition in .cpp
//template void PassList::apply_passes<Function>(Function*);
//template void PassList::apply_passes<Instruction>(Instruction*);




// PassManager

PassManager* PassManager::pass_manager = NULL;

PassManager::PassManager() {
    _pass_lib_path = "../passes";
}

PassManager::~PassManager() {
    std::vector<std::vector<Pass*> > all_passes;
    all_passes.push_back(_global_passes);
    all_passes.push_back(_module_passes);
    all_passes.push_back(_function_passes);
    all_passes.push_back(_basic_block_passes);

    for (auto l: all_passes) {
        for (auto p: l) {
            if (p->is_dynamic()) {
                p->unload();
            }
            else {
                delete p;
            }
        }
    }
}

void PassManager::init() {
    pass_manager = new PassManager();
    pass_manager->initialize_passes();
}

void PassManager::destroy() {
    if (pass_manager != NULL)
        delete pass_manager;
}

/* for debug, addr2line can't show line numbers in a .so */
#include <transform/new-clone/newClone.cpp>
void PassManager::initialize_passes() {
//    HelloFunction* hello = new HelloFunction();
//    add_parse_time_pass(hello);
    //add_pass("ATrace");

    if (DebugRun) {
        NewClonePass* p = new NewClonePass();
        p->set_name("NewClonePass");

        //p->arguments()["hot_aps_file"] = "/home/tzhou/ClionProjects/LLParser/src/transform/new-clone/test/test.txt";

        //p->set_argument("hot_aps_file", "/home/tzhou/ClionProjects/LLParser/src/transform/new-clone/test/bzip2_hot_a2l.txt");
        p->set_argument("hot_aps_file", "/home/tzhou/ClionProjects/LLParser/src/transform/new-clone/test/test.txt");
        add_pass(p);
    }

    for (auto p: SysArgs::passes()) {
        add_pass(p);
    }
}

int PassManager::insert_with_priority(std::vector<Pass *>& list, Pass *p) {
    int i;
    for (i = 0; i < list.size(); ++i) {
        if (list[i]->priority() < p->priority()) {
            break;
        }
    }
    list.insert(list.begin()+i, p);
}

void PassManager::add_pass(Pass *p) {
    if (p->is_global_pass()) {
        insert_with_priority(_global_passes, p);
    }
    if (p->is_module_pass()) {
        insert_with_priority(_module_passes, p);
        //_module_passes.push_back(p);
    }

    if (p->is_function_pass()) {
        insert_with_priority(_function_passes, p);
        //_function_passes.push_back(p);
    }

    if (p->is_basic_block_pass()) {
        insert_with_priority(_basic_block_passes, p);
        //_basic_block_passes.push_back(p);
    }

    if (p->is_instruction_pass()) {
        insert_with_priority(_instruction_passes, p);
        //_instruction_passes.push_back(p);
    }
}

/// The loaded passes will be deleted in PassManager's destructor
void PassManager::add_pass(string name) {

    string ld_path = SysArgs::get_property("ld-pass-path");
    if (ld_path != "") {
        _pass_lib_path = ld_path;
    }
    zp1(name.c_str())
    char path[1024], loader[1024], unloader[1024];
    zp1(name.c_str())
    string pass_name = name;
    string args;
    int arg_pos = name.find('?');
    if (arg_pos != name.npos) {
        pass_name = name.substr(0, arg_pos);
        args = name.substr(arg_pos+1);
    }

    /* if pass is an .so file, use its path */
    if (Strings::conatins(pass_name, ".so")) {
        //char cwd[1024];
        //getcwd(cwd, 1024);
        //sprintf(path, "%s/%s", cwd, pass_name.c_str());
        sprintf(path, "%s", pass_name.c_str());
        size_t p1 = pass_name.rfind("lib");
        p1 += 3;
        size_t p2 = pass_name.rfind(".so");
        guarantee(p1 != pass_name.npos && p2 != pass_name.npos, "Not a valid shared library object ");
        string classname = pass_name.substr(p1, p2-p1);
        sprintf(loader, "__load_pass_%sPass", classname.c_str());
        sprintf(unloader, "__unload_pass_%sPass", classname.c_str());
    }
    else { /* otherwise use ld-pass-path */
        /* rename ab-cd to AbCd */
        if (!isupper(pass_name[0])) {
            pass_name[0] = toupper(pass_name[0]);
        }

        while (pass_name.find('-') != pass_name.npos) {
            int p = pass_name.find('-');
            if (!isupper(pass_name[p+1])) {
                pass_name[p+1] = toupper(pass_name[p+1]);
            }
            pass_name[p] = ' ';
        }
        pass_name.erase(std::remove(pass_name.begin(), pass_name.end(), ' '), pass_name.end());

        sprintf(path, "%s/lib%s.so", _pass_lib_path.c_str(), pass_name.c_str());
        sprintf(loader, "__load_pass_%sPass", pass_name.c_str());
        sprintf(unloader, "__unload_pass_%sPass", pass_name.c_str());
    }


    zpl("load pass from %s", path);
    void *passso = dlopen(path, RTLD_NOW);
    //zpl("passso %p", passso);
    if (passso) {
        pass_loader ldp = (pass_loader)dlsym(passso, loader);
        pass_unloader unldp = (pass_unloader)dlsym(passso, unloader);
        if (ldp == NULL || unldp == NULL) {
            throw PassNotRegisteredError("pass " + pass_name + " does not have a loader or an unloader.\n"
                                                                       "Please add 'REGISTER_PASS(yourclassname)' at the end of your pass source file");
        }

        Pass* pass_obj = ldp();
        pass_obj->set_unloader(unldp);
        pass_obj->set_is_dynamic();
        if (!args.empty()) {
            pass_obj->parse_arguments(args);
        }
        add_pass(pass_obj);
    } else {
        throw PassOpenFailedError("dlopen failed: " + string(dlerror()));
    }

}


// apply passes

void PassManager::apply_global_passes() {
    std::vector<Pass*>& passes = _global_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_global();
    }
}

void PassManager::apply_passes(Module *module) {
    std::vector<Pass*>& passes = _module_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_module(module);
    }
}

void PassManager::apply_passes(Function *func) {
    std::vector<Pass*>& passes = _function_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_function(func);
    }
}

void PassManager::apply_passes(Instruction *inst) {
    std::vector<Pass*>& passes = _instruction_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_instruction(inst);
    }
}

void PassManager::apply_passes(BasicBlock *bb) {
    std::vector<Pass*>& passes = _basic_block_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->run_on_basic_block(bb);
    }
}

// initialization and finalization

void PassManager::apply_initializations(Module *module) {
    std::vector<Pass*>& passes = _function_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->do_initialization(module);
    }

    std::vector<Pass*>& passes1 = _basic_block_passes;
    for (int i = 0; i < passes1.size(); ++i) {
        int mutated = passes1[i]->do_initialization(module);
    }
}

void PassManager::apply_finalization(Module *module) {
    std::vector<Pass*>& passes = _function_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->do_finalization(module);
    }

    std::vector<Pass*>& passes2 = _basic_block_passes;
    for (int i = 0; i < passes2.size(); ++i) {
        int mutated = passes2[i]->do_finalization(module);
    }
}
