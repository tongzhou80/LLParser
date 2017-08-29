//
// Created by tlaber on 6/14/17.
//


#include <fstream>
#include "systems.h"


bool Systems::is_file_exist(std::string filename) {
    std::ifstream infile(filename);
    return infile.good();
}