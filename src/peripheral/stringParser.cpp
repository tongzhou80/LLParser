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
    _fail = false;
}

void StringParser::set_text(string & text) {
    _text = text;
    _intext_pos = 0;
    _char = _text[_intext_pos];
    _eol = false;
    _fail = false;
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

bool StringParser::match(const string& s) {
    for (int i = 0; i < s.size(); ++i) {
        if (!_eol && s[i] == _char) {
            inc_intext_pos();
            continue;
        }
        else {
            return false;
        }
    }
    return true;
}

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
    bool fail = _fail;

    get_word(delim, append_delim, skip_delim, skip_ws);
    _lookahead = _word;
    _aheadpos = _intext_pos;

    /* recover */
    _word = word;
    _intext_pos = intext_pos;
    _char = chr;
    _eol = eol;
    _fail = fail;
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

StringParser* StringParser::get_word(char delim, bool append_delim, bool skip_delim, bool skip_whitespace) {
    range_check();

    _word.clear();

    if (skip_whitespace) {
        skip_ws();
    }

    if (!_eol) {
        int startp = _intext_pos;
        int len = 0;
        while (!_eol && _text[_intext_pos] != delim) {
            inc_intext_pos();
            len++;
        }

        _word = _text.substr(startp, len);
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

    if (_word.empty()) {
        _fail = true;  // mimic std::getline()
    }

    return this;
}

StringParser::operator bool() const {
    return !_fail;
}

StringParser* StringParser::get_word_of(string delims, bool append_delim, bool skip_delim, bool skip_whitespace) {
    range_check();

    _word.clear();
    if (skip_whitespace) {
        skip_ws();
    }

    if (!_eol) {
        int startp = _intext_pos;
        int len = 0;
        //while (_text[_intext_pos] != delim) {
        while (delims.find(_text[_intext_pos]) == delims.npos) {
            inc_intext_pos();
            len++;
            if (_eol) {
                break;
            }
        }

        char delim = delims[delims.find(_text[_intext_pos])];
        _word = _text.substr(startp, len);

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

    if (_word.empty()) {
        _fail = true;
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
    parser_assert(_intext_pos < _text.size(), text(), "intext pointer out of range");
    parser_assert(_char == _text[_intext_pos], text(), "_char: %c, _text[_intext_pos]: %c", _char, _text[_intext_pos]);
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
    parser_assert(_aheadpos >= _intext_pos, text(), "Expect to jump forward");
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
        guarantee(0, "Bad scope start with %c", left);
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
    _fail = false;
}