//
// Created by WYSJ6174 on 2021/10/17.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H

#include <string>
#include <set>
#include <utility>
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
        {IntermOp::SUB,          "SUB"},
        {IntermOp::ADD,          "ADD"},
        {IntermOp::MUL,          "MUL"},
        {IntermOp::DIV,          "DIV"},
        {IntermOp::MOD,          "MOD"},
        {IntermOp::AND,          "AND"},
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
        {IntermOp::PUSH_ARR,     "PUSH_ARR"},
        {IntermOp::CALL,         "CALL"},
        {IntermOp::RET,          "RET"},
};

bool is_arith(IntermOp op);

bool is_bitwise(IntermOp op);

bool is_cmp(IntermOp op);

struct IntermCode {
    std::string dst;
    IntermOp op;
    std::string src1;
    std::string src2;

    IntermCode() {};

    IntermCode(IntermOp op, std::string dst, std::string src1, std::string src2) :
            op(op), dst(std::move(dst)), src1(std::move(src1)), src2(std::move(src2)) {}
};

std::string interm_code_to_string(const IntermCode &code, bool auto_indent);

std::string get_op_string(IntermOp op);

class DAGNode {
public:
    int id_;
    std::vector<std::string> symbols_; // the node represent these symbols
    IntermOp op_;
    std::vector<int> sons_;

    std::string GetSymbolName() {
        return symbols_[0];
    }

    bool ContainsSymbol(const std::string &symbol_name) {
        if (std::find(symbols_.begin(), symbols_.end(), symbol_name) != symbols_.end()) {
            return true;
        } else {
            return false;
        }
    }

    void AddSymbol(const std::string &symbol_name) {
        this->symbols_.push_back(symbol_name);
    }

    void RemoveSymbol(std::string symbol) {
        auto it = std::find(symbols_.begin(), symbols_.end(), symbol);
        if (it == symbols_.end()) std::cerr << "remove a not find symbol" << std::endl;
        symbols_.erase(it);
    }
};

class BasicBlock {
public:
    int id_;
    std::set<int> pred_blocks_;
    std::set<int> succ_blocks_;
    std::set<std::string> def_;
    std::set<std::string> use_;

    std::vector<IntermCode> label_codes_;
    std::vector<IntermCode> jb_codes_; // jump and branch codes
    std::vector<IntermCode> codes_;

    explicit BasicBlock(int id) {
        this->id_ = id;
    }

    bool IsRetBlock() {
        if (!this->codes_.empty() && this->codes_.back().op == IntermOp::RET) return true;
        else return false;
    }

    bool HasNoCodes() {
        return this->codes_.empty();
    }

    void AddCode(const IntermCode &code) {
        this->codes_.push_back(code);
    }

    void AddLabelCode(const IntermCode &code) {
        this->label_codes_.push_back(code);
    }

    void AddJBCode(const IntermCode &code) {
        this->jb_codes_.push_back(code);
    }

    void AddPred(int id) {
        this->pred_blocks_.insert(id);
    }

    void AddSucc(int id) {
        this->succ_blocks_.insert(id);
    }

    bool ContainsLabel(const std::string &label) {
        bool flag = false;
        for (auto &code: label_codes_) {
            if (code.dst == label) {
                flag = true;
                break;
            }
        }
        return flag;
    }

    void OutputBasicBlock(std::ofstream &out) {
        out << "---- block id: " << this->id_ << " ----" << std::endl;
        out << "pred: ";
        for (int id: pred_blocks_) {
            out << std::to_string(id) << " ";
        }
        out << std::endl;
        out << "--------" << std::endl;
        for (auto &code: label_codes_) {
            out << interm_code_to_string(code, true) << std::endl;
        }
        for (auto &code: codes_) {
            out << interm_code_to_string(code, true) << std::endl;
        }
        for (auto &code: jb_codes_) {
            out << interm_code_to_string(code, true) << std::endl;
        }
        out << "--------" << std::endl;
        out << "succ: ";
        for (int id: succ_blocks_) {
            out << std::to_string(id) << " ";
        }
        out << std::endl;
        out << "--------" << std::endl << std::endl;
    }
};

class FuncBlock {
private:
    std::set<std::string> modified_symbols_;
public:
    std::vector<int> block_ids_; // block id
    std::string func_name_;

    explicit FuncBlock(std::string func_name) {
        this->func_name_ = std::move(func_name);
    }

    void AddBlock(int block_id) {
        this->block_ids_.push_back(block_id);
    }

    void AddModifiedSymbol(const std::string &symbol) {
        this->modified_symbols_.insert(symbol);
    }

};

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

    int cur_block_id_ = -1;
//    int cur_func_block_id = -1;

    bool enable_inline_ = false;
    bool enable_peephole_ = true;
    bool enable_common_expr_ = true;
    bool enable_const_prop_ = true;
    bool enable_copy_prop_ = true;
    bool enable_DCE_ = true; // dead code elimination

    void peephole_optimize();

    void new_basic_block();

    void new_func_block(std::string func_name);

    void divide_basic_block();

    void construct_flow_rel();

    void add_modified_symbols();

    void common_expr();

    void handle_error(std::string msg);

public:
    std::vector<IntermCode> codes_;
    std::vector<std::string> strcons;
    std::vector<BasicBlock> basic_blocks_;
    std::vector<FuncBlock> func_blocks_;

    void OutputCodes();

    void OutputCodes(std::ofstream &out);

    void OutputBasicBlocks(std::ofstream &out);

    void OutputFuncBlocks(std::ofstream &out);

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

    std::string
    rename_inline_symbol(const std::string &caller_name, const std::string &callee_name, std::string symbol_name);

    void Optimize();

    void InlineFunc();


};

#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H
