
#include "Token.h"

#include <utility>

Token::Token(TypeCode type_code): type_code_(type_code){
}

Token::Token(TypeCode type_code, string str_value, int line_no, int col_no):
        type_code_(type_code), str_value_(std::move(str_value)), line_no_(line_no){

}

Token::Token(TypeCode type_code, int int_value, int line_no, int col_no):
    type_code_(type_code), int_value_(int_value), line_no_(line_no){

}

TypeCode Token::get_type_code() {
    return this->type_code_;
}

string Token::get_str_value() {
    return this->str_value_;
}

int Token::get_int_value() {
    return this->int_value_;
}

string Token::get_type_name() {
    // TODO
    // use the type_code to get its name to output
    return "hello world";
}

void Token::set_type_code(TypeCode type_code) {
    this->type_code_ = type_code;
}

void Token::set_str_value(string &str_value) {
    str_value_ = std::move(str_value);
}

string Token::to_string() {
    auto iter = type_code2str.find(type_code_);
    string s = iter->second;

    return std::__cxx11::string();
}





