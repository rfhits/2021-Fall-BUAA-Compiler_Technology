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
    ARR_SAVE, // save to array, arr_save arr_name index value
    ARR_LOAD, // load from array

    LABEL,
    JUMP, BEQ, BNE,

    FUNC_BEGIN, FUNC_END,
    PREPARE_CALL, PUSH_VAL, PUSH_ARR, CALL,

    RET, INVALID
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
        {IntermOp::INVALID,      "INVALID"}
};

bool is_arith(IntermOp op);

bool is_bitwise(IntermOp op);

bool is_cmp(IntermOp op);

// dst is read, not changed
// especially for the arr_save op, it is read from src2
bool is_read_op(IntermOp op);

bool is_write_op(IntermOp op) ;

struct IntermCode {
    std::string dst;
    IntermOp op;
    std::string src1;
    std::string src2;

    IntermCode() = default;

    IntermCode(IntermOp op, std::string dst, std::string src1, std::string src2) :
            op(op), dst(std::move(dst)), src1(std::move(src1)), src2(std::move(src2)) {}
};

std::string interm_code_to_string(const IntermCode &code, bool auto_indent);

std::string get_op_string(IntermOp op);


class DAGNode {
public:
    int id_;
    std::vector<std::string> symbols_; // the node represent these symbols
    IntermOp op_ = IntermOp::INVALID;
    int left_son_{};
    int right_son_{};

    explicit DAGNode(int id) {
        this->id_ = id;
    }

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

    bool IsEmpty() {
        return this->symbols_.empty();
    }

    void AddSymbol(const std::string &symbol_name) {
        auto it = std::find(symbols_.begin(), symbols_.end(), symbol_name);
        if (it != symbols_.end()) {
            return;
        } else {
            this->symbols_.push_back(symbol_name);
        }
    }

