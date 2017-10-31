//
// Created by tzhou on 8/10/17.
//

#include <set>
#include <algorithm>
#include <utilities/macros.h>
#include <asmParser/sysDict.h>
#include <passes/pass.h>
#include <di/diEssential.h>
#include <ir/irEssential.h>
#include <utilities/strings.h>
#include <peripheral/sysArgs.h>
#include <inst/instEssential.h>
#include <asmParser/irBuilder.h>


struct MFunc {
    string old_name;
    string new_name;
    bool add_id;

    MFunc(string oldname, string newname, bool addid):
            old_name(oldname), new_name(newname), add_id(addid) {}
};

/**@brief This class don't use SysDict::module()
 *
 */
class LSDAPass: public Pass {
    /* language specific stuff */
    string _lang;
    std::vector<MFunc*> _alloc_set;
    std::vector<MFunc*> _free_set;
    bool _use_indi;
public:
    LSDAPass() {
        set_is_module_pass();

        _lang = "all";
        _use_indi = false;
    }

    ~LSDAPass() {

    }

    const std::vector<MFunc *> &alloc_set() const {
        return _alloc_set;
    }

    void set_alloc_set(const std::vector<MFunc *> &_alloc_set) {
        LSDAPass::_alloc_set = _alloc_set;
    }

    const std::vector<MFunc *> &free_set() const {
        return _free_set;
    }

    void set_free_set(const std::vector<MFunc *> &_free_set) {
        LSDAPass::_free_set = _free_set;
    }

    bool use_indi() const {
        return _use_indi;
    }

    void set_use_indi(bool _use_indi) {
        LSDAPass::_use_indi = _use_indi;
    }

    void set_lang(string lang) {
        guarantee(lang == "c" || lang == "cpp" || lang == "fortran", "");
        _lang = lang;
    }

    const string& lang() {
        return _lang;
    }

    void init_lang(Module* module);

    void insert_lsd(Module* module);

    bool run_on_module(Module* module) override;

    bool insert_declaration(Module* m, string oldname, string newname, bool add_id=true);
};


REGISTER_PASS(LSDAPass);
