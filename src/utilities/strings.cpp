//
// Created by GentlyGuitar on 6/6/2017.
//

#include <sstream>
#include "strings.h"

std::vector<std::string> Strings::split(char* s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> Strings::split(const std::string  &s, char delim) {
    std::stringstream ss(s);
    std::string item;
    std::vector<std::string> elems;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

bool Strings::startswith(const std::string  &s, const char *substr, bool skip_nw) {
    std::string ss(substr);
    return Strings::startswith(s, ss, skip_nw);
}

bool Strings::startswith(const std::string  &s, std::string substr, bool skip_nw) {
    std::string newstr;
    if (skip_nw) {
        int real_start = s.find_first_not_of(" \t");
        if (real_start == s.npos) {
            return false;
        }
        else {
            newstr = s.substr(real_start);
        }
    }
    else {
        newstr = s;
    }

    if (newstr.find(substr) == 0) {
        return true;
    }
    else {
        return false;
    }
}

char Strings::first_nonws_char(const std::string  &s) {
    std::size_t found = s.find_first_not_of(" \t");
    if (found != std::string::npos) {
        return s[found];
    }
    else {
        return 0;
    }
}

bool Strings::endswith(const std::string  &s, const char *ending) {
    return Strings::endswith(s, std::string(ending));
}

bool Strings::endswith(const std::string  &s, std::string ending) {
    if (ending.size() > s.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), s.rbegin());
}

bool Strings::is_number(const std::string &s) {
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it)) ++it;
    return !s.empty() && it == s.end();
}

bool Strings::ireplace(std::string  &s, std::string oldsub, std::string newsub) {
    int pos = s.find(oldsub);
    if (pos == s.npos) {
        return false;
    }
    else {
        s.replace(pos, oldsub.size(), newsub);
        return true;
    }
}

bool Strings::ireplace(std::string  &s, const char *oldsub, std::string &newsub) {
    std::string o = oldsub;
    return ireplace(s, o, newsub);
}

bool Strings::ireplace(std::string  &s, std::string &oldsub, const char *newsub) {
    std::string n = newsub;
    return ireplace(s, oldsub, n);
}

bool Strings::ireplace(std::string  &s, const char *oldsub, const char *newsub) {
    std::string o = oldsub;
    std::string n = newsub;
    return ireplace(s, o, n);
}

std::string Strings::replace(const std::string &s, const char *oldsub, const char *newsub) {
    std::string ret = s;
    ireplace(ret, oldsub, newsub);
    return ret;
}

bool Strings::strip(std::string& str, const char *chars) {
    str.erase(0, str.find_first_not_of(chars));
    str.erase(str.find_last_not_of(chars)+1);
    return 1;
}

/** This function assumes str is null-terminated
 *
 * @param str
 * @param chars
 * @return
 */
char* Strings::strip(char *str, const char *chars) {
    int beg = 0;
    std::string chars_str(chars);
    for (; chars_str.find(str[beg]) != chars_str.npos; ++beg) {

    }

    if (str[beg] == '\0') {
        return str+beg;
    }

    int end = beg;
    for (; str[end] != '\0'; ++end) {

    }

    end--;
    for (; chars_str.find(str[end]) != chars_str.npos; --end) {

    }

    str[end+1] = '\0';

    return str+beg;
}
