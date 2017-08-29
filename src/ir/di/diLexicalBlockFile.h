//
// Created by tlaber on 6/22/17.
//

#ifndef LLPARSER_DILEXICALBLOCKFILE_H
#define LLPARSER_DILEXICALBLOCKFILE_H



#include <utilities/macros.h>
#include "diScope.h"

class DIFile;
class DISubprogram;

class DILexicalBlockFile: public DIScope {
    DIScope* _scope;
    DIFile* _file;
    int _discriminator;
//    int _file_id;
//    int _scope_id;
public:
    DILexicalBlockFile(): _scope(NULL), _file(NULL), _discriminator(0) {}

    DIScope* scope()                                           { return _scope; }
    int discriminator()                                        { return _discriminator; }
    void set_discriminator(int l)                              { _discriminator = l; }
//
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




#endif //LLPARSER_DILEXICALBLOCKFILE_H
