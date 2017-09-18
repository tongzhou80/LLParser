//
// Created by tlaber on 6/16/17.
//

#ifndef LLPARSER_TIMER_H
#define LLPARSER_TIMER_H

#include <string>
#include <sys/time.h>

class Timer {
    double _elapsed;
    timeval _start;
    timeval _end;
    timeval _delta;
    bool _has_started;

    void calc_time();
public:
    Timer();

    void start();
    void stop();
    void resume();
    void reset();

    double seconds();

    /* for debug */
    static void _test();
};

#endif //LLPARSER_TIMER_H
