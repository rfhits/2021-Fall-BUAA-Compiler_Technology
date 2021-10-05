//
// Created by WYSJ6174 on 2021/10/3.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H

#include <iostream>
#include <fstream>
#include "ErrorHandler.h"
#include "Lexer.h"

class Parser {
private:
    Lexer& lexer_;
    ErrorHandler& error_handler_;
    bool print_mode_;
    int pos_ = 0; // have process pos tokens
    Token token_ = Token(TYPE_UNDEFINED);
    TypeCode type_code_; // the token's type_code
    std::ostream& out_; // output stream
    std::vector<Token> read_tokens_; // the tokens have been read
    std::vector<string> out_strings_;


    void next_sym(); // put a token into read_tokens
    void retract();

    void output(const std::string& str);
    void handle_error(const std::string& msg);


    void Decl();
    void ConstDecl();
    void ConstDef();
    void ConstExp();
    void AddExp();
    void MulExp();
    void UnaryExp();
    void PrimaryExp();
    void Exp();
    void LVal();
    void Number();
    void IntConst();
    void FuncRParams();
    void UnaryOp();
    void ConstInitVal();

    void VarDecl();
    void VarDef();
    void InitVal();


    void FuncDef();
    void FuncType();
    void FuncFParams();
    void FuncFParam();
    void Block();
    void BlockItem();
    void Stmt();
    void AssignStmt();
    void IfStmt();
    void Cond();
    void LOrExp();
    void LAndExp();
    void EqExp();
    void RelExp();
    void WhileStmt();
    void ReturnStmt();
    void ReadStmt();
    void WriteStmt();

    void MainFuncDef();

public:
    Parser(Lexer& lexer, ErrorHandler& error_handler, bool print_mode, ofstream& out);
    void Program();



};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H
