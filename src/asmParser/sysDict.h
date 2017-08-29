//
// Created by tlaber on 6/9/17.
//

#ifndef LLPARSER_SYSDICT_H
#define LLPARSER_SYSDICT_H

#include <vector>
#include <ir/irEssential.h>


class LLParser;
class InstParser;

class SysDict {
public:
    static void init();
    static void destroy();

    //static Module* get_module(string name);

    /* thread specific */
    static void add_module(LLParser* );
    static LLParser* llparser();
    static Module* module();
    static const string& filename();

    static std::map<pthread_t , LLParser*> thread_table;
    static std::vector<Module*> modules;
    static LLParser* parser;
    //static InstParser* instParser;
};

#endif //LLPARSER_SYSDICT_H
