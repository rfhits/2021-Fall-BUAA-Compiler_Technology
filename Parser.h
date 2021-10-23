//
// Created by WYSJ6174 on 2021/10/3.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H

#include <iostream>
#include <fstream>
#include <set>
#include "utils.h"
#include "ErrorHandler.h"
#include "Lexer.h"
#include "SymbolTable.h"
#include "Intermediate.h"

enum class ErrorType {
    ILLEGAL_CHAR, // a, illegal char in format string
    REDEF, // b, redefinition
    UNDECL, //c, undeclared variable
    ARG_NO_MISMATCH, // d, argument number mismatch
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


const std::unordered_map<ErrorType, std::string> error_type_to_alpha = {
        {ErrorType::ILLEGAL_CHAR, "a"},
        {ErrorType::REDEF, "b"},
        {ErrorType::UNDECL, "c"},
        {ErrorType::ARG_NO_MISMATCH, "d"},
        {ErrorType::ARG_TYPE_MISMATCH, "e"},
        {ErrorType::RET_TYPE_MISMATCH, "f"},
        {ErrorType::MISSING_RET, "g"},
        {ErrorType::CHANGE_CONST, "h"},
        {ErrorType::EXPECTED_SEMICN, "i"},
        {ErrorType::EXPECTED_PARENT, "j"},
        {ErrorType::EXPECTED_BRACK, "k"},
        {ErrorType::PRINT_NO_MISMATCH, "l"},
        {ErrorType::NOT_IN_LOOP, "m"},
};


enum class BlockItemType {
    INVALID,
    VAR_DECL, CONST_DECL,
    ASSIGN_STMT, EXP_STMT, EMPTY_STMT, BLOCK_STMT,
    IF_STMT, WHILE_STMT, BREAK_STMT, CONTINUE_STMT,
    RETURN_STMT, READ_STMT, WRITE_STMT,
};

class Parser {
private:
    Lexer& lexer_;
    ErrorHandler& error_handler_;
    Intermediate& intermediate_;
    bool print_mode_;
    int pos_ = 0; // the number of the tokens been processed
    Token token_ = Token(TypeCode::TYPE_UNDEFINED);
    TypeCode type_code_; // the token's type_code
    std::ostream& out_; // output stream
    std::vector<Token> read_tokens_; // the tokens have been read
    std::vector<std::string> out_strings_;
    std::vector<std::pair<int, ErrorType>> errors;
    void output_error();

    std::string cur_func_name_;
    int cur_level_ = 0;
    bool has_ret_stmt_ = false; // the parsing function has the return statement
    std::vector<bool> loop_stack_ = {false}; // the last bool means whether in loop statement
    int redef_func_no_ = 0; // redefinition occurs, self plus, give the func a nickname
    int undef_name_no_ = 0;
    std::string name_;
    int dims_ = 0, dim0_size_=0, dim1_size_=0;
    int var_size_;
    SymbolTable& symbol_table_;

    std::set<TypeCode> first_exp = { // FIRST(<Exp>)
            TypeCode::PLUS, TypeCode::MINU, TypeCode::NOT,
            TypeCode::IDENFR, TypeCode::LPARENT, TypeCode::INTCON
    };


    void next_sym(); // put a token into read_tokens
    int get_prev_sym_line_no();
    void reset_sym();
    void retract();

    void output(const std::string& str);
    void handle_error(const std::string& msg);
    void add_error(int line_no, ErrorType error_type);
    void add_error(ErrorType error_type);


    BlockItemType Decl();
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
    std::vector<std::pair<DataType, std::string>> FuncRParams();
    int UnaryOp();
    std::pair<DataType, std::string> ConstInitVal();

    void VarDecl();
    void VarDef();
    std::pair<DataType, std::string> InitVal();


    void FuncDef();
    DataType FuncType();
    int FuncFParams();
    void FuncFParam(int param_ord);
    std::vector<BlockItemType> Block();
    BlockItemType BlockItem();
    BlockItemType Stmt();
    void AssignStmt(int line_no, std::string assigned_var_name, std::pair<DataType, std::string> lval_ret);
    void IfStmt();
    std::pair<DataType, std::string> Cond();
    std::pair<DataType, std::string> LOrExp();
    std::pair<DataType, std::string> LAndExp();
    std::pair<DataType, std::string> EqExp();
    std::pair<DataType, std::string> RelExp();
    void WhileStmt();
    void ReturnStmt();
    void ReadStmt(const std::string& assigned_var_name, std::pair<DataType, std::string> lval_ret);
    void WriteStmt();
    std::pair<int, std::vector<std::string>> FormatString();

    void MainFuncDef();

public:
    Parser(SymbolTable& symbol_table, Lexer& lexer, ErrorHandler& error_handler, Intermediate& intermediate,
           bool print_mode, std::ofstream& out);
    void Program();

};
#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_PARSER_H
