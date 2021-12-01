//
// Created by WYSJ6174 on 2021/10/17.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H

#include <string>
#include "SymbolTable.h"

enum class IntermOp {
    ADD, SUB, MUL, DIV, MOD,

    AND, OR, NOT, // deprecate, I thought it was used for condition, then i wrong

    EQ, NEQ, LSS, LEQ, GRE, GEQ,

    GETINT, PRINT,

    INIT_ARR_PTR,
    ARR_SAVE, // save to array
    ARR_LOAD, // load from array

    LABEL,
    JUMP, BEQ, BNE,

    FUNC_BEGIN, FUNC_END,
    PREPARE_CALL, PUSH_VAL, PUSH_ARR, CALL,

    RET
};


const std::unordered_map<IntermOp, std::string> op_to_str = {
        {IntermOp::SUB, "SUB"},
        {IntermOp::ADD,"ADD"},
        {IntermOp::MUL,"MUL"},
        {IntermOp::DIV,"DIV"},
        {IntermOp::MOD,"MOD"},
        {IntermOp::AND,"AND"},
        {IntermOp::OR,           "OR"},
        {IntermOp::NOT,          "NOT"},
        {IntermOp::EQ,           "EQ"},
        {IntermOp::NEQ,          "NEQ"},
        {IntermOp::LSS,          "LSS"},
        {IntermOp::LEQ,          "LEQ"},
        {IntermOp::GRE,          "GRE"},
        {IntermOp::GEQ,          "GEQ"},
        {IntermOp::GETINT,       "GETINT"},
        {IntermOp::PRINT,        "PRINT"},
        {IntermOp::INIT_ARR_PTR, "INIT_ARR_PTR"},
        {IntermOp::ARR_SAVE,     "ARR_SAVE"},
        {IntermOp::ARR_LOAD,     "ARR_LOAD"},
        {IntermOp::LABEL,        "LABEL"},
        {IntermOp::JUMP,         "JUMP"},
        {IntermOp::BEQ,          "BEQ"},
        {IntermOp::BNE,          "BNE"},
        {IntermOp::FUNC_BEGIN,   "FUNC_BEGIN"},
        {IntermOp::FUNC_END,     "FUNC_END"},
        {IntermOp::PREPARE_CALL, "PREPARE_CALL"},
        {IntermOp::PUSH_VAL,     "PUSH_VAL"},
        {IntermOp::PUSH_ARR,"PUSH_ARR"},
        {IntermOp::CALL,"CALL"},
        {IntermOp::RET,"RET"},
};

bool is_arith(IntermOp op);
bool is_bitwise(IntermOp op);
bool is_cmp(IntermOp op);

struct IntermCode {
    std::string dst;
    IntermOp op;
    std::string src1;
    std::string src2;
    IntermCode(){};
    IntermCode(IntermOp op, std::string dst, std::string src1, std::string src2):
        op(op), dst(std::move(dst)), src1(std::move(src1)), src2(std::move(src2)) {}
};

std::string interm_code_to_string(const IntermCode& code, bool auto_indent);
std::string get_op_string(IntermOp op);

class Intermediate {
private:
    int tmp_cnt_ = 0; // how many tmp vars have been generated
    int param_arr_cnt_ = 0;
    int label_cnt_ = 0; // how many labels have been generated
    int while_label_cnt_ = 0;
    int cond_label_cnt_ = 0;
    int land_label_cnt = 0;
    int inline_times_ = 0;

    SymbolTable &symbol_table_;
    std::ofstream &out_;

    void handle_error(std::string msg);

public:
    std::vector<IntermCode> codes_;
    std::vector<std::string> strcons;

    void OutputCodes();

    void OutputCodes(std::ofstream& out);

    Intermediate(SymbolTable &symbol_table, std::ofstream &out);

    std::string GenTmpVar(const std::string &func_name, DataType data_type, int level, unsigned int addr);

    std::string GenTmpArr(const std::string &func_name, DataType data_type, int level,
                          int dims, int dim0_size, int dim1_size, unsigned int addr);

    std::string GenLabel();

    std::string GenWhileHeadLabel();

    std::string GenWhileBeginLabel();

    std::string GenWhileEndLabel();

    std::string GenCondEndLabel();

    std::string GenLAndEndLabel();

    void AddMidCode(const std::string &dst, IntermOp op, const std::string &src1, const std::string &src2);

    void AddMidCode(const std::string &dst, IntermOp op, int src1, const std::string &src2);

    void AddMidCode(const std::string &dst, IntermOp op, const std::string &src1, int src2);

    void AddMidCode(const std::string &dst, IntermOp op, int src1, int src2);

    std::string rename_inline_symbol(const std::string& caller_name, const std::string& callee_name, std::string symbol_name);

    void InlineFunc();

};

#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H
