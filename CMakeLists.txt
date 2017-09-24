cmake_minimum_required(VERSION 3.5)
project(LLParser)

include_directories(src/)
SET(CMAKE_CXX_FLAGS "-std=c++11 -g -rdynamic")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DLLDEBUG -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DPRODUCTION -O3")

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}/build/passes/")

file(GLOB pass_cmakelists
        "src/transform/*/CMakeLists.txt"
        )
# include(${pass_cmakelists})  # uncomment to make each pass a individual project

add_library(GlobalHello SHARED src/transform/global-pass-hello/globalHello.cpp)
add_library(Malloc SHARED src/transform/malloc/nmalloc.cpp)
add_library(CallClone SHARED src/transform/call-clone/callClone.cpp src/transform/call-clone/contextGenerator.h src/transform/call-clone/contextGenerator.cpp)
add_library(Callgraph SHARED src/transform/call-graph/callGraph.cpp)
add_library(ATrace SHARED src/transform/atrace/atrace.cpp)
add_library(HotCallClone SHARED src/transform/hot-call-clone/hotCallClone3.cpp)
add_library(NewClone SHARED src/transform/new-clone/newClone.cpp)
add_library(PrintCaller SHARED src/transform/print-caller/printCaller.cpp)
add_library(SplitModule SHARED src/transform/split-module/splitModule.cpp)

set_target_properties(NewClone PROPERTIES COMPILE_FLAGS "-g -rdynamic")

install(TARGETS
        GlobalHello
        Malloc
        CallClone
        Callgraph
        ATrace
        HotCallClone
        NewClone
        PrintCaller
        DESTINATION /home/marena/llparser/passes/)

set(SOURCE_FILES main.cpp src/ir/module.cpp src/ir/module.h
        src/ir/value.cpp src/ir/value.h src/ir/shadow.cpp
        src/ir/shadow.h src/utilities/flags.cpp src/utilities/flags.h
        src/ir/globalVariable.cpp src/ir/globalVariable.h
        src/ir/function.cpp src/ir/function.h
        src/utilities/symbol.cpp src/utilities/symbol.h
        src/asmParser/llParser.cpp src/asmParser/llParser.h
        src/utilities/strings.cpp src/utilities/strings.h
        src/utilities/macros.cpp src/utilities/macros.h
        src/ir/structType.cpp src/ir/structType.h
        src/utilities/internalError.cpp src/utilities/internalError.h
        src/ir/basicBlock.cpp src/ir/basicBlock.h
        src/ir/instruction.cpp src/ir/instruction.h
        src/ir/attribute.cpp src/ir/attribute.h
        src/ir/metaData.cpp src/ir/metaData.h
        src/passes/pass.cpp src/passes/pass.h
        src/passes/passManager.cpp src/passes/passManager.h
        src/ir/irEssential.h
        src/asmParser/sysDict.cpp src/asmParser/sysDict.h
        src/transform/malloc/nmalloc.cpp
        src/asmParser/instParser.cpp src/asmParser/instParser.h
        src/inst/callInst.cpp src/inst/callInst.h
        src/inst/instEssential.h
        src/inst/instFlags.cpp src/inst/instFlags.h
        src/peripheral/optParser.cpp src/peripheral/optParser.h
        src/peripheral/sysArgs.cpp src/peripheral/sysArgs.h
        src/peripheral/stringParser.cpp src/peripheral/stringParser.h
        src/peripheral/argsParser.cpp src/peripheral/argsParser.h
        src/utilities/systems.cpp src/utilities/systems.h
        src/di/debugInfo.cpp src/di/debugInfo.h
        src/di/diFile.cpp src/di/diFile.h
        src/di/diSubprogram.cpp src/di/diSubprogram.h
        src/di/diLocation.cpp src/di/diLocation.h
        src/di/diEssential.h
        src/peripheral/timer.cpp src/peripheral/timer.h
        src/asmParser/irBuilder.cpp src/asmParser/irBuilder.h
        src/ir/type.cpp src/ir/type.h
        src/di/diScope.cpp src/di/diScope.h
        src/di/diLexicalBlock.cpp src/di/diLexicalBlock.h
        src/di/diLexicalBlockFile.cpp src/di/diLexicalBlockFile.h
        src/ir/comdat.cpp src/ir/comdat.h
        src/inst/loadInst.cpp src/inst/loadInst.h
        src/inst/bitcastInst.cpp src/inst/bitcastInst.h
        src/passes/InterModularPass.cpp src/passes/InterModularPass.h
        src/di/diType.cpp src/di/diType.h
        src/di/diCompositeType.cpp src/di/diCompositeType.h
        src/inst/callInstFamily.cpp src/inst/callInstFamily.h
        src/inst/invokeInst.cpp src/inst/invokeInst.h
        src/peripheral/FileParser.cpp src/peripheral/FileParser.h
        src/utilities/mutex.cpp src/utilities/mutex.h
        src/asmParser/llParserTLS.cpp src/asmParser/llParserTLS.h
        src/ir/alias.cpp src/ir/alias.h
        src/asmParser/irParser.cpp src/asmParser/irParser.h src/inst/branchInst.cpp src/inst/branchInst.h src/inst/getelementptrInst.cpp src/inst/getelementptrInst.h)
add_executable(sopt ${SOURCE_FILES} )
install(TARGETS sopt DESTINATION /home/marena/llparser/bin/)
target_link_libraries(sopt "-ldl") # for dyopen etc.

