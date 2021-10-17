//
// Created by WYSJ6174 on 2021/10/17.
//

#include "Intermediate.h"


Intermediate::Intermediate(SymbolTable &symbol_table, std::ofstream &out):symbol_table_(symbol_table), out_(out) {
}

// generate temp variable into symbol_table
std::string Intermediate::GenTmpVar(const std::string &func_name, DataType data_type, int level) {
    std::string name = "Tmp_Variable_Prefix" + std::to_string(tmp_cnt_++);
    symbol_table_.AddSymbol(func_name, data_type, SymbolType::VAR,name, 0, level);
    return name;
}

void Intermediate::AddMidCode(const std::string& dst, IntermOp op, const std::string& src1, const std::string& src2) {
    IntermCode interm_code;
    interm_code.dst = dst;
    interm_code.op = op;
    interm_code.src1 = src1;
    interm_code.src2 = src2;
    interm_codes.push_back(interm_code);
}
