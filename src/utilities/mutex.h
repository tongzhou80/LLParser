//
// Created by tzhou on 8/28/17.
//

#ifndef LLPARSER_MUTEX_H
#define LLPARSER_MUTEX_H

#include <pthread.h>
#include <semaphore.h>
#include "macros.h"


class Monitor {
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;
public:
    Monitor()  { int ret = pthread_mutex_init(&_mutex, NULL); guarantee(ret == 0, "Mutex initialize failed"); }
    ~Monitor()  { pthread_mutex_destroy(&_mutex); }
    pthread_mutex_t* mutex()  { return &_mutex; }
    pthread_cond_t* cond()  { return &_cond; }
    void lock()  { pthread_mutex_lock(&_mutex); }
    void unlock()  { pthread_mutex_unlock(&_mutex); }
    void wait()  { pthread_cond_wait(&_cond, &_mutex); }
    void signal()  { pthread_cond_signal(&_cond); }
    void signal_all()  { pthread_cond_broadcast(&_cond); }
};

class Mutex {
    sem_t _sem;
public:
    Mutex() {
        sem_init(&_sem, 0, 1);
    }
    ~Mutex() {
        ~sem_destroy(&_sem);
    }
    void lock() { sem_wait(&_sem); }
    void unlock() { sem_post(&_sem); }
};


class Locks {
public:
    static void init();
    static void destroy();

    static Mutex* module_list_lock;
    static Mutex* thread_table_lock;
    static Mutex* pass_manager_lock;
};

#endif //LLPARSER_MUTEX_H
