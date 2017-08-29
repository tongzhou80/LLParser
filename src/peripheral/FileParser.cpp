//
// Created by tzhou on 8/27/17.
//

#include <utilities/flags.h>
#include "FileParser.h"

FileParser::FileParser() {
    _line_number = 0;
}

void FileParser::reset_parser() {
    StringParser::reset_parser();
    _line_number = 0;

    if (_ifs.is_open()) {
        _ifs.close();
    }
    _file_name.clear();
}

std::ifstream& FileParser::getline() {
    if (std::getline(_ifs, _text)) {
        _line_number++;
        _intext_pos = 0;
        _char = line()[_intext_pos];
        _eol = false;

#ifdef __CYGWIN__
        assert(_line[_line.size()-1] == '\r' && "Windows new line encoding should be \\r\\n");
        _line = _line.substr(0, _line.size()-1);
#endif
        if (PrintParsedLine) {
            printf("%lld: %s\n", _line_number, line().c_str());
        }
    }

    return _ifs;
}

std::ifstream& FileParser::getline_nocomment() {
    do {
        if (!getline()) {
            break;
        }
    } while (line()[0] == ';');

    return _ifs;
}

std::ifstream& FileParser::getline_nonempty() {
    do {
        if (!getline()) {
            break;
        }
    } while (line().empty());

    return _ifs;
}

std::ifstream& FileParser::get_real_line() {
    do {
        if (!getline()) {
            break;
        }
    } while (line().empty() || line()[0] == ';');

    return _ifs;
}