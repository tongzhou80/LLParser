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

template <typename T>
class Point2D {
public:
    T x;
    T y;
    Point2D(T xx, T yy): x(xx), y(yy) {}
    const char* c_str() {
        std::string ret = std::to_string(x) + ", " + std::to_string(y);
        return ret.c_str();
    }
};


#endif //LLPARSER_TIMER_H
