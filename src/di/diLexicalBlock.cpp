//
// Created by tlaber on 6/22/17.
//

#include "diLexicalBlock.h"
#include "diFile.h"

void DILexicalBlock::resolve_non_refs() {
    DI_SET_INT_FIELD(line);
    DI_SET_INT_FIELD(column);
}

void DILexicalBlock::resolve_refs() {
    DI_SET_REF_FIELD(scope, DIScope);
    DI_SET_REF_FIELD(file, DIFile);
}