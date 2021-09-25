//
// Created by WYSJ6174 on 2021/9/24.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_TOKEN_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_TOKEN_H
#include <iostream>
#include <utility>
#include <vector>

#define INVALID_TYPE "invalid"
#define INVALID_STRING_VALUE "invalid"
#define INVALID_INTEGER_VALUE -1

#define IDENFR "IDENFR"


using namespace std;

class Token {
public:
    string type; //类别码
    string str_value; // if token is a string， store value
    int int_value;

    int line_no; // token appears in these line
    int col_no;

    Token(string type) {
        this->type = std::move(type);
    }

    Token(string type, string str_value, int line_no, int col_no) {
        this->type = std::move(type);
        this->str_value = move(str_value),
        this->line_no = line_no;
        this->col_no = col_no;
    };

    Token(string type, int int_value, int line_no, int col_no) {
        this->type = std::move(type);
        this->int_value = int_value,
        this->int_value = line_no;
        this->col_no = col_no;
    };


};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_TOKEN_H
