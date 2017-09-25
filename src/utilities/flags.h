//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_FLAGS_H
#define LLPARSER_FLAGS_H

#include <map>
#include <string>


#define DECLARE_DIAGNOSTIC_FLAG(type, name, value, doc)    extern type name;
#define DECLARE_DEVELOP_FLAG(type, name, value, doc)       extern type name;

#define DEFINE_DIAGNOSTIC_FLAG(type, name, value, doc)    type name = value;
#define DEFINE_DEVELOP_FLAG(type, name, value, doc)       type name = value;


/* In order to make add_flag work, GENERATE_RUNTIME_FLAGS only can have one argument for now */
/* Use short name (develop, etc) instead of the long macro name */
// #define GENERATE_RUNTIME_FLAGS(develop, product)
#define GENERATE_RUNTIME_FLAGS(develop) \
  develop(bool, DebugRun, 0,                                                              \
         "")                                                                              \
  develop(bool, ArgParsingVerbose, 0,                                                     \
         "")                                                                              \
  develop(bool, LazyParsing, 1,                                                           \
         "")                                                                              \
  develop(bool, UseLabelComments, 1,                                                      \
         "")                                                                              \
  develop(bool, ParallelModule, 0,                                                        \
         "Parse all inputs in parallel")                                                  \
  develop(bool, UseSplitModule, 0,                                                        \
         "Load sliced sub-modules and merge all inputs into one module.")                 \
  develop(bool, ParallelInstruction, 0,                                                   \
         "")                                                                              \
  develop(bool, PrintParsedLine, 0,                                                       \
         "")                                                                              \
  develop(bool, BasicBlockParsingVerbose, 0,                                              \
         "")                                                                              \
  develop(bool, FunctionParsingVerbose, 0,                                                \
         "")                                                                              \
  develop(bool, CallInstParsingVerbose, 0,                                                \
         "")                                                                              \
  develop(bool, InstructionParsingVerbose, 0,                                             \
         "")                                                                              \
  develop(bool, UseParseTimePasses, 0,                                                    \
         "")                                                                              \
  develop(bool, IgnoreIntrinsicCalls, 0,                                                  \
         "")                                                                              \
  develop(bool, NoParserWarning, 0,                                                       \
         "")                                                                              \

// GENERATE_RUNTIME_FLAGS(DECLARE_DEVELOP_FLAG, DECLARE_DIAGNOSTIC_FLAG, DECLARE_DEVELOP_FLAG, DECLARE_DIAGNOSTIC_FLAG)
GENERATE_RUNTIME_FLAGS(DECLARE_DEVELOP_FLAG)


struct Flag {
    Flag(char t, void* v): type(t), value(v) {}
    char type;
    void* value;
};

class Flags {
    static std::map<std::string, Flag*> _map;
public:
    static void init();
    static void destroy();
    static Flag* get_flag(std::string key);
    static void set_flag(std::string key, bool v);
    static void set_flag(std::string key, int v);
    static void set_flag(std::string key, std::string value);
};







#endif //LLPARSER_FLAGS_H
