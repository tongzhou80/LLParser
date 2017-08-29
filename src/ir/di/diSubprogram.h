//
// Created by tlaber on 6/15/17.
//

#ifndef LLPARSER_DISUBPROGRAM_H
#define LLPARSER_DISUBPROGRAM_H

#include <utilities/macros.h>
#include "diScope.h"

class DIFile;


// An example
// !235 = distinct !DISubprogram(name: "spec_init", scope: !3, file: !3, line: 80, type: !236,
// isLocal: false, isDefinition: true, scopeLine: 80, isOptimized: true, unit: !2, variables: !238)

//struct RawDISubprogram {
//    string name;
//    string scope;
//    string file;
//
//};

class DISubprogram: public DIScope {
    // string _diname;  // already have _name field from Value class
    DIScope* _scope;  // more specific than DIScope
    DIFile* _file;
    int _line;
public:
    DISubprogram();

    string filename();

    int line()                                        { return _line; }
    void set_line(int line)                           { _line = line; }

    void resolve();
};

#endif //LLPARSER_DISUBPROGRAM_H
