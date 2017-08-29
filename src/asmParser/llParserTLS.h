//
// Created by tzhou on 8/28/17.
//

#ifndef LLPARSER_LLPARSERTHREAD_H
#define LLPARSER_LLPARSERTHREAD_H

#include <utilities/macros.h>

class Module;

/* Thread local storage for a LLParser instance */
class LLParserTLS {
    Module* _module;
    string _file;
public:
    Module *module() const {
        return _module;
    }

    void set_module(Module *_module) {
        LLParserTLS::_module = _module;
    }

    const string &file() const {
        return _file;
    }

    void set_file(const string &_file) {
        LLParserTLS::_file = _file;
    }
};

#endif //LLPARSER_LLPARSERTHREAD_H
