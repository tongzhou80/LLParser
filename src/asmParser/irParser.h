//
// Created by tzhou on 9/17/17.
//

#ifndef LLPARSER_LLASMPARSER_H
#define LLPARSER_LLASMPARSER_H

#include <peripheral/stringParser.h>

class Value;

/**@brief IRParser provides common parsing routines for LLVM language.
 * Both LLParser (parse the whole file) and InstParser (parse an instruction) derive from it.
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
    void set_fastmath_flag(Value* ins);
    void set_cconv_flag(Value* ins);
    void set_linkage_flag(Value* ins);
    void set_param_attrs(Value* ins);
    void set_ret_attrs(Value* ins);
};

#endif //LLPARSER_LLASMPARSER_H
