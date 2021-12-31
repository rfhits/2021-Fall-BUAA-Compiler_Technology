
#include "Token.h"

#include <utility>

Token::Token(TypeCode type_code): type_code_(type_code){
}

Token::Token(TypeCode type_code, std::string str_value, int line_no, int col_no):
        type_code_(type_code), str_value_(std::move(str_value)), line_no_(line_no){

}

Token::Token(TypeCode type_code, int int_value, int line_no, int col_no):
    type_code_(type_code), int_value_(int_value), line_no_(line_no){

}

TypeCode Token::get_type_code() {
    return this->type_code_;
}

std::string Token::get_str_value() {
    return this->str_value_;
}

int Token::get_int_value() {
    return this->int_value_;
}

std::string Token::get_type_name() {
    // TODO
    // use the type_code to get its name to output
    return "hello world";
}

void Token::set_type_code(TypeCode type_code) {
    this->type_code_ = type_code;
}

void Token::set_str_value(std::string &str_value) {
    str_value_ = std::move(str_value);
}

std::string Token::to_string() {
    auto iter = type_code2str.find(type_code_);
    std::string s = iter->second;
    s.push_back(' ');
    s += str_value_;
    return s;
}

void Token::set_line_no(int line_no) {
    this->line_no_ = line_no;
}





