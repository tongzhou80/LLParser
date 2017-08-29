//
// Created by tzhou on 8/27/17.
//

#ifndef LLPARSER_FILEPARSER_H
#define LLPARSER_FILEPARSER_H

#include <fstream>
#include "stringParser.h"

class FileParser: public StringParser {
protected:
    string _file_name;
    std::ifstream _ifs;
    long long _line_number;

public:
    FileParser();
    const string& filename()                                 { return _file_name; }
    string& line()                                           { return _text; }
    long long line_numer()                                   { return _line_number; }

    std::ifstream& getline();
    std::ifstream& getline_nocomment();
    std::ifstream& getline_nonempty();
    std::ifstream& get_real_line();

    virtual void reset_parser();
};

#endif //LLPARSER_FILEPARSER_H
