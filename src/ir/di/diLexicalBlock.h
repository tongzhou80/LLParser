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
    DIFile* _file;
    int _line;
    int _column;
//    int _file_id;
//    int _scope_id;
public:
    DILexicalBlock(): _scope(NULL), _file(NULL), _line(0), _column(0) {}

    DIScope* scope()                                  { return _scope; }
    DIFile* file()                                    { return _file; }

    int line()                                        { return _line; }
    void set_line(int l)                              { _line = l; }

    int column()                                      { return _column; }
    void set_column(int c)                            { _column = c; }

//    int scope_id()                                       { return _scope_id; }
//    void set_scope_id(int s)                             { _scope_id = s; }
//
//    int file_id()                                        { return _file_id; }
//    void set_file_id(int s)                              { _file_id = s; }

    void resolve();

    string filename();
    string function();
    string dump(bool newline=true);
};


#endif //LLPARSER_DILEXICALBLOCK_H
