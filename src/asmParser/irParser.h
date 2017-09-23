//
// Created by tzhou on 9/17/17.
//

#ifndef LLPARSER_LLASMPARSER_H
#define LLPARSER_LLASMPARSER_H

#include <peripheral/stringParser.h>

/**@brief IRParser provides common parsing routines for LLVM language.
 * Both LLParser (parse the whole file) and InstParser (parse an instruction) derive from it.
 *
 */
class IRParser: public virtual StringParser {
public:
    string parse_identifier();
    string parse_basic_type();
    string parse_compound_type();
    string parse_complex_structs();
};

#endif //LLPARSER_LLASMPARSER_H
