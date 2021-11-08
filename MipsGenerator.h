//
// Created by WYSJ6174 on 2021/11/8.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_MIPSGENERATOR_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_MIPSGENERATOR_H

#include "utils.h"
#include "SymbolTable.h"
#include "Intermediate.h"

class MipsGenerator {
private:
    SymbolTable symbol_table_;
    std::vector<IntermCode> interm_codes_;
    std::vector<std::string> strcons_;
    std::vector<std::string> mips_codes_;
    std::ofstream &out_;
    std::vector<std::string> s_reg_table_ = {"", "", "", "", "", "", "", "", "", ""};
    std::vector<std::string> t_reg_table_ = {"", "", "", "", "", "", "", ""};

public:
    std::string tab = "\t";
    std::string tab_space = "    ";
    std::vector<std::string> reg = {
            "$zero", "$at", "$v0", "$v1",
            "$a0", "$a1", "$a2", "$a3",
            "$t0", "$t1", "$t2", "$t3",
            "$t4", "$t5", "$t6", "$t7",
            "$s0", "$s1", "$s2", "$s3",
            "$s4", "$s5", "$s6", "$s7",
            "$t8", "$t9", "$k0", "$k1",
            "$gp", "$sp", "$fp", "$ra"
    };

    MipsGenerator(SymbolTable &symbol_table, std::vector<IntermCode> &interm_codes, std::vector<std::string> &strcons,
                  std::ofstream &out);

    void translate();

    void add_mips_code(const std::string &code);

    void add_error(std::string error_msg);

};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_MIPSGENERATOR_H
