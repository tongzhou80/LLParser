//
// Created by tlaber on 6/9/17.
//

#include <unistd.h>
#include <wait.h>
//#include "utilities/macros.h"
#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>
#include <di/diEssential.h>
#include <iomanip>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>

struct MFunc {
    string old_name;
    string new_name;
    bool add_id;
    bool need_declare;

    MFunc(string oldname, string newname, bool addid, bool needdeclare):
            old_name(oldname), new_name(newname), add_id(addid), need_declare(needdeclare) {}
};

class MallocPass: public Pass {
    int _counter;
    std::vector<MFunc*> _targets;
    std::ofstream _ofs;
    bool _test_mode;
public:
    MallocPass() {
        _test_mode = 0;
        set_is_module_pass();

        _counter = 1;
        init_targets();
        _ofs.open("alloc-guide.txt");

        _ofs << std::setiosflags(std::ios::left);
        _ofs << std::setw(10) << "id" << std::setw(10) << "tier" << std::setw(20) << "location" << std::endl;

    }

    ~MallocPass() {
        _ofs.close();        
    }

    void init_targets() {
        /* right now need_declare is always true */
        // C
        _targets.push_back(new MFunc("malloc", "ben_malloc", true, true));
        _targets.push_back(new MFunc("calloc", "ben_calloc", true, true));
        _targets.push_back(new MFunc("realloc", "ben_realloc", true, true));
        _targets.push_back(new MFunc("free", "ben_free", false, true));

        // C++
        _targets.push_back(new MFunc("_Znam", "ben_malloc", true, true));
        _targets.push_back(new MFunc("_Znwm", "ben_malloc", true, true));
        _targets.push_back(new MFunc("_ZdaPv", "ben_free", false, true));
        _targets.push_back(new MFunc("_ZdlPv", "ben_free", false, true));

        // Fortran
        _targets.push_back(new MFunc("f90_alloc", "f90_ben_alloc", true, true));
        _targets.push_back(new MFunc("f90_alloc03", "f90_ben_alloc03", true, true));
        _targets.push_back(new MFunc("f90_alloc03_chk", "f90_ben_alloc03_chk", true, true));
        _targets.push_back(new MFunc("f90_alloc04", "f90_ben_alloc04", true, true));
        _targets.push_back(new MFunc("f90_alloc04_chk", "f90_ben_alloc04_chk", true, true));

        _targets.push_back(new MFunc("f90_kalloc", "f90_ben_kalloc", true, true));
        _targets.push_back(new MFunc("f90_calloc", "f90_ben_calloc", true, true));
        _targets.push_back(new MFunc("f90_calloc03", "f90_ben_calloc03", true, true));
        _targets.push_back(new MFunc("f90_calloc04", "f90_ben_calloc04", true, true));
        _targets.push_back(new MFunc("f90_kcalloc", "f90_ben_kcalloc", true, true));
        _targets.push_back(new MFunc("f90_ptr_alloc", "f90_ben_ptr_alloc", true, true));
        _targets.push_back(new MFunc("f90_ptr_alloc03", "f90_ben_ptr_alloc03", true, true));
        _targets.push_back(new MFunc("f90_ptr_alloc04", "f90_ben_ptr_alloc04", true, true));
        _targets.push_back(new MFunc("f90_ptr_src_alloc03", "f90_ben_ptr_src_alloc03", true, true));
        _targets.push_back(new MFunc("f90_ptr_src_alloc04", "f90_ben_ptr_src_alloc04", true, true));
        _targets.push_back(new MFunc("f90_ptr_src_calloc03", "f90_ben_ptr_src_calloc03", true, true));
        _targets.push_back(new MFunc("f90_ptr_src_calloc04", "f90_ben_ptr_src_calloc04", true, true));
        _targets.push_back(new MFunc("f90_ptr_kalloc", "f90_ben_ptr_kalloc", true, true));
        _targets.push_back(new MFunc("f90_ptr_calloc", "f90_ben_ptr_calloc", true, true));
        _targets.push_back(new MFunc("f90_ptr_calloc03", "f90_ben_ptr_calloc03", true, true));
        _targets.push_back(new MFunc("f90_ptr_calloc04", "f90_ben_ptr_calloc04", true, true));
        _targets.push_back(new MFunc("f90_ptr_kcalloc", "f90_ben_ptr_kcalloc", true, true));
        _targets.push_back(new MFunc("f90_auto_allocv", "f90_ben_auto_allocv", true, true));
        _targets.push_back(new MFunc("f90_auto_alloc", "f90_ben_auto_alloc", true, true));
        _targets.push_back(new MFunc("f90_auto_alloc04", "f90_ben_auto_alloc04", true, true));
        _targets.push_back(new MFunc("f90_auto_calloc", "f90_ben_auto_calloc", true, true));
        _targets.push_back(new MFunc("f90_auto_calloc04", "f90_ben_auto_calloc04", true, true));

        _targets.push_back(new MFunc("f90_dealloc", "f90_ben_dealloc", false, true));
        _targets.push_back(new MFunc("f90_dealloc03", "f90_ben_dealloc03", false, true));
        _targets.push_back(new MFunc("f90_dealloc_mbr", "f90_ben_dealloc_mbr", false, true));
        _targets.push_back(new MFunc("f90_dealloc_mbr03", "f90_ben_dealloc_mbr03", false, true));
        _targets.push_back(new MFunc("f90_deallocx", "f90_ben_deallocx", false, true));
        _targets.push_back(new MFunc("f90_auto_dealloc", "f90_ben_auto_dealloc", false, true));
        // todo
    }

