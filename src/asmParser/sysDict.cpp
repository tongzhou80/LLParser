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

std::vector<Module*> SysDict::modules;
std::map<pthread_t , LLParser*> SysDict::thread_table;  // only used when ParallelModule is on
LLParser* SysDict::llParser = NULL;
InstParser* SysDict::instParser = NULL;

void SysDict::init() {
    llParser = new LLParser();
    instParser = new InstParser();

    InstFlags::init();
    Locks::init();
}

void SysDict::destroy() {
    if (llParser) {
        delete llParser;
    }
    if (instParser) {
        delete instParser;
    }

    if (ParallelModule) {
        for (auto it: thread_table) {
            delete it.second;
        }
    }
    
    Locks::destroy();
}

Module* SysDict::module() {
    guarantee(modules.size() > 0, " ");
    get_thread_llparser()->module();
}

const string& SysDict::filename() {
    LLParser* lp = (ParallelModule) ? get_thread_llparser(): SysDict::llParser;
    return lp->filename();
}

/**
 *
 * ThreadLocal data is also initilized here
 *
 * @param module
 */
void SysDict::add_module(LLParser* parser) {
    Locks::module_list_lock->lock();
    modules.push_back(parser->module());
    Locks::module_list_lock->unlock();

    if (ParallelModule) {
        Locks::thread_table_lock->lock();
        thread_table[pthread_self()] = parser;
        Locks::thread_table_lock->unlock();
    }

}

LLParser* SysDict::get_thread_llparser() {
    if (ParallelModule) {
        pthread_t id = pthread_self();
        LLParser* lp;

        Locks::thread_table_lock->lock();
        guarantee(thread_table.find(id) != thread_table.end(), " ");
        lp = thread_table[id];
        Locks::thread_table_lock->unlock();
        return lp;
    }
    else {
        return SysDict::llParser;
    }
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