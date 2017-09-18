//
// Created by tlaber on 6/15/17.
//

#include "diFile.h"

void DIFile::resolve() {
    DI_SET_STR_FIELD(filename);
    DI_SET_STR_FIELD(directory);
}