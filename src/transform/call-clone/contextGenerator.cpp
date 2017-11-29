//
// Created by tzhou on 9/18/17.
//

#include <ctime>
#include <asmParser/sysDict.h>
#include <di/diEssential.h>
#include <set>
#include "contextGenerator.h"

ContextGenerator::ContextGenerator() {
    string out = SysDict::filedir() + "contexts.txt";
    _ofs.open(out);
    _ofs.close();  // clear the file and start appending
    _counter = 0;
    _ofs.open(out, std::ios::app);
}

ContextGenerator::~ContextGenerator() {
    _ofs.close();
}

std::vector<XPath*> ContextGenerator::generate(Module* module, string alloc, int nlevel) {
    _paths.clear();
    _stack.clear();
    Function* alloc_f = module->get_function(alloc);

    if (!alloc_f) {
        return std::vector<XPath*>();
    }

    int skip_cnt = 0;
    for (auto ci: alloc_f->caller_list()) {
        /* A special handler for wrf, with disabling this problematic function,
         * 1. there're over 2M contexts which cause memory usage issue
         * 2. the generated new IR is uncompilable for whatever reason
         */
        // if (ci->function()->name() == "module_configure_in_use_for_config_") {
        //     skip_cnt++;
        //     continue;
        // }
        
        XPath* path = new XPath;
        path->hotness = 0;
        path->path.push_back(ci);
        _paths.push_back(path);
    }

    printf("skipped: %d\n", skip_cnt);

    zpl("to get all paths of %d layers of %s call chain", nlevel, alloc.c_str())

    /* keep all contexts whose depth is less than nlevel */
    while (--nlevel) {
        std::vector<XPath*> new_paths;
        for (auto xpath: _paths) {
            auto& context = xpath->path;
            auto tos = context[context.size()-1];

            //            if (tos->function()->name() == "main") {
//                new_paths.push_back(xpath);
//                zpl("reached main:")
//                for (auto i: context) {
//                    zpl("%s ", i->function()->name_as_c_str())
//                }
//            }

            if (tos->function()->caller_list().empty()
                || tos->function()->name() == "module_configure_in_use_for_config_") {
                new_paths.push_back(xpath);
                zpl("reached top: %s", tos->function()->name_as_c_str());
            }
            else {
                for (auto ci: tos->function()->caller_list()) {
                    XPath* nxp = new XPath();
                    nxp->hotness = xpath->hotness;
                    nxp->path = context;
                    nxp->path.push_back(ci);
                    new_paths.push_back(nxp);
                }
            }
            //delete xpath;
        }
        
        _paths.clear();
        _paths = new_paths;
        zpl("context generation one round done")
    }

    zpl("done with %s", alloc.c_str())

    srand(time(NULL));
    double p = (double)rand() / (double)RAND_MAX;
    int id = 0;
    for (auto xpath: _paths) {
        string hotness = "0x0";
        if (rand() / (double)RAND_MAX > 0.5) {
            hotness = "0xffffffff";
        }
        _ofs << _counter++ << " " + hotness + " " << alloc << std::endl;
        for (auto ci: xpath->path) {
            guarantee(ci, "");
            DILocation* loc = ci->debug_loc();
            if (!loc) {
                std::cerr << "No DILocation found for:" << ci->raw_text() << std::endl;
                exit(-1);
            }
            _ofs << '(' << ci->function()->name() << '+' << ci->get_position_in_function() << ") "
                << loc->filename() << ':' << loc->line() << std::endl;
        }
        _ofs << std::endl;
    }

    return _paths;
}

void ContextGenerator::traverse() {
    auto tos = _stack[_stack.size()-1];
    for (auto ci: tos->function()->caller_list()) {
        _stack.push_back(ci);
        if (_stack.size() < 3) {
            traverse();
        }
        else {

        }
    }
}
