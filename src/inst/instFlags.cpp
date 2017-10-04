//
// Created by tlaber on 6/11/17.
//

#include <utilities/strings.h>
#include <cstring>
#include "instFlags.h"


std::set<string> InstFlags::_fastmaths;
std::set<string> InstFlags::_linkages;
std::set<string> InstFlags::_cconvs;
std::set<string> InstFlags::_visibilities;
std::set<string> InstFlags::_dll_storage_classes;
std::set<string> InstFlags::_param_attrs;
std::set<string> InstFlags::_tails;
std::set<string> InstFlags::_terminator_insts;

void InstFlags::init() {
    _fastmaths = { "nnan", "ninf", "nsz", "arcp", "contract", "fast"};
    _linkages = { "private", "internal", "available_externally", "weak", "linkonce", "common",
                  "appending", "extern_weak", "linkonce_odr", "weak_odr", "external" };
    _cconvs = { "ccc", "fastcc", "coldcc", "cc 10", "cc 11", "webkit_jscc", "anyregcc", "preserve_mostcc", "preserve_allcc", "cxx_fast_tlscc", "swiftcc" };
    _visibilities = { "default", "hidden", "protected" };  // A symbol with internal or private linkage must have default visibility.
    _dll_storage_classes = { "dllimport", "dllexport" };
    _param_attrs = { "zeroext", "signext", "inreg", "byval", "inalloca", "sret", "align", "noalias",
                     "nocapture", "nest", "returned", "nonnull", "dereferenceable", "dereferenceable_or_null" }; // more to go
    _tails = { "tail", "musttail", "notail" };

    _terminator_insts = { "ret", "br", "switch", "indirectbr", "invoke", "resume", "catchswitch", "catchret", "cleanupret", "unreachable" };
}

bool InstFlags::is_cconv_flag(const string& key) {
    if (_cconvs.find(key) != _cconvs.end()) {
        return true;
    }

    if (Strings::startswith(key, "cc")) {
        return Strings::is_number(key.substr(2));
    }
}


bool InstFlags::is_param_attr_flag(const string& key) {
    const char* p1 = "align%d";
    int dummy;
    int matched1 = sscanf(key.c_str(), p1, &dummy);
    if (matched1 == 1) {
        return true;
    }

    /* dereferenceable_or_null never seemed to be used */
    const char* prefixes[2] = { "dereferenceable", "dereferenceable_or_null" };
    for (auto prefix: prefixes) {
        if (Strings::startswith(key, prefix)) {
            int start_pos = strlen(prefix) + 1;
            guarantee(key[start_pos-1] == '(', "char: %c, key: %s", key[start_pos-1], key.c_str());
            int end_pos = key.find(')', start_pos);
            string number = key.substr(start_pos, end_pos-start_pos);
            guarantee(Strings::is_number(number), " ");
            return true;
        }
    }

    return _param_attrs.find(key) != _param_attrs.end();
}