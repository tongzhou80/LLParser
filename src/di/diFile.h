//
// Created by tlaber on 6/15/17.
//

#ifndef LLPARSER_DIFILE_H
#define LLPARSER_DIFILE_H

#include <utilities/macros.h>
#include "diScope.h"

class DIFile: public DIScope {
    string _filename;
    string _directory;

public:
    string filename()                          { return _filename; }
    void set_filename(string fn)               { _filename = fn; }

    string directory()                         { return _directory; }
    void set_directory(string d)               { _directory = d; }

    void resolve();
};

#endif //LLPARSER_DIFILE_H
