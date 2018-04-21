//
// Created by tlaber on 6/14/17.
//


#include <fstream>
#include <array>
#include <memory>
#include "systems.h"


bool Systems::is_file_exist(std::string filename) {
  std::ifstream infile(filename);
  return infile.good();
}

std::string Systems::exec(std::string cmd) {
  std::array<char, 128> buffer;
  std::string result;
  std::shared_ptr<FILE> pipe(popen(cmd.c_str(), "r"), pclose);
  if (!pipe) throw std::runtime_error("popen() failed!");
  while (!feof(pipe.get())) {
    if (fgets(buffer.data(), 128, pipe.get()) != nullptr)
      result += buffer.data();
  }
  return result;
}