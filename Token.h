//
// Created by WYSJ6174 on 2021/9/24.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_TOKEN_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_TOKEN_H
#include <iostream>
#include <utility>
#include <vector>
#include <map>

//using namespace std;

enum TypeCode {
    TYPE_UNDEFINED, TYPE_EOF,
    IDENFR, INTCON, STRCON, MAINTK,CONSTTK,
    INTTK, BREAKTK, CONTINUETK, IFTK,
    ELSETK, NOT, AND, OR, WHILETK,
    GETINTTK, PRINTFTK, RETURNTK, PLUS,
    MINU, VOIDTK, MULT, DIV,
    MOD, LSS, LEQ, GRE,
    GEQ, EQL, NEQ, ASSIGN,
    SEMICN, COMMA, LPARENT,RPARENT,
    LBRACK, RBRACK,LBRACE, RBRACE
};

const std::map<std::string, TypeCode> reserved_word2type_code = {
        {"main", MAINTK},
        {"const", CONSTTK},
        {"int", INTTK},
        {"break", BREAKTK},
        {"continue", CONTINUETK},
        {"if", IFTK},
        {"else", ELSETK},
        {"while", WHILETK},
        {"getint", GETINTTK},
        {"printf", PRINTFTK},
        {"return", RETURNTK},
        {"void", VOIDTK},
};


const std::map<std::string, TypeCode> char2type_code = {
        {"+", TypeCode::PLUS},
        {"-", TypeCode::MINU},
        {"*", TypeCode::MULT},
        {"/", TypeCode::DIV},
        {"%", TypeCode::MOD},
        {";", TypeCode::SEMICN},
        {",", TypeCode::COMMA},
        {"(", TypeCode::LPARENT},
        {")", TypeCode::RPARENT},
        {"[", TypeCode::LBRACK},
        {"]", TypeCode::RBRACK},
        {"{", TypeCode::LBRACE},
        {"}", TypeCode::RBRACE}
};


const std::map<TypeCode, std::string> type_code2str = {
        {TypeCode::IDENFR, "IDENFR"},
        {TypeCode::INTCON, "INTCON"},
        {TypeCode::STRCON, "STRCON"},
        {TypeCode::MAINTK, "MAINTK"},
        {TypeCode::CONSTTK, "CONSTTK"},
        {TypeCode::INTTK, "INTTK"},
        {TypeCode::BREAKTK, "BREAKTK"},
        {TypeCode::CONTINUETK, "CONTINUETK"},
        {TypeCode::IFTK, "IFTK"},
        {TypeCode::ELSETK, "ELSETK"},
        {TypeCode::NOT, "NOT"},
        {TypeCode::AND, "AND"},
        {TypeCode::OR, "OR"},
        {TypeCode::WHILETK, "WHILETK"},
        {TypeCode::GETINTTK,"GETINTTK"},
        {TypeCode::PRINTFTK,"PRINTFTK"},
        {TypeCode::RETURNTK,"RETURNTK"},
        {TypeCode::PLUS, "PLUS"},
        {TypeCode::MINU, "MINU"},
        {TypeCode::VOIDTK, "VOIDTK"},
        {TypeCode::MULT, "MULT"},
        {TypeCode::DIV, "DIV"},
        {TypeCode::MOD, "MOD"},
        {TypeCode::LSS, "LSS"},
        {TypeCode::LEQ, "LEQ"},
        {TypeCode::GRE, "GRE"},
        {TypeCode::GEQ, "GEQ"},
        {TypeCode::EQL, "EQL"},
        {TypeCode::NEQ,"NEQ"},
        {TypeCode::ASSIGN, "ASSIGN"},
        {TypeCode::SEMICN, "SEMICN"},
        {TypeCode::COMMA, "COMMA"},
        {TypeCode::LPARENT, "LPARENT"},
        {TypeCode::RPARENT, "RPARENT"},
        {TypeCode::LBRACK, "LBRACK"},
        {TypeCode::RBRACK, "RBRACK"},
        {TypeCode::LBRACE, "LBRACE"},
        {TypeCode::RBRACE, "RBRACE"},
};

class Token {
private:
    TypeCode type_code_ = TYPE_UNDEFINED; //类别码

    // if token is a string, store value
    // identifier: the name
    std::string str_value_;


    int int_value_ ;

    int line_no_; // token appears in this line

public:

    Token(TypeCode type_code);

    Token(TypeCode type_code, std::string str_value, int line_no, int col_no);

    Token(TypeCode type_code, int int_value, int line_no, int col_no);

    TypeCode get_type_code();
    std::string get_str_value();
    int get_int_value();
    std::string get_type_name();
    int get_line_no() const {
        return line_no_;
    }
    void set_type_code(TypeCode type_code);
    void set_str_value(std::string& str_value);
    std::string to_string();

};

#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_TOKEN_H
