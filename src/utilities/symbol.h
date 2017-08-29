//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_SYMBOL_H
#define LLPARSER_SYMBOL_H

#include <string>
#include <sstream>
#include <vector>
#include "strings.h"



class Symbol {
private:
    std::string _str;
public:
    Symbol();
    Symbol(std::string);
    Symbol(const char*);
    Symbol& operator=(std::string s);
    Symbol& operator=(const char* s);

    std::string str();
    const char* c_str();

    Symbol& prepend(std::string);
    Symbol& append(std::string);
    Symbol& operator+(std::string);
    friend Symbol& operator+(std::string, Symbol&);
};


#endif //LLPARSER_SYMBOL_H
