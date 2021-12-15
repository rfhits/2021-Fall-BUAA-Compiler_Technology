//
// Created by WYSJ6174 on 2021/11/8.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_MIPSGENERATOR_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_MIPSGENERATOR_H

#include "utils.h"
#include "SymbolTable.h"
#include "Intermediate.h"

const std::unordered_map<IntermOp, std::string> interm_op_to_instr = {
        {IntermOp::EQ,"seq"},
        {IntermOp::NEQ,"sne"},
        {IntermOp::LSS,"slt"},
        {IntermOp::LEQ,"sle"},
        {IntermOp::GRE,"sgt"},
        {IntermOp::GEQ,"sge"},
};

class MipsGenerator {
private:
    SymbolTable& symbol_table_;
    std::vector<IntermCode>& interm_codes_;
    std::vector<std::string> mips_codes_;
    std::ofstream &out_;
    std::string cur_func_name_; // in which function label
    std::string cur_callee_name_; // in which PREPARE_CALL
    std::vector<std::string> callee_name_stack_{};
    int param_no_ = 0;
    std::vector<std::string> s_regs_table_ = {"", "", "", "", "", "", "", ""};
    std::vector<int> s_order_ = {0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<std::string> saved_s_regs_table_{};
    std::vector<int> saved_s_order_{};

    std::vector<std::string> t_regs_table_ = {"", "", "", "", "", "", "", "", "", ""};
    std::vector<int> t_order_= {0, 1, 2, 3, 4, 5, 6, 7};
    std::vector<std::string> saved_t_regs_table_{};
    std::vector<int> saved_t_order{};

    std::vector<std::string> write_back_symbols_ = {};
    std::vector<int> frame_size_stack_ = {};

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

    MipsGenerator(SymbolTable &symbol_table, std::vector<IntermCode> &interm_codes, std::ofstream &out);

    bool will_be_used_later(const std::string& symbol, int i);

    void translate();

    void remove_from_reg(std::string reg_name);

    std::pair<bool, std::string> search_in_st_regs(const std::string& symbol);

    void save_to_memo(const std::string& table_name, const std::string& symbol);

    void remove_from_reg_save_to_memo(const std::string& table_name, const std::string& symbol);

    std::string get_empty_s_reg();

    void move_reg_no_to_order_end(std::string table_name, int reg_no);

    std::string assign_t_reg(std::string symbol);

    std::string assign_s_reg_require_load_from_memo(const std::string& symbol);

    std::string assign_s_reg_without_load_from_memo(const std::string& symbol);

    std::string get_reg_require_load_from_memo(std::string symbol);

    std::string get_reg_without_load_from_memo(std::string symbol);

    void save_symbol_to_the_reg(std::string symbol, const std::string& reg_name);

    std::pair<int, std::string> get_memo_addr(const std::string& symbol);

    void remove_s_regs_save_to_memo();

    void remove_t_regs_save_to_memo();

    std::string assign_reg_require_load_from_memo(const std::string& symbol);

    std::string assign_reg_without_load_from_memo(std::string symbol);

    void add_code(const std::string &code);

    void add_code(const std::string &op, const std::string &dst, const std::string &src1, const std::string &src2);

    void add_code(const std::string &op, const std::string &dst, const std::string &src1, int src2);

    void add_code(std::string op, const std::string &reg_name, int off, std::string addr);

    void add_code(const std::string &op, const std::string &dst, const std::string &src1);

    void add_error(const std::string &error_msg);



};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_MIPSGENERATOR_H
