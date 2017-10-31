//
// Created by GentlyGuitar on 6/6/2017.
//

#include "macros.h"

template <>
Point2D<int>::Point2D(std::string s) {
    int pos = s.find('_');
    guarantee(pos != std::string::npos, "");
    x = std::stoi(s.substr(0, pos));
    y = std::stoi(s.substr(pos+1));
}