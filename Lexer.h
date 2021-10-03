#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_LEXER_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_LEXER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "Token.h"
#include "ErrorHandler.h"

class Lexer {

private:
    char ch_;
    char prev_char_ = -1;
    string str_token_; // while analyzing, read a token
    string source_;
    int pos_ = 0; // in source_code[0]
    int line_no_ = 1; // now at this line
    bool print_mode_ = false;
    std::ofstream& out_;
    ErrorHandler& error_handler_;


public:
    explicit Lexer(string&& source, ErrorHandler& error_handler,bool print_mode, std::ofstream& out);
    int get_char();
    Token get_token();
    void retract();
    void uncomment();
    void handle_error(const string& msg);
};

#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_LEXER_H
