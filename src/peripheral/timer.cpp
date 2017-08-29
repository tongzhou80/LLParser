//
// Created by tlaber on 6/16/17.
//

#include <cstdio>
#include <utilities/macros.h>
#include "timer.h"
#include "unistd.h"

Timer::Timer() {
    _elapsed = 0;
    _has_started = false;
}

void Timer::start() {
    reset();
    _has_started = true;
    gettimeofday(&_start, NULL);
}

void Timer::stop() {
    guarantee(_has_started, "can call stop() without calling start() first");
    gettimeofday(&_end, NULL);
    timersub(&_end, &_start, &_delta);
    _has_started = false;
    calc_time();
}

void Timer::reset() {
    _elapsed = 0;
}

void Timer::resume() {
    _has_started = true;
    gettimeofday(&_start, NULL);
}

void Timer::calc_time() {
    guarantee(!_has_started, "timer is still running!");
    char elapsed_in_str[2048];
    sprintf(elapsed_in_str, "%lld.%06ld", (long long int)_delta.tv_sec, (long int)_delta.tv_usec);
    _elapsed += std::stof(elapsed_in_str);
}

double Timer::seconds() {
    return _elapsed;
}

void Timer::_test() {
    Timer t;
    t.start();
    usleep(3000000);
    t.stop();
    zpl("seconds: %.3f", t.seconds());
}