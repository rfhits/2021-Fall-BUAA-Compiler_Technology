#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_LEXER_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_LEXER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include "Token.h"



class Lexer {

public:
    char ch;
    string token; // while analyzing, read a token
    string source;
    int pos = 0; // in source_code[0]
    int line_no = 1; // no at this line
    int col_no = 1;
    static map<string, string> token_to_reserved_type;
    static map<string, string> token_to_operation_type;


    //
    explicit Lexer(const string& source_code_path) {
        ifstream source_file;
        source_file.open(source_code_path.c_str(), std::ios::in);
        if (!source_file.is_open()) {
            cout << "not read";
        }
        stringstream buffer;

        buffer << source_file.rdbuf() ;
        source = buffer.str()+ "\n";
        if (source.empty()) {
            cout<< "empty file" << endl;
        }
        source_file.close();
    };
public:
    int get_char();
    Token get_token();
    void pre_treat();
    void retract();
};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_LEXER_H
