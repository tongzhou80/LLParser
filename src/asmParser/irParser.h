//
// Created by tzhou on 9/17/17.
//

#ifndef LLPARSER_LLASMPARSER_H
#define LLPARSER_LLASMPARSER_H

#include <peripheral/stringParser.h>

class Value;

/**@brief IRParser provides common parsing routines for LLVM language.
 * Both LLParser (parse the whole file) and InstParser (parse an vtruction) derive from it.
 *
 */
class IRParser: public virtual StringParser {
public:
    string match_identifier();
    string match_xalpha();
    string match_xalphanumeric();
    string parse_basic_type();
    string parse_compound_type();
    string parse_complex_structs();
    void set_fastmath(Value* v);
    void set_cconv(Value* v);
    void set_linkage(Value* v);
    void set_param_attrs(Value* v);
    void set_ret_attrs(Value* v);
    void set_visibility(Value* v);
};

#endif //LLPARSER_LLASMPARSER_H
