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
    int _discriminator;
public:
    DILexicalBlockFile(): _scope(NULL), _discriminator(0) {}

    DIScope* scope()                                           { return _scope; }
    int discriminator()                                        { return _discriminator; }
    void set_discriminator(int l)                              { _discriminator = l; }

    void resolve();

    string function();
    string dump(bool newline=true);
};




#endif //LLPARSER_DILEXICALBLOCKFILE_H
