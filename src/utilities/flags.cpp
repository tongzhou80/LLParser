//
// Created by GentlyGuitar on 6/6/2017.
//

#include <map>
#include <cassert>
#include <iomanip>
#include "flags.h"
#include "macros.h"


// GENERATE_RUNTIME_FLAGS(DEFINE_DEVELOP_FLAG, DEFINE_DIAGNOSTIC_FLAG, DEFINE_DEVELOP_FLAG, DEFINE_DIAGNOSTIC_FLAG)
GENERATE_RUNTIME_FLAGS(DEFINE_DEVELOP_FLAG)


#define ADD_FLAG_TO_MAP(type, name, value, doc)                ty = #type; _map[#name] = new Flag(ty[0], &name);
#define DELETE_FLAG(type, name, value, doc)                    delete _map[#name];

#define PRINT_FLAG(type, name, value, doc) \
  std::cout \
  << "flag: " \
  << std::left << std::setw(30) << #name \
  << " type: " \
  << std::left << std::setw(10) << #type \
  << " default: " \
  << std::left << std::setw(10) << value \
  << " doc: " \
  << doc << "\n";

std::map<std::string, Flag*> Flags::_map;



void Flags::init() {
    std::string ty;
    GENERATE_RUNTIME_FLAGS(ADD_FLAG_TO_MAP)
}

void Flags::destroy() {
    GENERATE_RUNTIME_FLAGS(DELETE_FLAG)
}

void Flags::print_flags() {
    GENERATE_RUNTIME_FLAGS(PRINT_FLAG)
}

Flag* Flags::get_flag(std::string key) {
    if (_map.find(key) == _map.end()) {
        return NULL;
    }
    else {
        return _map[key];
    }
}

void Flags::set_flag(std::string key, bool v) {
    Flag* f = get_flag(key);
    if (f == NULL) {
        fprintf(stderr, "Unrecognized option: %s\n", key.c_str());
        std::terminate();
    }
    assert(f->type == 'b' && "option type must be bool");
    *(bool*)(f->value) = v;

    if (key == "UseSplitModule") {
        set_flag("ParallelModule", true);
    }
}

void Flags::set_flag(std::string key, int v) {
    Flag* f = get_flag(key);
    if (f == NULL) {
        fprintf(stderr, "Unrecognized option: %s\n", key.c_str());
        std::terminate();
    }
    assert(f->type == 'i' && "option type must be int");
    *(int*)(f->value) = v;
}

void Flags::set_flag(std::string key, std::string value) {
    Flag* f = get_flag(key);
    if (f == NULL) {
        fprintf(stderr, "Unrecognized option: %s\n", key.c_str());
        std::terminate();
    }
    assert(f->type == 's' && "option type must be std::string");
    *(std::string*)(f->value) = value;
//    else {
//        std::string& type = f->type;
//        void* v;
//        if (type == "bool") {
//            *(bool*)v = std::stoi(value);
//        }
//        else if (type == "std::string") {
//            *(std::string*)v = value;
//        }
//        else if (type == "int") {
//            *(int*)v = std::stoi(value);
//        }
//        else {
//            fprintf(stderr, "Unsupported flag type: %s", type.c_str());
//            std::terminate();
//        }
//    }
}