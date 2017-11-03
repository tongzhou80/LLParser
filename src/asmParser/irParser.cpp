//
// Created by tzhou on 9/17/17.
//

#include <asmParser/instFlags.h>
#include <ir/instruction.h>
#include "irParser.h"

/**@brief Match extended alphabetic ([-a-zA-Z$._]) strings until the current char is not a match
 * The state of the parser changes as the matching goes
 * @return The matched string
 */
string IRParser::match_xalpha() {
    int startp = _intext_pos;
    int len = 0;
    while (!_eol) {
        if (std::isalpha(_char)) {
            inc_intext_pos();
        }
        else if (_char == '-' || _char == '$' || _char == '.' || _char == '_') {
            inc_intext_pos();
        }
        else {
            break;
        }
        len++;
    }

    return _text.substr(startp, len);
}

/**@brief Match extended alphanumeric ([-a-zA-Z$._0-9]) strings until the current char is not a match
 * The state of the parser changes as the matching goes
 * @return The matched string
 */
string IRParser::match_xalphanumeric() {
    int startp = _intext_pos;
    int len = 0;
    while (!_eol) {
        if (std::isalnum(_char)) {
            inc_intext_pos();
        }
        else if (_char == '-' || _char == '$' || _char == '.' || _char == '_') {
            inc_intext_pos();
        }
        else {
            break;
        }
        len++;
    }

    return _text.substr(startp, len);
}

/**@brief Match an identifier, which could be the name of variables, functions, structs, etc.
 * The actual regular expression used is '[%@][-a-zA-Z$._][-a-zA-Z$._0-9]*'.
 * Identifiers that require other characters in their names can be surrounded with quotes.
 *
 * @return The matched string
 */
string IRParser::match_identifier() {
    /* not including the starting % or @ */
    range_check();

    if (_char == '"') {
        inc_intext_pos();
        get_word('"');
        return '"' + _word + '"';
    }
    else {
        string p1 = match_xalpha();
        parser_assert(!p1.empty(), "The identifier does not start with [-a-zA-Z$._]: %c", _char);
        string p2 = match_xalphanumeric();

        return p1 + p2;
    }
}

string IRParser::match_value() {
    if (_char == '%' || _char == '@') {  // named variables and unnamed variables(%num)
        get_word_of(" ,");
        return _word;
    }
    else {  // constants
        // todo: check if lookahead is constant expressions such as inttoptr
        return match_constant();
    }
}

string IRParser::match_simple_constant() {
    if (_char == 't') {
        match("true");
        return "true";
    }
    else if (_char == 'f') {
        match("false");
        return "false";
    }
    else if (_char == 'n') {
        get_word_of(" ,");
        syntax_check(_word == "none" || _word == "null");
        return _word;
    }
    else {
        string number = match_number();
        parser_assert(!number.empty(), "_char: %c", _char);
        return number;
    }
}

string IRParser::match_complex_constant() {
    return jump_to_end_of_scope();
}

string IRParser::match_constant() {
    get_lookahead_of(" ,");
    if (_lookahead == "zeroinitializer") {
        jump_ahead();
        return _lookahead;
    }
    else if (_lookahead == "inttoptr"
             || _lookahead == "trunc"
             || _lookahead == "zext"
             || _lookahead == "sext"
             || _lookahead == "fptrunc"
             || _lookahead == "fpext"
             || _lookahead == "fptoui"
             || _lookahead == "fptosi"
             || _lookahead == "uitofp"
             || _lookahead == "sitofp"
             || _lookahead == "ptrtoint"
             || _lookahead == "inttoptr"
             || _lookahead == "bitcast"
             || _lookahead == "addrspacecast"
            ) {
        jump_ahead();
        return jump_to_end_of_scope();
    }

    if (_char == '{' || _char == '[' || _char == '<') {
        return match_complex_constant();
    }
    else {
        return match_simple_constant();
    }
}

/**@brief Match an integer or a float number
 *
 * @return
 */
string IRParser::match_number() {
    if (try_match("0x")) {
        return match_hexnum();
    }
    else {
        return match_decnum();
    }
}

string IRParser::match_hexnum() {
    int startp = _intext_pos;
    int len = 0;
    int ndot = 0;
    for (; !_eol && std::isxdigit(_char); len++) {
        inc_intext_pos();
    }

    string matched = _text.substr(startp, len);
    return matched;
}

string IRParser::match_decnum() {
    int startp = _intext_pos;
    int len = 0;
    int ndot = 0;
    string sign = "";
    if (_char == '-' || _char == '+') {
        sign += _char;
        inc_intext_pos();
    }

    while (!_eol) {
        if (std::isdigit(_char)) {
            inc_intext_pos();
        }
        else if (_char == '.') {
            inc_intext_pos();
            ndot++;
        }
        else if (_char == 'e') {
            inc_intext_pos();
            syntax_check(_char == '+' || _char == '-'); // exponential notation
            inc_intext_pos();
        }
        else {
            break;
        }
        len++;
    }

    string matched = _text.substr(startp, len);
    guarantee(ndot < 2, "invalid number: %s", matched.c_str());
    return sign+matched;
}

