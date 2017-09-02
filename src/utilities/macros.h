//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_MACROS_H
#define LLPARSER_MACROS_H


#include <string>
#include <cstdio>
#include <iostream>
#include <vector>
#include "internalError.h"

#define guarantee(condition, args...) \
    do { \
        if (! (condition)) { \
            fflush(stdout); \
            fprintf(stderr, "Assertion `" #condition "` failed in %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, ##args); \
            fprintf(stderr, "\n"); \
            Errors::semantic_error_handler(); \
        } \
    } while (false)


typedef std::string string;

#define parser_assert(condition, line, args...) \
    do { \
        if (! (condition)) { \
            fflush(stdout); \
            fprintf(stderr, "Assertion `" #condition "` failed in %s:%d: ", __FILE__, __LINE__); \
            fprintf(stderr, ##args); \
            fprintf(stderr, "\n"); \
            fprintf(stderr, "problematic line: %s\n", line.c_str()); \
            Errors::semantic_error_handler(); \
        } \
    } while (false)


typedef std::string string;



#define zpp(fmt, args...) \
  std::printf(fmt, ##args);

#define zpl(fmt, args...) \
  std::printf(fmt, ##args); \
  std::printf("\n");

#define zps(arg) \
  std::printf(#arg ": %s", arg); \
  std::printf("\n");

#define zpd(arg) \
  std::printf(#arg ": %d", arg); \
  std::printf("\n");

#endif //LLPARSER_MACROS_H
