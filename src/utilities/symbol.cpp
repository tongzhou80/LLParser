//
// Created by GentlyGuitar on 6/6/2017.
//

#include "symbol.h"



Symbol::Symbol() {

}

Symbol::Symbol(std::string s) {
    _str = s;
}

Symbol::Symbol(const char * name) {
    _str = name;
}

Symbol& Symbol::operator=(std::string s) {
    _str = s;
}

Symbol& Symbol::operator=(const char *s) {
    _str = s;
}

Symbol& Symbol::prepend(std::string pre) {
    _str = pre + _str;
    return *this;
}

Symbol& Symbol::operator+(std::string s) {
    return append(s);
}

Symbol& Symbol::append(std::string ap) {
    _str += ap;
    return *this;
}

Symbol& operator+(std::string str, Symbol & symbol) {
    return symbol.prepend(str);
}

const char* Symbol::c_str() {
    return _str.c_str();
}

std::string Symbol::str() {
    return _str;
}
