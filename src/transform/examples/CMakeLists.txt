cmake_minimum_required(VERSION 3.5)
include(../CMake.config)

set(LLParserRoot "../../..")
include_directories(${LLParserRoot}/src)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LLParserRoot}/build/passes/")

add_library(ExampleModulePass SHARED examplePasses.cpp)
install(TARGETS ExampleModulePass DESTINATION /home/marena/llparser/passes/)