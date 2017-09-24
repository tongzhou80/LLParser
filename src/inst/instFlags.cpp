//
// Created by tlaber on 6/11/17.
//

#include <utilities/strings.h>
#include <cstring>
#include "instFlags.h"


std::set<string> InstFlags::_fastmaths;
std::set<string> InstFlags::_linkages;
std::set<string> InstFlags::_cconvs;
std::set<string> InstFlags::_param_attrs;
std::set<string> InstFlags::_tails;
std::set<string> InstFlags::_terminator_insts;

void InstFlags::init() {
    _fastmaths = { "nnan", "ninf", "nsz", "arcp", "contract", "fast"};
    _linkages = { "private", "internal", "available_externally", "weak", "linkonce", "common",
                  "appending", "extern_weak", "linkonce_odr", "weak_odr", "external" };
    _cconvs = { "ccc", "fastcc" };
    _param_attrs = { "zeroext", "signext", "inreg", "byval", "inalloca", "sret", "align", "noalias",
                     "nocapture", "nest", "returned", "nonnull", "dereferenceable", "dereferenceable_or_null" }; // more to go
    _tails = { "tail", "musttail", "notail" };

    _terminator_insts = { "ret", "br", "switch", "indirectbr", "invoke", "resume", "catchswitch", "catchret", "cleanupret", "unreachable" };
}

//bool InstFlags::in_tails(string key) {
//    return key == "tail" || key == "musttail" || key == "notail";
//}
void InstFlags::print_tails() {
    zpl("tails:")
    for (auto t: _tails) {
        zps(t);
    }
}

bool InstFlags::in_param_attrs(string key) {
    const char* p1 = "align%d";
    int dummy;
    int matched1 = sscanf(key.c_str(), p1, &dummy);
    if (matched1 == 1) {
        return true;
    }

    if (Strings::startswith(key, "dereferenceable_or_null")) {
        int start_pos = strlen("dereferenceable_or_null") + 2;
        guarantee(key[start_pos-1] == '(', " ");
        int end_pos = key.find(')', start_pos);
        string number = key.substr(start_pos, end_pos-start_pos);
        guarantee(Strings::is_number(number), " ");
        return true;
    }
    else if (Strings::startswith(key, "dereferenceable")) {
        int start_pos = strlen("dereferenceable") + 1;
        guarantee(key[start_pos-1] == '(', " ");
        int end_pos = key.find(')', start_pos);
        string number = key.substr(start_pos, end_pos-start_pos);
        guarantee(Strings::is_number(number), " ");
        return true;
    }

    return _param_attrs.find(key) != _param_attrs.end();
}