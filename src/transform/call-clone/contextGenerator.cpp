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

void ContextGenerator::generate(Module* module, string alloc, int nlevel) {
    _paths.clear();
    _stack.clear();
    Function* alloc_f = module->get_function(alloc);

    if (!alloc_f) {
        return;
    }
    
    for (auto ci: alloc_f->caller_list()) {
        XPath* path = new XPath;
        path->hotness = 0;
        path->path.push_back(ci);
        _paths.push_back(path);
    }

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

            if (tos->function()->caller_list().empty()) {
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
        }
        _paths = new_paths;
    }

    zpl("get all paths of %d layers of %s call chain, write to file...", nlevel, alloc.c_str())

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
            DILocation* loc = ci->debug_loc();
            _ofs << '(' << ci->function()->name() << '+' << ci->get_position_in_function() << ") "
                << loc->filename() << ':' << loc->line() << std::endl;
        }
        _ofs << std::endl;
    }
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