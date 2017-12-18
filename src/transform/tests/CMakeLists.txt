cmake_minimum_required(VERSION 3.5)
include(../CMake.indi)

add_library(AddPrintf SHARED addPrintf.cpp)
add_library(WrfDebug SHARED wrf.cpp) 
