//
// Created by WYSJ6174 on 2021/10/3.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H

#include <iostream>
#include <fstream>
#include "utils.h"
#include "ErrorHandler.h"
#include "Lexer.h"
#include "SymbolTable.h"
#include "Intermediate.h"

enum class ErrorType {
    A, B, C, D,
    E, F, G, H,
    I, J, K, L,
    M,
    REDEF, // b, redefinition
    UNDECL, //c, undeclared variable
    ARG_NO_MISMATCH, // d
    ARG_TYPE_MISMATCH, // e
    RET_TYPE_MISMATCH, // f
    MISSING_RET, // g
    CHANGE_CONST, // h
    EXPECTED_SEMICN, // i
    EXPECTED_PARENT, // j, expected  )
    EXPECTED_BRACK, // k, expected ]
    PRINT_NO_MISMATCH, // l
    NOT_IN_LOOP // m
};


class Parser {
private:
    Lexer& lexer_;
    ErrorHandler& error_handler_;
    Intermediate& intermediate_;
    bool print_mode_;
    int pos_ = 0; // the number of the tokens been processed
    Token token_ = Token(TYPE_UNDEFINED);
    TypeCode type_code_; // the token's type_code
    std::ostream& out_; // output stream
    std::vector<Token> read_tokens_; // the tokens have been read
    std::vector<std::string> out_strings_;

    std::string name_;
    int dims_ = 0, dim0_size_=0, dim1_size_=0;
    std::string cur_func_name_;
    int cur_level_ = 0;
    int var_size_;
    bool has_ret_;
    SymbolTable& symbol_table_;


    void next_sym(); // put a token into read_tokens
    void retract();

    void output(const std::string& str);
    void handle_error(const std::string& msg);
    void handle_error(ErrorType error_type);



    void Decl();
    void ConstDecl();
    void ConstDef();
    std::pair<DataType, std::string> ConstExp();
    std::pair<DataType, std::string> AddExp();
    std::pair<DataType, std::string> MulExp();
    std::pair<DataType, std::string> UnaryExp();
    std::pair<DataType, std::string> PrimaryExp();
    std::pair<DataType, std::string> Exp();
    std::pair<DataType, std::string> LVal();
    int Number();
    int IntConst();
    std::vector<std::string> FuncRParams(const std::string& func_name, int param_num);
    int UnaryOp();
    std::pair<DataType, std::string> ConstInitVal();

    void VarDecl();
    void VarDef();
    std::pair<DataType, std::string> InitVal();


    void FuncDef();
    DataType FuncType();
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
    Parser(SymbolTable& symbol_table, Lexer& lexer, ErrorHandler& error_handler, Intermediate& intermediate,
           bool print_mode, std::ofstream& out);
    void Program();

};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H
