//
// Created by GentlyGuitar on 6/8/2017.
//

#ifdef __linux__
#include <dlfcn.h>
#include <unistd.h>


#endif
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
    /* need to call __unload_pass, can't delete */
//    for (int i = 0; i < _module_passes.size(); ++i) {
//        delete _module_passes[i];
//    }
//
//    for (int i = 0; i < _function_passes.size(); ++i) {
//        delete _function_passes[i];
//    }
//
//    for (int i = 0; i < _basic_block_passes.size(); ++i) {
//        delete _basic_block_passes[i];
//    }
}

void PassManager::init() {
    pass_manager = new PassManager();
    pass_manager->initialize_passes();
}

void PassManager::destroy() {
    if (pass_manager != NULL)
        delete pass_manager;
}

void PassManager::initialize_passes() {
//    HelloFunction* hello = new HelloFunction();
//    add_parse_time_pass(hello);
    //add_pass("ATrace");
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
    if (!isupper(name[0])) {
        name[0] = toupper(name[0]);
    }

    string ld_path = SysArgs::get_property("ld-pass-path");
    if (ld_path != "") {
        _pass_lib_path = ld_path;
    }
    char path[1024], loader[1024];

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
    }
    else { /* otherwise use ld-pass-path */
        sprintf(path, "%s/lib%s.so", _pass_lib_path.c_str(), pass_name.c_str());
        sprintf(loader, "__load_pass_%sPass", pass_name.c_str());
    }


    zpl("load pass from %s", path);
    void *passso = dlopen(path, RTLD_NOW);
    //zpl("passso %p", passso);
    if (passso) {
        pluggable_pass_loader ldp = (pluggable_pass_loader)dlsym(passso, loader);
        if (ldp == NULL) {
            throw PassNotRegisteredError("pass " + pass_name + " does not have a loader and an unloader.\n"
                                                                       "Please add 'REGISTER_PASS(yourclassname)' at the end of your pass source file");
        }
        Pass* pass_obj = ldp();
        if (!args.empty()) {
            pass_obj->parse_arguments(args);
        }
        add_pass(pass_obj);
    } else {
        throw PassOpenFailedError("dlopen failed: " + string(dlerror()));
    }

}

//Pass* PassManager::get_pass(std::vector<Pass*>& passes, int i) {
//    if (ParallelModule) {
//
//    }
//    else {
//        return NULL;
//    }
//}

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

void PassManager::apply_initializations() {
    std::vector<Pass*>& passes = _module_passes;
    for (int i = 0; i < passes.size(); ++i) {
        int mutated = passes[i]->do_initialization();
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