//
// Created by tlaber on 6/14/17.
//

#ifndef LLPARSER_STRINGPARSER_H
#define LLPARSER_STRINGPARSER_H

#include "utilities/macros.h"

class StringParser {
protected:
    string _text;
    int _intext_pos;
    string _word;
    string _lookahead;
    int _aheadpos;
    char _char;
    bool _eol;
    bool _fail;
public:
    int MAX_VALUE_LEN;

    StringParser();

    string text()                                        { return _text; }
    void set_text(string& );
    void set_intext_pos(int p);

    string jump_to_end_of_scope();
    void skip_ws();

    StringParser* get_word(char delim=' ', bool append_delim=0, bool skip_delim=1, bool skip_ws=1);
    void get_lookahead(char delim=' ', bool append_delim=0, bool skip_delim=1, bool skip_ws=1);
    void jump_ahead();
    StringParser* get_word_of(string delims,  bool append_delim=0, bool skip_delim=1, bool skip_ws=1);

    bool match(string s);

    void get_char(bool skip_ws=1);
    bool inc_intext_pos(int steps=1);
    void range_check();

    virtual void reset_parser();

    explicit operator bool() const;
};

#endif //LLPARSER_STRINGPARSER_H
