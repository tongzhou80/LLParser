//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_STRINGS_H
#define LLPARSER_STRINGS_H


#include <string>
#include <vector>


class Strings {
public:
    static std::vector<std::string> split(const std::string &s, char delim);
    static std::vector<std::string> split(char* s, char delim);
    static bool startswith(const std::string& s, const char* substr, bool skip_ws=true);
    static bool startswith(const std::string& s, const std::string substr, bool skip_ws=true);
    static bool endswith(const std::string& s, const char* substr);
    static bool endswith(const std::string& s, const std::string substr);
    static bool conatins(const std::string& s, const char* substr)             { return s.find(substr) != std::string::npos; }
    static bool conatins(const std::string& s, const std::string substr)             { return s.find(substr) != std::string::npos; }
    static char first_nonws_char(const std::string& s);
    static bool is_number(const std::string& s);
    static bool replace(std::string& s, std::string& oldsub, std::string& newsub);
    static bool replace(std::string& s, const char* oldsub, std::string& newsub);
    static bool replace(std::string& s, std::string& oldsub, const char* newsub);
    static bool replace(std::string& s, const char* oldsub, const char* newsub);
    static char* strip(char* str, const char* chars=" \t\n");
    static bool strip(std::string& str, const char* chars=" \t\n");
};



#endif //LLPARSER_STRINGS_H
