//
// Created by tlaber on 6/14/17.
//

#include "stringParser.h"

StringParser::StringParser() {
    MAX_VALUE_LEN = 1024;
    _aheadpos = 0;
    _intext_pos = 0;
    _char = _text[_intext_pos];
    _eol = false;
}

void StringParser::set_text(string & text) {
    _text = text;
    _intext_pos = 0;
    _char = _text[_intext_pos];
    _eol = false;
}

/**@brief This function won't set _eol in any case.
 * In other words, the argument can't be an out-of-range position
 *
 * @param p
 */
void StringParser::set_intext_pos(int p) {
    guarantee(_intext_pos < _text.size() && _intext_pos > -1, "range check");
    _intext_pos = p;
    _char = _text[_intext_pos];
}

void StringParser::match(char c, bool skip_whitspace) {
    if (skip_whitspace)
        skip_ws();

    parser_assert(c == _char, "match |%c| but get |%c|", c, _char);
    inc_intext_pos();
}

void StringParser::match(const string& s, bool skip_whitspace) {
    if (skip_whitspace) skip_ws();

    int startp = _intext_pos;
    int len = 0;
    for (int i = 0; i < s.size(); ++i) {
        if (!_eol && s[i] == _char) {
            inc_intext_pos();
            len++;
            continue;
        }
        else {
            parser_assert(0, "match |%s| but get |%s|", s.c_str(), text().substr(startp, len).c_str());
        }
    }
}

/**@brief Similar to match(), but may not find a match and return false
 *
 * @param s
 * @param skip_whitspace
 */
bool StringParser::try_match(const string& s, bool skip_whitspace) {
    if (skip_whitspace) skip_ws();

    int startp = _intext_pos;
    if (text().find(s, startp) == startp) {
        inc_intext_pos(s.size());
        return true;
    }
    else {
        return false;
    }
}

//void StringParser::match(const string& s, bool skip_whitspace) {
//    if (skip_whitspace) skip_ws();
//
//    int startp = _intext_pos;
//    int matchp = text().find(s, startp);
//    parser_assert(matchp = startp, "match |%s| failed", s.c_str());
//    inc_intext_pos(s.size());
//}

/**@brief Similar to get_word(), except that the state of parser does not change. The result is stored in _lookahead.
 *
 * Use jump_ahead() to actually set the states. get_lookahead() + jump_ahead() === get_word().
 *
 * @param delim 
 * @param append_delim
 * @param skip_delim
 * @param skip_ws
 */
void StringParser::get_lookahead(char delim, bool append_delim, bool skip_delim, bool skip_ws) {
    /* save state */
    string word = _word;
    int intext_pos = _intext_pos;
    char chr = _char;
    bool eol = _eol;

    get_word(delim, append_delim, skip_delim, skip_ws);
    _lookahead = _word;
    _aheadpos = _intext_pos;

    /* recover */
    _word = word;
    _intext_pos = intext_pos;
    _char = chr;
    _eol = eol;
}

/**@brief Similar to get_word_of(), except that the state of parser does not change. The result is stored in _lookahead.
 *
 * @param delims
 * @param append_delim
 * @param skip_delim
 * @param skip_ws
 */
void StringParser::get_lookahead_of(string delims, bool append_delim, bool skip_delim, bool skip_ws) {
    /* save state */
    string word = _word;
    int intext_pos = _intext_pos;
    char chr = _char;
    bool eol = _eol;

    get_word_of(delims, append_delim, skip_delim, skip_ws);
    _lookahead = _word;
    _aheadpos = _intext_pos;

    /* recover */
    _word = word;
    _intext_pos = intext_pos;
    _char = chr;
    _eol = eol;
}
//
//void StringParser::get_lookahead(char delim, bool append_delim, bool skip_delim, bool skip_whitspace) {
//    guarantee(_intext_pos < _text.size(), "intext pointer out of range");
//    guarantee(_char == _text[_intext_pos], "_char: %c, _text[_intext_pos]: %c", _char, _text[_intext_pos]);
//
//    if (skip_whitspace) {
//        skip_ws();
//    }
//
//    if (_eol) {
//        return;
//    }
//
//    int startp = _intext_pos;
//    _aheadpos = startp;
//    while (_aheadpos < _text.size() && _text[_aheadpos] != delim) {
//        _aheadpos++;
//    }
//
//    _lookahead = _text.substr(startp, _aheadpos-startp);
//    if (append_delim) {
//        _lookahead += delim;
//        zpl("append %c", delim);
//    }
//
//    if (skip_delim) {
//        if (!_eol) {
//            inc_intext_pos();
//        }
//    }
//}

