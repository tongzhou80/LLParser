//
// Created by tlaber on 7/8/17.
//


#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <set>
#include <inst/loadInst.h>
#include <asmParser/irBuilder.h>
#include <peripheral/sysArgs.h>

class AtracePass: public Pass {
    bool PrintCloning;
    bool TracingVerbose;
    std::set<string> _black;
    int _cast_cnt = 0;
    std::ofstream _ofs;
public:
    AtracePass() {
        set_is_basic_block_pass();
    }

    // Declare but not implement the destructor causes a ATrace symbol not found error in dlopen

    bool do_initialization(Module* M) override {
        string text = "declare void @xps_record_load(i8*)";
        Function* newfunc = IRBuilder::create_function_declaration(text);
        M->append_new_function(newfunc);
    }

    bool do_finalization(Module* M) {
        string out = Strings::replace(SysDict::filename(), ".ll", ".atrace.ll");
        if (SysArgs::has_property("output")) {
            out = SysArgs::get_property("output");
        }

        SysDict::module()->print_to_file(out);
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
            LoadInst* li = dynamic_cast<LoadInst*>(I);
            I->dump();
            if (li && !li->get_raw_field("pointer").empty()) {
                zpl("got %s, %s", I->raw_c_str(), li->pointer_type_str().c_str());
                string addr = li->get_raw_field("pointer");

                string casted_addr = "%tz_casted_addr" + std::to_string(_cast_cnt++);
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
            ++i;
        }

        return true;
    }

    bool run_on_instruction(Instruction* inst) {

    }


    //bool do_finalization(Module* module);
};

REGISTER_PASS(AtracePass)
