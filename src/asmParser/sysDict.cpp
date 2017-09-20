//
// Created by tlaber on 6/9/17.
//

#include <utilities/mutex.h>
#include <utilities/flags.h>
#include <asmParser/llParserTLS.h>
#include <inst/instFlags.h>
#include "sysDict.h"
#include "instParser.h"
#include "llParser.h"

std::map<pthread_t , Module*> SysDict::_thread_module_table;  // use pthread id to index
std::map<string, Module*> SysDict::_module_table;  // use input file name to index
LLParser* SysDict::parser = NULL;
std::vector<Instruction*> SysDict::_inst_stack;
//InstParser* SysDict::instParser = NULL;

void SysDict::init() {
    parser = new LLParser();
    //instParser = new InstParser();

    InstFlags::init();
    Locks::init();
}

void SysDict::destroy() {
    delete parser;

//    if (instParser) {
//        delete instParser;
//    }

    if (ParallelModule) {
        for (auto it: thread_module_table()) {
            delete it.second;
        }
    }
    
    Locks::destroy();
}

void SysDict::worker_push_inst(Instruction *inst) {
    Locks::inst_stack_lock->lock();
    _inst_stack.push_back(inst);
    //zpl("push: %p", inst)
    Locks::inst_stack_lock->unlock();
}

Instruction* SysDict::worker_fetch_instruction() {
    Locks::inst_stack_lock->lock();
    Instruction* ret = NULL;
    if (!_inst_stack.empty()) {
        ret = _inst_stack.at(_inst_stack.size()-1);
        _inst_stack.pop_back();
    }

    Locks::inst_stack_lock->unlock();
    return ret;
}

bool SysDict::inst_stack_is_empty() {
    Locks::inst_stack_lock->lock();
    bool ret = _inst_stack.empty();
    Locks::inst_stack_lock->unlock();
    return ret;
}

const string& SysDict::filename() {
    return module()->input_file();
}

const string SysDict::filedir() {
    auto last_slash = filename().rfind('/');
    if (last_slash != string::npos) {
        return filename().substr(0, last_slash+1);
    }
    else {
        return "./";
    }
}

/**@brief Register a module.
 *
 * ThreadLocal data is also initilized here.
 * Hopefully the lock/unlock won't have much overhead in case of single thread.
 *
 * @param module
 */
void SysDict::add_module(Module* m) {
    Locks::module_list_lock->lock();
    module_table()[m->input_file()] = m;
    Locks::module_list_lock->unlock();

    Locks::thread_table_lock->lock();
    thread_module_table()[pthread_self()] = m;
    Locks::thread_table_lock->unlock();
}

/**@brief Returns the current module.
 * This method uses pthread_self() to do the lookup even if there is just one thread (with SysDict::parser).
 *
 * @return
 */
Module* SysDict::module() {
    pthread_t id = pthread_self();
    Module* m;

    Locks::thread_table_lock->lock();
    guarantee(thread_module_table().find(id) != thread_module_table().end(), " ");
    m = thread_module_table()[id];
    Locks::thread_table_lock->unlock();
    return m;
}
//
//Module* SysDict::get_module(string name) {
//    if (modules.find(name) == modules.end()) {
//        return NULL;
//    }
//    else {
//        return modules[name];
//    }
//}

/**@brief Merge SysDict::modules into one module which is stored in SysDict::module()
 *
 */
void SysDict::merge_modules() {

}