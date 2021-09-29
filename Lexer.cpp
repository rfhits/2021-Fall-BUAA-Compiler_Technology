//
// Created by WYSJ6174 on 2021/9/24.
//

#include "Lexer.h"
#include "Error.h"


Lexer::Lexer(string &&source) {
    source_ = source;

}

// replace comment to ' '
void Lexer::uncomment() {

    // Error treat, may illegal comment
    for (int i = 0; i<source_.length(); ) {
        if (source_[i] == '"') {
            do {
                i += 1;
            } while(source_[i] != '"');
            i+=1;
        }
        else if (source_[i] == '/' && source_[i+1] == '/') {
            source_[i] = source_[i+1] = ' ';
            i += 2; // the char after "//"
            while (source_[i] != '\n') {
                source_[i] = ' ';
                i+=1;
            }
            // now source[i] is \n
            i+=1;
        }
        else if (source_[i] == '/' && source_[i+1] == '*') {
            source_[i] = source_[i+1] = ' ';
            i += 2; // at the char after "/*"
            while (source_[i] != '*' || source_[i+1] != '/') {
                source_[i] = (source_[i] == '\n')? '\n':' ';
                i+=1;
            }
            // now source[i] is at "*/"
            source_[i] = source_[i+1] = ' ';
            i += 2;
        }
        else {
            i += 1;
        }
    }
}

// read a char from source and store it to ch_
// maintain the line_no_, col_num, pos.
int Lexer::get_char() {
    if (pos_ >= source_.length()) {
        cerr << "position out of source code" << endl;
    }
    if (ch_ == '\n') {
        line_no_+= 1;
    }
    prev_char_ = ch_;
    ch_ = source_[pos_++];
    return 0;
}

void Lexer::retract() {
    if (prev_char_ == '\n') {
        line_no_-= 1;
    }
    pos_ -= 1;
    ch_ = source_[pos_-1];
    if (pos_ >= 2) {
        prev_char_ = source_[pos_ - 2];
    }

}

// put a word into Lexer::str_token_
Token Lexer::get_token() {
    Token r_token(TypeCode::TYPE_UNDEFINED);
    if (pos_ == source_.length()) { // at the end
        r_token.set_type_code(TypeCode::TYPE_EOF);
        return r_token;
    }

    // read till there is a none-blank char in ch_
    get_char();

    while (isspace(ch_) && pos_ < source_.length()) {
        get_char();
    }

    if (isspace(ch_)) {
        r_token.set_type_code(TypeCode::TYPE_EOF);
        return r_token;
    }

    str_token_.clear(); // r_token::str_value
    TypeCode type_code = TypeCode::TYPE_UNDEFINED;


    // identifier begins with alpha or underline
    if (isalpha(ch_) || ch_ == '_') {
        do {
            str_token_ += ch_;
            get_char();
        } while(isalnum(ch_) || ch_ =='_');
        retract();
        auto iter = reserved_word2type_code.find(str_token_);
        if (iter != reserved_word2type_code.end()) {
            type_code = iter->second;
        } else {
            type_code = TypeCode::IDENFR;
        }
    } else if (isdigit(ch_)) {
        do {
            str_token_ += ch_;
            get_char();
        } while (isdigit(ch_));
        retract();
        type_code = TypeCode::INTCON;
    } else if (ch_ == '\"') {
        do {
            str_token_ += ch_;
            get_char();
        } while (ch_ != '\"');
        str_token_ += ch_;
        type_code = TypeCode::STRCON;
    }
    // we will begin pre-read here
    // !, !=; &&; ||, <, <=; >, >=; =, ==;
    else if (ch_ == '!') {
        get_char();
        if (ch_ != '=') {
            retract();
            str_token_ = "!";
            type_code = TypeCode::NOT;
        } else {
            str_token_ = "!=";
            type_code = TypeCode::NEQ;
        }
    }
    else if (ch_ == '&') {
        get_char();
        if (ch_ == '&') {
            str_token_ = "&&";
            type_code = TypeCode::AND;
        } else {
            Error e("expect & after &");
            e.alert();
        }
    }
    else if (ch_ == '|') {
        get_char();
        if (ch_ == '|') {
            str_token_ = "||";
            type_code = TypeCode::OR;
        } else {
            Error e("expect | after |");
            e.alert();
        }
    }
    else if (ch_ == '<') {
        get_char();
        if (ch_ != '=') {
            retract();
            str_token_ = "<";
            type_code = TypeCode::LSS;
        } else {
            str_token_ = "<=";
            type_code = TypeCode::LEQ;
        }
    }
    else if (ch_ == '>') {
        get_char();
        if (ch_ != '=') {
            retract();
            str_token_ = ">";
            type_code = TypeCode::GRE;
        }
        else {
            str_token_ = ">=";
            type_code = TypeCode::GEQ;
        }
    }
    else if (ch_ == '=') {
        get_char();
        if (ch_ != '=') {
            retract();
            str_token_ = "=";
            type_code = TypeCode::ASSIGN;
        }
        else {
            str_token_ = "==";
            type_code = TypeCode::EQL;
        }
    }
    else if (ch_ == '+' || ch_ == '-' || ch_ == '*' || ch_ == '/' || ch_ == '%'
        || ch_ == ';' || ch_ == ',' || ch_ == '(' || ch_ == ')' || ch_ == '['
        || ch_ == ']' || ch_ == '{' || ch_ == '}') {
        auto iter = char2type_code.find(string(1,ch_));
        if (iter != char2type_code.end()) {
            str_token_ = string(1, ch_);
            type_code = iter->second;
        }
        else {
            Error e("str_token_ to reserved operations match error");
            e.alert();
        }
    }
    else {
        Error e("ch_ can't match any char ");
        e.alert();
    }
    r_token.set_str_value(str_token_);
    r_token.set_type_code(type_code);
    return r_token;
}
