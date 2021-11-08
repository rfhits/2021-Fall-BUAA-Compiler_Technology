//
// Created by WYSJ6174 on 2021/11/8.
//

#include "MipsGenerator.h"

#define MIPS_DBG true

MipsGenerator::MipsGenerator(SymbolTable &symbol_table, std::vector<IntermCode> &interm_codes,
                             std::vector<std::string> &strcons, std::ofstream &out) :
        symbol_table_(symbol_table), interm_codes_(interm_codes), strcons_(strcons), out_(out) {}

void MipsGenerator::translate() {
    // use the global table in symbol table to assign memory in .data
    add_mips_code(".data");
    for (auto &i: symbol_table_.global_table_) {
        if (i.symbol_type == SymbolType::CONST || i.symbol_type == SymbolType::VAR) {
            std::string code = (i.alias + ":" + tab + tab);
            if (i.data_type == DataType::INT) {
                code += (".word 0");
            } else if (i.data_type == DataType::INT_ARR) {
                code += (".space " + std::to_string(i.size));
            } else {
                add_error(".data error");
            }
        }
    }

}

void MipsGenerator::add_mips_code(const std::string &code) {
    if (MIPS_DBG) {
        std::cout << code << std::endl;
    } else {
        mips_codes_.push_back(code);
    }
}

void MipsGenerator::add_error(std::string error_msg) {
    std::cout << error_msg << std::endl;
}


