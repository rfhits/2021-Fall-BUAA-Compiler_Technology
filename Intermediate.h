//
// Created by WYSJ6174 on 2021/10/17.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H

#include <string>
#include "SymbolTable.h"

enum class IntermOp {
    ADD, SUB, MUL, DIV, MOD,
    AND, OR, NOT, EQ, NEQ,
    LSS, LEQ, GRE, GEQ,
    GETINT, PRINT,

    ARR_SAVE, // save to array
    ARR_LOAD, // load from array

    BEQ, BNE, BGT, BGE, BLT, BLE,

    FUNC_BEGIN, FUNC_END,
    PREPARE_CALL, PUSH_VAL, PUSH_ARR, CALL, RET,


    GOTO, LABEL, EXIT
};

struct IntermCode {
    std::string dst;
    IntermOp op;
    std::string src1;
    std::string src2;

};

class Intermediate {
private:
    int tmp_cnt_ = 0; // how many tmp vars have been generated
    int label_cnt_ = 0; // how many labels have been generated
    std::vector<IntermCode> interm_codes;
    SymbolTable &symbol_table_;
    std::ofstream &out_;

public:
    Intermediate(SymbolTable &symbol_table, std::ofstream &out);

    std::string GenTmpVar(const std::string &func_name, DataType data_type, int level, unsigned int addr);

    std::string GenTmpArr(const std::string &func_name, DataType data_type, int level,
                          int dims, int dim0_size, int dim1_size, unsigned int addr);

    void AddMidCode(const std::string &dst, IntermOp op, const std::string &src1, const std::string &src2);

    void AddMidCode(const std::string &dst, IntermOp op, int src1, const std::string &src2);

    void AddMidCode(const std::string &dst, IntermOp op, const std::string &src1, int src2);

    void AddMidCode(const std::string &dst, IntermOp op, int src1, int src2);
};

#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H
