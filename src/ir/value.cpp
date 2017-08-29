//
// Created by GentlyGuitar on 6/6/2017.
//

#include <fstream>
#include <algorithm>
#include <cstdio>
#include "value.h"

Value::Value(): Shadow() {
    _name = "";
    _copy_cnt = 0;
}
//
//bool Value::has_property(string key) {
//    if (_properties.find(key) == _properties.end()) {
//        return false;
//    }
//    else {
//        return true;
//    }
//}
//
//string Value::get_properties(string key) {
//    if (has_property(key)) {
//        return _properties[key];
//    }
//    else {
//        return "";
//    }
//}
//
//void Value::set_property(string key, string value) {
//    _properties[key] = value;
//}

void Value::remove_user(Instruction *user) {
    auto& vec = user_list();
    auto newend = std::remove(vec.begin(), vec.end(), user);
    vec.erase(newend, vec.end());
}

void Value::print_to_file(const char *file) {
    FILE* fp = fopen(file, "w");
    if (fp == NULL) {
        fprintf(stderr, "open file %s failed", file);
        return;
    }
    print_to_stream(fp);
    fclose(fp);
}

void Value::print_to_file(string file) {
    print_to_file(file.c_str());
//    std::ofstream ofs(file);
//    print_to_stream(ofs);
//    ofs.close();
}

void Value::print_to_stream(std::ofstream &ofs) {
    //ofs << "Value.print_to_stream called, are you sure this should be called?\n";
}

void Value::print_to_stream(FILE *fp) {
    fprintf(fp, "%s\n", raw_text().c_str());
    //fprintf(fp, "Value.print_to_stream called, are you sure this should be called?\n");
}