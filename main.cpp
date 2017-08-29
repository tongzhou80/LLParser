#include <iostream>
#include <utilities/flags.h>
#include <asmParser/sysDict.h>
#include "asmParser/llParser.h"
#include "passes/passManager.h"
#include "peripheral/optParser.h"
#include "peripheral/sysArgs.h"
#include "peripheral/timer.h"

void* llparser_start(string* filename) {
    Timer t;
    t.start();
    LLParser* llparser = NULL;
    if (ParallelModule) {
        llparser = new LLParser();
    }
    else {
        llparser = SysDict::parser;
    }
    llparser->parse(*filename);
    t.stop();
    zpl("file: %s; time: %.3f seconds, line: %lld", (*filename).c_str(), t.seconds(), llparser->line_numer());
}


int main(int argc, char** argv) {
    Flags::init();
    SysDict::init();
    PassManager::init();
    Errors::init();
    SoptInitArgs sopt_args;
    sopt_args.argc = argc;
    sopt_args.argv = argv;
    SysArgs::init(&sopt_args);


    /* todo:
     * callinst_list could be std::set, which is more error-resistent
     * Instruction needs a function_name field for debug
     */

    pthread_t* tids = NULL;
    int file_num = SysArgs::filenames().size();
    if (ParallelModule) {
        tids = new pthread_t[file_num];
    }


    for (int i = 0; i < file_num; ++i) {
        string file = SysArgs::filenames()[i];
        if (ParallelModule) {
            pthread_create(&tids[i], NULL, (void* (*)(void*))llparser_start, &SysArgs::filenames()[i]);
        }
        else {
            llparser_start(&file);
        }
    }

    if (ParallelModule) {
        for (int i = 0; i < file_num; i++)
            pthread_join(tids[i], NULL);
    }

    PassManager::pass_manager->apply_global_passes();


    Flags::destroy();
    SysDict::destroy();
    PassManager::destroy();

    return 0;
}