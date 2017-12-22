//
// Created by tlaber on 7/8/17.
//


#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <set>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>
#include <peripheral/sysArgs.h>

class AtracePass: public Pass {
    size_t _load_cnt = 0;
    size_t _store_cnt = 0;
    size_t _skipped = 0;
    bool _skip_gint = true;
    std::ofstream _ofs;
public:
    AtracePass() {
        set_is_basic_block_pass();
    }

    // Declare but not implement the destructor causes a ATrace symbol not found error in dlopen

    void do_initialization(Module* M) override {
        string text = "declare void @xps_record_load(i8*)";
        Function* newfunc = IRBuilder::create_function_declaration(text);
        M->append_new_function(newfunc);

        text = "declare void @xps_record_store(i8*)";
        newfunc = IRBuilder::create_function_declaration(text);
        M->append_new_function(newfunc);
    }

    void do_finalization(Module* M) override {
        string out = Strings::replace(SysDict::filename(), ".ll", ".atrace.ll");
        if (SysArgs::has_property("output")) {
            out = SysArgs::get_property("output");
        }

        SysDict::module()->print_to_file(out);
        printf("load count: %zu, store count: %zu, skip count: %zu, all: %zu\n",
               _load_cnt, _store_cnt, _skipped, _load_cnt+_store_cnt+_skipped);
    }

    void instrument_load(LoadInst* I, BasicBlock* bb) {

    }

    void instrument_store(LoadInst* I, BasicBlock* bb) {

    }

    bool run_on_basic_block(BasicBlock* bb) {
        if (bb->parent()->name() == "xps_record_load") {
            return false;
        }

        /* cannot use iterator here because of insertion */
        auto& ls = bb->instruction_list();
        int inst_num = ls.size();
        int i = 0;
        while (i < inst_num) {
            Instruction* I = ls[i];

            /* do load inst */
            if (LoadInst* li = dynamic_cast<LoadInst*>(I)) {
                if (!li->get_raw_field("pointer").empty()) {
                    //zpl("got %s, %s", I->raw_c_str(), li->pointer_type_str().c_str());
                    string addr = li->get_raw_field("pointer");
                    string value_ty = li->get_raw_field("ty");
                    // skip global ints and bools
                    if (_skip_gint && (value_ty == "i64" || value_ty == "i32" || value_ty == "i8") && addr[0] == '@') {  
                        i++;
                        zpl("skip %s", li->raw_c_str());
                        _skipped++;
                        continue;
                    }

                    string casted_addr = "%tz_load_addr" + std::to_string(_load_cnt++);
                    string cast_inst_text = "  " +  casted_addr + " = bitcast " + li->pointer_type_str() + " " + addr + " to i8*";
                    Instruction* cast_inst = IRBuilder::create_instruction(cast_inst_text);

                    string call_inst_text = "  call void @xps_record_load(i8* " + casted_addr + ")";
                    Instruction* call_inst = IRBuilder::create_instruction(call_inst_text);

                    BasicBlock::InstList ls = {cast_inst, call_inst};
                    if (call_inst && cast_inst) {
                        bb->insert_instruction_after(li, ls);

                        inst_num += ls.size();
                        i += ls.size();
                        continue;
                    }
                    else {
                        zpl("create instruction failed");
                    }
                }
            }

            /* do store inst */
            if (StoreInst* si = dynamic_cast<StoreInst*>(I)) {
                if (!si->get_raw_field("pointer").empty()) {
                    //zpl("got %s, %s", I->raw_c_str(), si->pointer_type_str().c_str());
                    string addr = si->get_raw_field("pointer");
                    string value_ty = si->get_raw_field("ty");
                    if (_skip_gint && (value_ty == "i64" || value_ty == "i32" || value_ty == "i8") && addr[0] == '@') {
                        i++;
                        zpl("skip %s", si->raw_c_str());
                        _skipped++;
                        continue;
                    }


                    string casted_addr = "%tz_store_addr" + std::to_string(_store_cnt++);
                    string cast_inst_text = "  " +  casted_addr + " = bitcast " + si->get_raw_field("ty") + "* " + addr + " to i8*";
                    Instruction* cast_inst = IRBuilder::create_instruction(cast_inst_text);

                    string call_inst_text = "  call void @xps_record_store(i8* " + casted_addr + ")";
                    Instruction* call_inst = IRBuilder::create_instruction(call_inst_text);

                    BasicBlock::InstList ls = {cast_inst, call_inst};
                    if (call_inst && cast_inst) {
                        bb->insert_instruction_after(si, ls);

                        inst_num += ls.size();
                        i += ls.size();
                        continue;
                    }
                    else {
                        zpl("create instruction failed");
                    }
                }
            }

            ++i;
        }

        return true;
    }

    //void do_finalization(Module* module);
};

REGISTER_PASS(AtracePass)
