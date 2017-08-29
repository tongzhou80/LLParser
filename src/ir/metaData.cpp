//
// Created by GentlyGuitar on 6/7/2017.
//

#include <asmParser/sysDict.h>
#include "metaData.h"

MetaData* MetaData::get_reference_by_value(string md_id) {
    guarantee(!md_id.empty() && md_id[0] == '!', "A metadata reference always starts with a '!'");

    int id = std::stoi(md_id.substr(1));
    return SysDict::module()->get_debug_info(id);
}

MetaData* MetaData::get_reference_by_key(string field) {
    if (has_raw_field(field)) {
        return get_reference_by_value(get_raw_field(field));
    }
    else {
        return NULL;
    }
}