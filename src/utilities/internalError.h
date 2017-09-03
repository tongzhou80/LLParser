//
// Created by GentlyGuitar on 6/7/2017.
//

#ifndef LLPARSER_EXCEPTION_H
#define LLPARSER_EXCEPTION_H

#ifdef __linux__
#include <execinfo.h>
#endif
#include <signal.h>
#include <stdexcept>


class Errors {
public:
    static void init();
    static void install_sig_handlers();
    static void semantic_error_handler();
    static void sigsegv_handler(int sig, siginfo_t *siginfo, void *context);
    //static void sigsegv_handler(int sig);
    static void print_backtrace_symbols();
    static void die();
};


class FunctionNotFoundError: public std::runtime_error {
public:
    FunctionNotFoundError(std::string msg): runtime_error("Function " + msg + " not found") {}
    FunctionNotFoundError(): runtime_error("") {}
};

class InstructionNotFoundError: public std::runtime_error {
public:
    InstructionNotFoundError(std::string msg): runtime_error(msg) {}
    InstructionNotFoundError(): runtime_error("") {}
};

class SymbolTableError: public std::runtime_error {
public:
    SymbolTableError(std::string msg): runtime_error(msg) {}
    SymbolTableError(): runtime_error("") {}
};

class SymbolRedefinitionError: public std::runtime_error {
public:
    SymbolRedefinitionError(std::string msg): runtime_error(msg) {}
    SymbolRedefinitionError(): runtime_error("") {}
};

class BadPassArgumentError: public std::runtime_error {
public:
    BadPassArgumentError(std::string msg): runtime_error(msg) {}
    BadPassArgumentError(): runtime_error("") {}
};

class PassNotRegisteredError: public std::runtime_error {
public:
    PassNotRegisteredError(std::string msg): runtime_error(msg) {}
    PassNotRegisteredError(): runtime_error("") {}
};

class PassOpenFailedError: public std::runtime_error {
public:
    PassOpenFailedError(std::string msg): runtime_error(msg) {}
    PassOpenFailedError(): runtime_error("") {}
};

class TypeError: public std::runtime_error {
public:
    TypeError(std::string msg): runtime_error(msg) {}
    TypeError(): runtime_error("") {}
};



#endif //LLPARSER_EXCEPTION_H
