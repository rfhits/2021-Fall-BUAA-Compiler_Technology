//
// Created by WYSJ6174 on 2021/10/3.
//

#include <commctrl.h>
#include "Parser.h"

Parser::Parser(Lexer &lexer, ErrorHandler &error_handler, bool print_mode, std::ofstream &out) :
        lexer_(lexer), error_handler_(error_handler), print_mode_(print_mode), out_(out) {}


// if pos is behind read_tokens
// then read from it
// else use get_token() to get a token from src
void Parser::next_sym() {

    // may retract
    if (pos_ < read_tokens_.size()) {
        token_ = read_tokens_[pos_];
    } else {
        token_ = lexer_.get_token();
        read_tokens_.push_back(token_);
    }
    type_code_ = token_.get_type_code();
    pos_ += 1;
    out_strings_.push_back(token_.to_string());
}

// change pos and output_strings
void Parser::retract() {
    pos_ -= 1;
    token_ = read_tokens_[pos_ - 1];
    type_code_ = token_.get_type_code();

    // erase the output until meets <V_n>
    for (unsigned int i = out_strings_.size() - 1; i >= 0; i--) {
        if (out_strings_[i][0] != '<') {
            out_strings_.erase(out_strings_.begin() + i);
            break;
        }
    }
}


void Parser::handle_error(const std::string& msg) {
    error_handler_.log_error(token_.get_line_no(), msg);
}

// CompUnit::=
void Parser::Program() {
    next_sym();
    // three cond: const / int / void
    // three branches: Decl, Func, Main
    while (true) {
        if (type_code_ == TypeCode::CONSTTK) {
            Decl();
            next_sym();
        } else if (type_code_ == TypeCode::VOIDTK) {
            break;
        }
        // int a = / int a[ / int a( / int main(
        else if (type_code_ == TypeCode::INTTK) {
            next_sym(); // int a
            next_sym(); // int a *
            if (type_code_ == TypeCode::ASSIGN ||
                type_code_ == TypeCode::LBRACK) {
                retract();
                retract();
                Decl();
                next_sym();
            } else {
                retract();
                retract();
                break;
            }
        } else {
            handle_error("const or int or void at {Decl}");
        }
    }

    // {FuncDef}
    while (true) {
        if (type_code_ == TypeCode::VOIDTK) {
            FuncDef();
            next_sym();
        }
        // int a( / int main(
        else if (type_code_ == TypeCode::INTTK) {
            next_sym();
            if (type_code_ == TypeCode::IDENFR) {
                retract();
                FuncDef();
                next_sym();
            }
            else {
                break;
            }
        } else {
//            handle_error("funcdef error");
            break;
        }
    }

    // MainDef
    next_sym();
    if (type_code_ == TypeCode::MAINTK){
        retract();
        MainFuncDef();
    } else {
        handle_error("expect main");
    }

    out_strings_.emplace_back("CompUnit");
    // TODO
    // OUTPUT string in vec
}

void Parser::FuncDef() {

}

void Parser::FuncFParams() {

}

void Parser::FuncFParam() {

}

void Parser::Block() {

}

void Parser::BlockItem() {

}

void Parser::Stmt() {

}
