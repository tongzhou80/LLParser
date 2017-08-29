//
// Created by GentlyGuitar on 6/6/2017.
//

#ifndef LLPARSER_STRUCTTYPE_H
#define LLPARSER_STRUCTTYPE_H

#include "shadow.h"
#include "value.h"

/* in LLVM StructType derives from Type instead of Value */

class StructType: public Value {
public:
    StructType(): Value() {}
};


#endif //LLPARSER_STRUCTTYPE_H
