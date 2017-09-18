//
// Created by tzhou on 9/18/17.
//

#include <asmParser/sysDict.h>
#include <di/diEssential.h>
#include <set>
#include "contextGenerator.h"

void ContextGenerator::generate(Module* module, string alloc, int nlevel) {
    Function* malloc = module->get_function(alloc);
    if (!malloc) {
        return;
    }

    std::ofstream ofs;
    ofs.open(SysDict::filepath() + "contexts.txt");

    std::vector<CallInstFamily*> roots = malloc->caller_list();

    while (--nlevel) {
        std::vector<CallInstFamily*> new_roots;

        for (auto ci: roots) {
            Function* f = ci->function();
            new_roots.insert(new_roots.end(), f->caller_list().begin(), f->caller_list().end());
        }
        roots = new_roots;
    }

    int id = 0;
    for (auto ci: roots) {
        DILocation* loc = ci->debug_loc();
        ofs << id++ << " 0x0 " << alloc << std::endl;
        ofs << '(' << ci->function()->name() << '+' << ci->get_position_in_function() << ") " << loc->filename() << ':' << loc->line() << "\n\n";
    }

    ofs.close();
}