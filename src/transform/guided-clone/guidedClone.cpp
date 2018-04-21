//
// Created by tzhou on 10/28/17.
//

#include <cassert>
#include <cstdio>
#include <ir/irEssential.h>
#include <passes/pass.h>
#include <asmParser/sysDict.h>
#include <asmParser/llParser.h>
#include <di/diSubprogram.h>
#include <utilities/strings.h>
#include <transform/ben-alloc/benAlloc.cpp>

class GuidedClonePass: public Pass {
  std::ofstream _ofs;
  BenAllocPass* _lsda;
  std::map<string, std::vector<string>> _name_map;

  /* command line args */
  string _log_dir;
  string _src_dir;
  string _lang;
  bool _use_indi;
  bool _noclone;  // only does ben replacement
  bool _noben;  // only does clone and no replacement


  /* flags */
  bool _load_verbose;
  bool _replace_verbose;

  /* statistics */
  int _clone_num;
public:
  GuidedClonePass() {
    set_is_global_pass();
    _log_dir = "./";
    _src_dir = "./";
    _lang = "all";
    _use_indi = false;
    _noclone = false;
    _noben = false;
    _load_verbose = false;
    _replace_verbose = false;
    _clone_num = 0;
  }

  void init() {
    /* set source file dir */
    if (has_argument("log_dir")) {
      _log_dir = get_argument("log_dir");
    }
    if (has_argument("src_dir")) {
      _src_dir = get_argument("src_dir");
    }
    if (has_argument("lang")) {
      _lang = get_argument("lang");
    }
    if (has_argument("indi")) {
      _use_indi = (bool)std::stoi(get_argument("indi"));
    }
    if (has_argument("noben")) {
      _noben = (bool)std::stoi(get_argument("noben"));
    }
    if (has_argument("noclone")) {
      _noclone = (bool)std::stoi(get_argument("noclone"));
    }
    _lsda = new BenAllocPass(_lang);
    _lsda->do_initialization();
    _lsda->set_use_indi(_use_indi);

    load_filename_map();
  }

  string get_ir_name_from_source_name(string source) {
    int pos = source.rfind('.');
    guarantee(pos != string::npos, "");
    string ir_name = _src_dir + source.substr(0, pos) + ".o.ll";
    return ir_name;
  }

  Module* get_module(string ir_name) {
    Module* m = SysDict::get_module(ir_name);
    if (!m) {
      m = SysDict::parser->parse(ir_name);
      if (_load_verbose) {
        printf("parsed %s\n", m->name_as_c_str());
      }
      //guarantee(m, "");
    }

    return m;
  }

  /** @brief Get function by function name and file name
   *
   */
  Function* get_function(string& func, string& file) {
    if (Strings::endswith(file, "c")
      || Strings::endswith(file, "cc")
      || Strings::endswith(file, "cpp")
      ) {
      string ir_name = get_ir_name_from_source_name(file);
      Module* m = get_module(ir_name);
      guarantee(m, "Module not found for %s", file.c_str());
      return m->get_function_by_orig_name(func);
    } else {
      // There is no corresponding IR for <file>
      // Typically <func> is defined in a header
      // Inclusion relations should be present
      guarantee(_name_map.find(file) != _name_map.end(), "");
      for (auto src: _name_map[file]) {
        if (auto f = get_function(func, src)) {
          return f;
        }
      }
      guarantee(0, "Failed to find function %s (%s)",
                func.c_str(), file.c_str());
    }

  }

  void process_clone_log() {
    std::ifstream ifs(_log_dir+"/clone.log");
    string line;
    while (std::getline(ifs, line)) {
      auto fields = Strings::split(line, ' ');
      string callee_file = fields.at(0);
      // name of the function that's to be cloned
      string callee = fields.at(1);
      string new_callee = fields.at(2);
      string caller_file = fields.at(3);
      string caller = fields.at(4);
      string use_loc = fields.at(5);
      Point2D<int> point(use_loc);

      Function* calleeF = get_function(callee, callee_file);
      Function* callerF = get_function(caller, caller_file);
      guarantee(calleeF, "Function %s not found", callee.c_str());
      guarantee(callerF, "Function %s not found", caller.c_str());
      do_clone(calleeF, callerF, new_callee, point);
    }
  }

  void do_clone(Function* callee, Function* caller, string newname,
                Point2D<int> callsite) {
    Module* calleeM = callee->parent();
    Module* callerM = caller->parent();
    Function* callee_clone = callee->clone(newname);
    _clone_num++;
    //zpl("callee: %s, user: %s", callee.c_str(), user.c_str());
    //zpl("append cloned f %s", callee_clone->name().c_str());
    calleeM->append_new_function(callee_clone);

    auto user_i = dynamic_cast<CallInstFamily*>(
      caller->get_instruction(callsite));
    guarantee(user_i, "");

    /* need to insert declaration if inter-procedural */
    if (callerM != calleeM) {
      _lsda->insert_declaration(callerM,
                                user_i->called_function()->name(),
                                callee_clone->name(), false);
    }
    user_i->replace_callee(callee_clone->name());
    if (_replace_verbose) {
      printf("replaced %s\n", callee_clone->name_as_c_str());
    }
  }

  bool run_on_global() override {
    init();

    if (!_noclone) {
      process_clone_log();
    }
    printf("clone done\n");

    if (!_noben) {
      use_ben_malloc();
      printf("ben-alloc done\n");
    }

    for (auto it: SysDict::module_table()) {
      Module* m = it.second;
      //m->print_to_file(Strings::replace(
      // m->input_file(), ".ll", ".clone.ll"));
      m->print_to_file(m->input_file());
    }
    return true;
  }

  void load_filename_map() {
    std::ifstream ifs(_log_dir+"/inclusion.txt");
    string line;
    guarantee(ifs.good(), "file inclusion.txt not found");
    while (std::getline(ifs, line)) {
      auto p1 = line.find(' ');
      auto f1 = line.substr(0, p1);
      auto f2 = line.substr(p1+1);
      if (_name_map.find(f1) == _name_map.end()) {
        _name_map[f1] = std::vector<string>();
      }
      _name_map[f1].push_back(f2);
    }
  }

  void use_ben_malloc() {
    std::ifstream ifs(_log_dir+"/ben.log");
    string line;
    std::set<Module*> scanned;

    while (std::getline(ifs, line)) {
      if (Module* m = get_module(line)) {
        if (scanned.find(m) != scanned.end()) {
          continue;
        }
        _lsda->insert_lsd(m);
        _lsda->replace_alloc(m);
        _lsda->replace_free(m);
        if (_use_indi) {
          _lsda->replace_indi(m);
        }
        scanned.insert(m);
      }
    }
  }
};

REGISTER_PASS(GuidedClonePass)
