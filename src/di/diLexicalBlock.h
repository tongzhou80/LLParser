//
// Created by tlaber on 6/22/17.
//

#ifndef LLPARSER_DILEXICALBLOCK_H
#define LLPARSER_DILEXICALBLOCK_H


#include <utilities/macros.h>
#include "diScope.h"

class DIFile;
class DISubprogram;

class DILexicalBlock: public DIScope {
    DIScope* _scope;
    int _line;
    int _column;
public:
    DILexicalBlock(): _scope(NULL), _line(0), _column(0) {}

    DIScope* scope()                                  { return _scope; }
    DIFile* file()                                    { return _file; }

    int line()                                        { return _line; }
    void set_line(int l)                              { _line = l; }

    int column()                                      { return _column; }
    void set_column(int c)                            { _column = c; }

    void resolve_non_refs();
    void resolve_refs();

    string function();
    string dump(bool newline=true);
};


#endif //LLPARSER_DILEXICALBLOCK_H
