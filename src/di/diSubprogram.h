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
    string _linkageName;
    DIScope* _scope;  // more specific than DIScope
    int _line;
public:
    DISubprogram();

    int line()                                        { return _line; }
    void set_line(int line)                           { _line = line; }

    const string &linkageName() const {
        if (has_raw_field("linkageName")) {
            return _linkageName;
        }
        else {
            return name();
        }
    }

    void set_linkageName(const string &linkageName) {
        _linkageName = linkageName;
    }

    void resolve_non_refs() override ;
    void resolve_refs() override ;

    string to_string();
};

#endif //LLPARSER_DISUBPROGRAM_H
