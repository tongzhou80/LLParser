//
// Created by tzhou on 8/24/17.
//

#include "diScope.h"
#include "diEssential.h"

string DIScope::filename() {
    guarantee(_file, " ");
    return _file->filename();
}