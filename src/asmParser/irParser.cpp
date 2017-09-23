//
// Created by tzhou on 9/17/17.
//

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
        parser_assert(!p1.empty(), text(), "The identifier does not start with [-a-zA-Z$._]: %c", _char);
        string p2 = match_xalphanumeric();

        return p1 + p2;
    }
}


string IRParser::parse_basic_type() {
    skip_ws();

    string fulltype;
    switch (_char) {
        case 'v': {
            fulltype = "void";
            guarantee(match(fulltype), " ");
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
            guarantee(match(fulltype), " ");
            break;
        }
        case 'd': {  // double
            fulltype = "double";
            guarantee(match(fulltype), " ");
            break;
        }
        case 'h': {  // half
            fulltype = "half";
            guarantee(match(fulltype), " ");
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
            parser_assert(0, text(), "bad type!");
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

    return ty;
}

/**@brief parse structs of the following form: (not parsing the * at the end, if any)
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
    syntax_check(_char == '%');

    string name = match_identifier();
    parser_assert(name[0] == 's' || name[0] == 'c' || name[0] == 'u' || name[0] == '"', text(), "sanity");
    return '%' + name;
}