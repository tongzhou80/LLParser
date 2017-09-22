cmake_minimum_required(VERSION 3.5)
include(../CMake.config)

set(LLParserRoot "../../..")
include_directories(${LLParserRoot}/src)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LLParserRoot}/build/passes/")

add_library(Hello SHARED hello.cpp)
install(TARGETS Hello DESTINATION /home/marena/llparser/passes/)