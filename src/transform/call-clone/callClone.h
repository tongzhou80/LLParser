//
// Created by tlaber on 6/25/17.
//

#ifndef LLPARSER_CALLCLONE_H
#define LLPARSER_CALLCLONE_H



#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <set>

class CallClonePass: public Pass {
    bool PrintCloning;
    bool TracingVerbose;
    std::set<string> _black;
    bool _has_overlapped_path;
    std::ofstream _ofs;
public:
    CallClonePass();
    ~CallClonePass();

    bool run_on_module(Module* module);

    void do_clone(Function* f, Module *module);

    //bool do_finalization(Module* module);
};




#endif //LLPARSER_CALLCLONE_H
