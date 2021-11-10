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
    std::string cur_func_name_; // in which function label
    std::string callee_name_;
    int param_no_ = 0;
    std::vector<std::string> s_regs_table_ = {"", "", "", "", "", "", "", ""};
    std::vector<std::string> s_old_table_ = {"", "", "", "", "", "", "", ""};
    std::vector<int> s_fifo_order_ = {0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<int> s_old_order_;

    std::vector<std::string> t_regs_table_ = {"", "", "", "", "", "", "", "", "", ""};
    std::vector<std::string> t_old_table_ = {"", "", "", "", "", "", "", "", "", ""};
    std::vector<int> t_fifo_order_= {};
    std::vector<int> t_old_order_= {};
    std::vector<int> frame_size_stack = {};

public:
    std::string tab = "\t";
    std::string tab_space = "    ";
    const int context_size = 100;
    const int ra_off = 0; // $ra's offset to the pointer on the bottom of context stack
    const int sp_off = ra_off + 4; // 4-7
    const int a_regs_off = sp_off + 4; // 8-23
    const int s_regs_off = a_regs_off + 16; // =24-55
    const int t_regs_off = s_regs_off + 32; // =56-95
    const int conserved_off = t_regs_off + 40; // =96-99


    std::vector<std::string> reg = { // given a reg number, return its name
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

    std::pair<bool, std::string> search_in_st_regs(const std::string& symbol);

    std::string assign_t_reg(std::string symbol);

    std::string assign_s_reg(std::string symbol);

    std::pair<int, std::string> get_memo_addr(std::string symbol);

    void add_code(const std::string &code);

    void add_code(const std::string &op, const std::string &dst, const std::string &src1, const std::string &src2);

    void add_code(const std::string &op, const std::string &dst, const std::string &src1, int src2);

    void add_code(std::string op, const std::string &reg_name, int off, std::string addr);

    void add_code(const std::string &op, const std::string &dst, const std::string &src1);

    void add_error(const std::string &error_msg);

    void release_reg(std::string);

};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_MIPSGENERATOR_H