    void delete_targets() {
        // todo
    }

    void do_replacement() {
        for (auto it = _targets.begin(); it != _targets.end(); ++it) {
            MFunc* mf = *it;
            if (mf->need_declare) {
                insert_declaration(mf->old_name, mf->new_name, mf->add_id);
            }

            Function* target = SysDict::module()->get_function(mf->old_name);

            if (target) {
                auto users_copy = target->caller_list();
                for (auto it = users_copy.begin(); it != users_copy.end(); ++it) {
                    if (CallInstFamily* ci = dynamic_cast<CallInstFamily*>(*it)) {
                        BasicBlock* parent = ci->parent();
                        modify_callinst(ci, mf->new_name, mf->add_id);
                    }
                }
            }
        }
    }

    bool run_on_module(Module* module) {
        do_replacement();
        
        string out = SysDict::filename();
        if (out.empty()) {
            out = "new";
        }
        out += ".malloc";
        if (SysArgs::has_property("output")) {
            out = SysArgs::get_property("output");
        }

        zpl("counter: %d", _counter);

        module->print_to_file(out);
    }

    bool insert_declaration(string oldname, string newname, bool add_id=true) {
        Function* func = SysDict::module()->get_function(oldname);

        if (func == NULL) {
            return 0;
        }
        guarantee(func->is_external(), "malloc family should be external");
        //Function* newfunc = SysDict::module()->create_child_function(_new_malloc);

        Function* existed = SysDict::module()->get_function(newname);
        if (existed) {
            return 0; // return if already inserted
        }

        /* manipulate the text */
        string text = func->raw_text();
        string old_call = oldname + '(';
        string new_call = newname + "(i32, ";
        if (!add_id || Strings::startswith(oldname, "f90_")) {  // a bit funky
            new_call = newname + '(';
        }

        Strings::ireplace(text, old_call, new_call);
        Function* newfunc = IRBuilder::create_function_declaration(text);
        SysDict::module()->insert_function_after(func, newfunc);
    }

    void modify_callinst(CallInstFamily* ci, string new_callee, bool add_id=true) {
        zpl("old inst %s", ci->raw_text().c_str());

        // only when add_id is on will it be an allocation site
        if (add_id) {
            _ofs << std::setiosflags(std::ios::left);
            _ofs << std::setw(10) << _counter;

            int default_tier = 0;
            _ofs << std::setw(10) << default_tier;

            /* disable debug info for a while, I want to rewrite the debug info parsing system */
            DILocation* loc = ci->debug_loc();
            if (loc) {
                printf("  in %s:%d (%s)\n", loc->filename().c_str(), loc->line(), loc->function().c_str());
                string loc_string = loc->filename() + ':' + std::to_string((long long)loc->line());
                loc_string += '(' + loc->function() + ')';
                _ofs << std::setw(20) << loc_string;
            }
            _ofs << std::endl;
        }
        else {
            DILocation* loc = ci->debug_loc();
            if (loc) {
                printf("  in %s:%d (%s)\n", loc->filename().c_str(), loc->line(), loc->function().c_str());
            }
        }


        if (ci->is_indirect_call()) {
            Instruction* tarins = ci->chain_inst();
            BitCastInst* bci = dynamic_cast<BitCastInst*>(tarins);
            if (bci && modify_bitcast(bci, new_callee, add_id)) {
                string new_args;
                if (add_id) {
                    new_args = "i32 " + std::to_string(_counter) + ", " + ci->get_raw_field("args");
                }
                else {
                    new_args = ci->get_raw_field("args");
                }
                ci->replace_args(new_args);
            }
        }
        else {
            ci->replace_callee(new_callee);

            string new_args;
            if (add_id) {
                new_args = "i32 " + std::to_string(_counter) + ", " + ci->get_raw_field("args");
            }
            else {
                new_args = ci->get_raw_field("args");
            }
            ci->replace_args(new_args);
        }

        if (ci->is_varargs()) {
            if (add_id) {
                string args_sig = ci->get_raw_field("fnty");
                guarantee(!args_sig.empty(), "problematic line: %s", ci->raw_c_str());
                string new_sig = args_sig;
//            zpl("old sig: %s", args_sig.c_str());
//            zpl("new sig: %s", new_sig.c_str());

                new_sig.insert(1, "i32, ");
                Strings::ireplace(ci->raw_text(), args_sig, new_sig);
            }
        }

        zpl("new inst %s\n", ci->raw_text().c_str());

        if (add_id && !_test_mode) {
            ++_counter;
        }
    }

