//
// Created by tlaber on 6/14/17.
//

#ifndef LLPARSER_SYSTEM_H
#define LLPARSER_SYSTEM_H

#include "macros.h"

class Systems {
public:
  static bool is_file_exist(std::string filename);
  static std::string exec(std::string cmd);
};

#endif //LLPARSER_SYSTEM_H
