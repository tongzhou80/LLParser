//
// Created by tlaber on 6/22/17.
//

#include "diLexicalBlockFile.h"
#include "diFile.h"

void DILexicalBlockFile::resolve_non_refs() {
//    if (has_field("scope")) {
//        _scope = dynamic_cast<DIScope*>(get_reference_by_key("scope"));
//        guarantee(_scope, " ");
//    }
    DI_SET_INT_FIELD(discriminator);
}

void DILexicalBlockFile::resolve_refs() {
    DI_SET_REF_FIELD(scope, DIScope);
    DI_SET_REF_FIELD(file, DIFile);
}