    Instruction* create_new_call(CallInst* old, string new_callee, bool add_id=true) {
        zpl("old inst %s", old->raw_text().c_str());

        // only when add_id is on will it be an allocation site
        if (add_id) {
            _ofs << std::setiosflags(std::ios::left);
            _ofs << std::setw(10) << _counter;

            int default_tier = 0;
            _ofs << std::setw(10) << default_tier;

            /* disable debug info for a while, I want to rewrite the debug info parsing system */
            DILocation* loc = old->debug_loc();
            if (loc) {
                printf("  in %s:%d (%s)\n", loc->filename().c_str(), loc->line(), loc->function().c_str());
                string loc_string = loc->filename() + ':' + std::to_string((long long)loc->line());
                loc_string += '(' + loc->function() + ')';
                _ofs << std::setw(20) << loc_string;
            }
            _ofs << std::endl;
        }
        else {
            DILocation* loc = old->debug_loc();
            if (loc) {
                printf("  in %s:%d (%s)\n", loc->filename().c_str(), loc->line(), loc->function().c_str());
            }
        }

        CallInst* ci = dynamic_cast<CallInst*>(old);
        guarantee(ci, " ");
        string old_callee;
        char new_call[1024] = {0};

        if (ci->is_indirect_call()) {
            old_callee = '%' + ci->called_label() + '(';
            if (add_id) {
                sprintf(new_call, "%%%s(i32 %d, ", ci->called_label().c_str(), _counter);
            }
            else {
                sprintf(new_call, "%%%s(", ci->called_label().c_str());
            }
        }
        else {
            old->replace_callee(new_callee);
            string new_args;
            if (add_id) {
                new_args = "i32 " + std::to_string(_counter) + ", " + old->get_raw_field("args");
            }
            else {
                new_args = old->get_raw_field("args");
            }

            old->replace_args(new_args);
        }

        /* manipulate the text */
        string text = old->raw_text();
        Strings::ireplace(text, old_callee, new_call);

        if (ci->is_varargs()) {
            if (add_id) {
                string args_sig = ci->get_raw_field("fnty");
                guarantee(!args_sig.empty(), "problematic line: %s", ci->raw_c_str());
                string new_sig = args_sig;
//            zpl("old sig: %s", args_sig.c_str());
//            zpl("new sig: %s", new_sig.c_str());

                new_sig.insert(1, "i32, ");
                Strings::ireplace(text, args_sig, new_sig);
            }
        }
        Instruction* newinst = IRBuilder::create_instruction(text);
        zpl("new inst %s\n", newinst->raw_text().c_str());

        newinst->copy_metadata_from(old);
        if (add_id && !_test_mode) {
            ++_counter;
        }
        return newinst;
    }

    bool modify_bitcast(BitCastInst* bci, string new_value, bool add_id=true) {
        /* manipulate the text */
        if (bci->get_raw_field("value")[0] != '@') {
            return false;
        }

        zpl("  old bitcast %s", bci->raw_text().c_str());
        bci->update_raw_field("value", '@' + new_value);

        if (add_id) {
            string new_casted_ty = bci->get_raw_field("ty2");
            int insert_pos = new_casted_ty.find('(');
            new_casted_ty.insert(insert_pos+1, "i32, ");
            bci->update_raw_field("ty2", new_casted_ty);
        }
        zpl("  new bitcast %s", bci->raw_text().c_str());
        return true;
    }

    Instruction* create_new_bitcast(Instruction* old, string new_value, bool add_id=true) {
        zpl("old bitcast %s", old->raw_text().c_str());

        BitCastInst* bci = dynamic_cast<BitCastInst*>(old);
        guarantee(bci, " ");

        /* manipulate the text */
        string text = old->raw_text();
        string old_value = bci->get_raw_field("value");
        new_value = '@' + new_value;
        Strings::ireplace(text, old_value, new_value);

        if (add_id) {
            string old_casted_ty = bci->get_raw_field("ty2");
            string new_casted_ty = old_casted_ty;
            int insert_pos = new_casted_ty.find('(');
            new_casted_ty.insert(insert_pos+1, "i32, ");
            Strings::ireplace(text, old_casted_ty, new_casted_ty);
        }

        Instruction* newinst = IRBuilder::create_instruction(text);
        zpl("new inst %s\n", newinst->raw_text().c_str());

        newinst->copy_metadata_from(old);
        return newinst;
    }

};



REGISTER_PASS(MallocPass);