    void RemoveSymbol(const std::string &symbol) {
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
public:
    std::set<std::string> modified_symbols_;
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

class BasicBlockDAGManager {
public:
    std::vector<DAGNode> nodes;
    std::unordered_map<std::string, int> symbol_to_node_id;

    // @brief: given a symbol name, search it in the table
    //         if found, return its node id
    //         else: new a node, add to table
    int get_symbol_node_require_new(const std::string &symbol) {
        auto it = symbol_to_node_id.find(symbol);
        if (it != symbol_to_node_id.end()) {
            return it->second;
        } else {
            int id = nodes.size();
            DAGNode node = DAGNode(id);
            node.AddSymbol(symbol);
            nodes.push_back(node);
            symbol_to_node_id[symbol] = id;
            return id;
        }
    }

    // @pre: the op is binary op
    // @brief: op is the same and the son contains the symbol needed
    std::pair<bool, int> find_pattern(IntermOp op, const std::string &src1, std::string src2) {
        for (auto &node: nodes) {
            if (node.op_ != op) continue;
            int left_id = node.left_son_;
            int right_id = node.right_son_;
            if (op == IntermOp::ADD || op == IntermOp::MUL) {
                if ((nodes[left_id].ContainsSymbol(src1) && nodes[right_id].ContainsSymbol(src2)) ||
                    (nodes[left_id].ContainsSymbol(src2) && nodes[right_id].ContainsSymbol(src1))) {
                    return std::make_pair(true, node.id_);
                }
            } else {
                if (nodes[left_id].ContainsSymbol(src1) && nodes[right_id].ContainsSymbol(src2)) {
                    return std::make_pair(true, node.id_);
                } else {
                    continue;
                }
            }
        }
        return std::make_pair(false, -1);
    }

    void RemoveNodes(std::set<std::string> &modified_symbols) {
        for (auto &symbol: modified_symbols) {
            auto it = symbol_to_node_id.find(symbol);
            if ( it != symbol_to_node_id.end()) {
                nodes[it->second].RemoveSymbol(symbol);
                symbol_to_node_id.erase(it);
                get_symbol_node_require_new(symbol);
            }
        }
    }

    // remove the symbol from the node manager
    void remove_symbol(const std::string& symbol) {
        auto it = symbol_to_node_id.find(symbol);
        if (it == symbol_to_node_id.end()) return;
        // really exists
        nodes[it->second].RemoveSymbol(symbol);
        symbol_to_node_id.erase(it);
    }

    // given a code, return an eval code
    IntermCode GetEvalCode(const IntermCode &code) {
        std::string dst = code.dst;
        IntermOp op = code.op;
        std::string src1 = code.src1;
        std::string src2 = code.src2;
        if (is_read_op(op)) {
            // try to find a symbol in the dag,
            // if can not be found, new a node, add to table
            if (op == IntermOp::ARR_SAVE) {
                int id = get_symbol_node_require_new(src2);
                std::string re_src2 = nodes[id].GetSymbolName();
                return {op, dst, src1, re_src2};
            } else {
                // PRINT, remove %ret
                remove_symbol("%RET");
                int id = get_symbol_node_require_new(dst);
                std::string re_dst = nodes[id].GetSymbolName();
                return {op, re_dst, src1, src2};
            }
        }
            // is written op
        else if (is_write_op(op)) {
            // assign
            if (op == IntermOp::ADD && src2 == "0") {
                remove_symbol(dst); // dst is assigned, so remove if from the table and nodes
                int id = get_symbol_node_require_new(src1);
                nodes[id].AddSymbol(dst);
                symbol_to_node_id[dst] = id;
                std::string re_src1 = nodes[id].GetSymbolName();
                return {op, dst, re_src1, src2};
            }

            // read a value from io, return the original code, but change the table
            // same as the arr_load value arr_name index
            if (op == IntermOp::GETINT || op == IntermOp::ARR_LOAD) {
                remove_symbol(dst);
                int id = get_symbol_node_require_new(dst);
                symbol_to_node_id[dst] = id;
                return code;
            }

            std::pair<bool, int> search_res = find_pattern(op, src1, src2);
            if (search_res.first) {
                int pattern_id = search_res.second;
                // 1. find dst origin node id, if not exist ...
                // 2. move this symbol from the origin node, move to the pattern, change the symbol table
                // if the origin id not exists, directly move it to the pattern
                remove_symbol(dst);
                symbol_to_node_id[dst] = pattern_id; // the dst is in the node
                // through we find a pattern, but the node may be empty
                if (nodes[pattern_id].IsEmpty()) {
                    nodes[pattern_id].AddSymbol(dst);
                    std::string re_src1 = nodes[nodes[pattern_id].left_son_].GetSymbolName();
                    std::string re_src2 = nodes[nodes[pattern_id].right_son_].GetSymbolName();
                    return {op, dst, re_src1, re_src2};
                } else {
                    nodes[pattern_id].AddSymbol(dst);
                    return {IntermOp::ADD, dst, nodes[pattern_id].GetSymbolName(), "0"};
                }
            } else {
                // get the src1 and src2 node,
                // find dst origin node,
                // new a node then put it into
                int src1_id = get_symbol_node_require_new(src1);
                int src2_id = get_symbol_node_require_new(src2);
                std::string re_src1 = nodes[src1_id].GetSymbolName();
                std::string re_src2 = nodes[src2_id].GetSymbolName();
                remove_symbol(dst);
                // now we are sure dst is not in the table and nodes
                int dst_id = get_symbol_node_require_new(dst);
                nodes[dst_id].op_ = op;
                nodes[dst_id].left_son_ = src1_id;
                nodes[dst_id].right_son_ = src2_id;
                return {op, dst, re_src1, re_src2};
            }
        }
            // a simple code
        else {
            return code;
        }
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

    std::set<std::string> get_modified_symbols(std::string func_name);

    void divide_blocks();

    void construct_flow_rel();

    void add_modified_symbols();

    void common_expr();

    void sync_codes();

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