string IRParser::parse_basic_type() {
    skip_ws();

    string fulltype;
    switch (_char) {
        case 'v': {
            fulltype = "void";
            match(fulltype);
            break;
        }
        case 'i': {  // i8, i16, i32...
            inc_intext_pos();
            int size = parse_integer();
            fulltype = 'i' + std::to_string(size);
            break;
        }
        case 'f': {  // float
            fulltype = "float";
            match(fulltype);
            break;
        }
        case 'd': {  // double
            fulltype = "double";
            match(fulltype);
            break;
        }
        case 'h': {  // half
            fulltype = "half";
            match(fulltype);
            break;
        }
        case '<': {  // vector
            fulltype = jump_to_end_of_scope();
            break;
        }
        case '%': {
            fulltype = parse_complex_structs();
            break;
        }
        case '{': {  // structure type like %5 = tail call { i64, { float, float }* } @quantum_new_matrix(i32 2, i32 2) #9, !dbg !2700
            fulltype = jump_to_end_of_scope();
            break;
        }
        case '[': {  // array type
            fulltype = jump_to_end_of_scope();
            break;
        }
        default: {
            parser_assert(0, "bad type, _char: %c", _char);
        }
    }

    while (!_eol && _char == '*') {
        fulltype += _char;
        inc_intext_pos();
    }

    return fulltype;
}

/**@brief Parse a basic type or a compound type such as function pointer
 *
 * Like function pointers, a function type (without a '*' at the end) is also considered a compound type.
 * The _char will point to the next non-whitespace character upon returning
 *
 * @return
 */
string IRParser::parse_compound_type() {
    /* corner case
     * tail call void bitcast (void (%struct.bContext*, %struct.uiBlock.22475* (%struct.bContext*, %struct.ARegion*, i8*)*, i8*)* @uiPupBlock
     */
    string ty = parse_basic_type();

    skip_ws();

    if (_char == '(') {
        ty += ' ';
        ty += jump_to_end_of_scope();
//        guarantee(_char == '*', "Bad function pointer type: %s", _text.c_str());
//        ty += '*';
//        inc_intext_pos();

        // could a pointer to a function pointer, like i8* (i8*, i32, i32)** %bzalloc ...
        while (!_eol && _char == '*') {
            ty += '*';
            inc_intext_pos();
        }
    }
    skip_ws();

    return ty;
}

/**@brief Parse structs of the following form: (not parsing the * at the end, if any)
 * Parsing starts with '%'
 *
 * %struct.name
 * %class.name
 * %union.name
 * %"struct..."
 * %"class..."
 *
 * @return
 */
string IRParser::parse_complex_structs() {
    match('%');

    string name = match_identifier();
    /* doesn't have to start with these */
    //parser_assert(name[0] == 's' || name[0] == 'c' || name[0] == 'u' || name[0] == '"', "sanity");

    return '%' + name;
}

void IRParser::set_optional_field(Value *v, string field) {
    get_lookahead();
    if (_lookahead == field) {
        v->set_raw_field(field, field);
        jump_ahead();
    }
}

/**@brief The following functions check _lookahead and set the relevant flag if presented
 * 
 * @param v 
 */
void IRParser::set_cconv(Value *v) {
    get_lookahead();
    if (IRFlags::is_cconv_flag(_lookahead)) {
        v->set_raw_field("cconv", _lookahead);
        jump_ahead();
    }
}

void IRParser::set_tail(Value *v) {
    get_lookahead();
    if (IRFlags::is_tail_flag(_lookahead)) {
        v->set_raw_field("tail", _lookahead);
        jump_ahead();
    }
}

void IRParser::set_fastmath(Value *v) {
    get_lookahead();
    if (IRFlags::is_fastmath_flag(_lookahead)) {
        v->set_raw_field("fast-math", _lookahead);
        jump_ahead();
    }
}

void IRParser::set_linkage(Value *v) {
    get_lookahead();
    if (IRFlags::is_linkage_flag(_lookahead)) {
        v->set_raw_field("linkage", _lookahead);
        jump_ahead();
    }
}

void IRParser::set_visibility(Value *v) {
    get_lookahead();
    if (IRFlags::is_visibility_flag(_lookahead)) {
        v->set_raw_field("visibility", _lookahead);
        jump_ahead();
    }
}

void IRParser::set_dll_storage_class(Value *v) {
    get_lookahead();
    if (IRFlags::is_dll_storage_class_flag(_lookahead)) {
        v->set_raw_field("visibility", _lookahead);
        jump_ahead();
    }
}

void IRParser::set_param_attrs(Value *v) {
    get_lookahead();
    string flag = "param-attrs";
    while (IRFlags::is_param_attr_flag(_lookahead)) {
        if (!v->has_raw_field(flag)) {
            v->set_raw_field(flag, _lookahead);
        }
        else {
            v->set_raw_field(flag, v->get_raw_field(flag) + ' ' + _lookahead);
        }

        jump_ahead();
        get_lookahead();
    }
}

void IRParser::set_ret_attrs(Value *v) {
    // This assert is in the document, but not in practice
    //assert(_word == "zeroext" || _word == "signext" || _word == "inreg" || _word == "noalias" &&
    //          "Only ‘zeroext‘, ‘signext‘, and ‘inreg‘ attributes are valid here for return type");

    get_lookahead();
    string flag = "ret-attrs";
    while (IRFlags::is_param_attr_flag(_lookahead)) {
        if (!v->has_raw_field(flag)) {
            v->set_raw_field(flag, _lookahead);
        }
        else {
            v->set_raw_field(flag, v->get_raw_field(flag) + ' ' + _lookahead);
        }

        jump_ahead();
        get_lookahead();
    }
}

/* todo: more to add */
