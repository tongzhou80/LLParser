//
// Created by tlaber on 6/15/17.
//

#ifndef LLPARSER_DILOCATION_H
#define LLPARSER_DILOCATION_H

#include <ir/metaData.h>

/* Strangely I don't have to declare DISubprogram and DIScope to make it compilable
 * I guess it's because that two classes happen to be parsed when it sees this one
 */
class DISubprogram;
class DIScope;

class DILocation: public MetaData {
    int _line;
    int _column;
    DISubprogram* _subprogram;
    DIScope* _scope;
    DILocation* _inlinedAt;  // should be of subtype DILocation*
    DILocation* _inline_root;
public:
    DILocation();
    int line();
    void set_line(int l)                              { _line = l; }

    int column()                                      { return _column; }
    void set_column(int c)                            { _column = c; }

    DIScope* scope()                                  { return _scope; }
    DILocation* inlinedAt()                           { return _inlinedAt; }

    string filename();
    string function();
    string dump(bool newline=true);

    void resolve();
    void second_resolve();
};

#endif //LLPARSER_DILOCATION_H
