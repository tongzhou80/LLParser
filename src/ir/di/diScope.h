//
// Created by tzhou on 8/24/17.
//

#ifndef LLPARSER_DISCOPE_H
#define LLPARSER_DISCOPE_H

#include <ir/metaData.h>

class DIFile;

/** DIScope represents a scope such as DIFile, DISubprogram
 *
 */
class DIScope: public MetaData {
protected:
    DIFile* _file;
public:
    DIScope(): _file(NULL) {}
    string filename();
};

#endif //LLPARSER_DISCOPE_H