int StringParser::parse_integer(bool skip_whitespace) {
    range_check();

    if (skip_whitespace) {
        skip_ws();
    }

    if (!_eol) {
        int startp = _intext_pos;
        int len = 0;
        while (!_eol && std::isdigit(_char)) {
            inc_intext_pos();
            len++;
        }

        return std::stoi(_text.substr(startp, len));
    }
    else {
        guarantee(0, "end of line");
    }
}
//
//StringParser* StringParser::get_word(char delim, bool append_delim, bool skip_delim, bool skip_whitespace) {
//    range_check();
//
//    _word.clear();
//
//    if (skip_whitespace) {
//        skip_ws();
//    }
//
//    if (!_eol) {
//        int startp = _intext_pos;
//        int len = 0;
//        while (!_eol && _text[_intext_pos] != delim) {
//            inc_intext_pos();
//            len++;
//        }
//
//        _word = _text.substr(startp, len);
//        if (append_delim) {
//            _word += delim;
//            zpl("append %c", delim);
//        }
//
//        if (skip_delim) {
//            if (!_eol) {
//                inc_intext_pos();
//            }
//        }
//    }
//
//    if (_word.empty()) {
//        _fail = true;  // mimic std::getline()
//    }
//
//    return this;
//}
//

StringParser* StringParser::get_word(char delim, bool append_delim, bool skip_delim, bool skip_whitespace) {
    range_check();

    _word.clear();

    if (skip_whitespace) {
        skip_ws();
    }

    if (!_eol) {
        int startp = intext_pos();
        int endp = text().find(delim, startp);

        if (endp == string::npos) {
            endp = text().size();
        }

        inc_intext_pos(endp - startp);
        _word = _text.substr(startp, endp - startp);
        //guarantee(endp != string::npos, "Char %c not found in subtext %s", delim, text().substr(intext_pos()).c_str());


        if (append_delim) {
            _word += delim;
            zpl("append %c", delim);
        }

        if (skip_delim) {
            if (!_eol) {
                inc_intext_pos();
            }
        }
    }

    return this;
}

StringParser* StringParser::get_word_of(string delims, bool append_delim, bool skip_delim, bool skip_whitespace) {
    range_check();

    _word.clear();
    if (skip_whitespace) {
        skip_ws();
    }

    if (!_eol) {
        int startp = intext_pos();
        int endp = text().find_first_of(delims, startp);

        if (endp == string::npos) {
            endp = text().size();
        }

        inc_intext_pos(endp - startp);
        _word = text().substr(startp, endp - startp);
        char delim = text()[endp];

        if (append_delim) {
            _word += delim;
            zpl("append %c", delim);
        }

        if (skip_delim) {
            if (!_eol) {
                inc_intext_pos();
            }
        }
    }

    return this;
}

//void StringParser::get_char(bool skip_ws) {
//    range_check();
//
//    if (skip_ws) {
//        while (_text[_intext_pos] == ' ' || _text[_intext_pos] == '\t' || _text[_intext_pos] == '\n') {
//            inc_intext_pos();
//            if (_eol) {
//                break;
//            }
//        }
//    }
//
//    if (_eol) {
//        _char = '\0';
//    }
//    else {
//        _char = _text[_intext_pos];
//    }
//}

void StringParser::range_check() {
#ifdef LLDEBUG
    parser_assert(_intext_pos < _text.size(), "intext pointer out of range, pos: %d, size: %d", _intext_pos, _text.size());
    parser_assert(_char == _text[_intext_pos], "_char: %c, _text[_intext_pos]: %c", _char, _text[_intext_pos]);
#endif
}

/**@brief Increment the position in text. _intext_pos and _char will change. _eol will possibly be set.
 *
 * When increment out of range, the position pointer stays out of range and eol will be set
 *
 * @param steps
 * @return
 */
bool StringParser::inc_intext_pos(int steps) {
    range_check();
    _intext_pos += steps;
    if (_intext_pos > _text.size()-1) {
        _eol = true;  // _intext_pos is no longer valid
        return false;
    }
    else {
        _char = _text[_intext_pos];
        return true;
    }
}

/**@brief Jump to the position of lookahead. All relevant parser states will be set.
 *
 */
void StringParser::jump_ahead() {
    parser_assert(_aheadpos >= _intext_pos, "Expect to jump forward");
    if (_aheadpos < _text.size()) {
        inc_intext_pos(_aheadpos-_intext_pos);
    }
    else {
        _eol = true;
    }
}

/**@brief When current char is '(', '{', '[' or '<', jump to the end of the scope and return the skipped part
 *
 * One assumption of this function is that the left mark (like '[') must be different from the right mark.
 * @return
 */
string StringParser::jump_to_end_of_scope() {
    int num = 1;
    char left = _char;
    char right = ' ';
    if (left == '(') {
        right = ')';
    }
    else if (left == '{') {
        right = '}';
    }
    else if (left == '[') {
        right = ']';
    }
    else if (left == '<') {
        right = '>';
    }
    else {
        parser_assert(0, "Bad scope start with %c", left);
    }

    string text;
    while (num != 0) {
        text += _char;
        inc_intext_pos();
        if (_char == left) {
            num++;
        }
        else if (_char == right) {
            num--;
        }
    }
    text += right;

    guarantee(_char == right, " ");
    inc_intext_pos();

    return text;
}

void StringParser::skip_ws() {
    while (!_eol && _text[_intext_pos] == ' ' || _text[_intext_pos] == '\t' || _text[_intext_pos] == '\n') {
        inc_intext_pos();
    }
}

void StringParser::reset_parser() {
    _text.clear();
    _intext_pos = 0;
    _word.clear();
    _lookahead.clear();
    _aheadpos = 0;
    _char = 0;
    _eol = false;
}