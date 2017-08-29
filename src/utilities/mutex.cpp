//
// Created by tzhou on 8/28/17.
//

#include "mutex.h"

Mutex* Locks::module_list_lock = NULL;
Mutex* Locks::thread_table_lock = NULL;
Mutex* Locks::pass_manager_lock = NULL;

void Locks::init() {
    module_list_lock = new Mutex();
    thread_table_lock = new Mutex();
    pass_manager_lock = new Mutex();
}

void Locks::destroy() {
    if (module_list_lock) {
        delete module_list_lock;
    }

    if (thread_table_lock) {
        delete thread_table_lock;
    }

    if (pass_manager_lock) {
        delete pass_manager_lock;
    }
}