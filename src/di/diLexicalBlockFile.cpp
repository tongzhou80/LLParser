//
// Created by tlaber on 6/22/17.
//

#include "diLexicalBlockFile.h"
#include "diFile.h"

void DILexicalBlockFile::resolve() {
//    if (has_field("scope")) {
//        _scope = dynamic_cast<DIScope*>(get_reference_by_key("scope"));
//        guarantee(_scope, " ");
//    }

    DI_SET_REF_FIELD(scope, DIScope);
    DI_SET_REF_FIELD(file, DIFile);
    DI_SET_INT_FIELD(discriminator);
}
