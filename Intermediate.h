//
// Created by WYSJ6174 on 2021/10/17.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_INTERMEDIATE_H

#include <string>
#include <set>
#include <utility>
#include <limits.h>
#include "SymbolTable.h"

enum class IntermOp {
    ADD, SUB, MUL, DIV, MOD,

    AND, OR, NOT, // deprecate, I thought it was used for condition, then I wrong

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

bool op_modify_dst(IntermOp op);

struct IntermCode {
    std::string dst;
    IntermOp op = IntermOp::INVALID;
    std::string src1;
    std::string src2;

    IntermCode() = default;

    IntermCode(IntermOp op, std::string dst, std::string src1, std::string src2) :
            op(op), dst(std::move(dst)), src1(std::move(src1)), src2(std::move(src2)) {}
};

std::string interm_code_to_string(const IntermCode &code, bool auto_indent);

std::string get_op_string(IntermOp op);


class ConflictGraph {
public:
    std::unordered_map<std::string, std::set<std::string>> graph;
    std::unordered_map<std::string, std::set<std::string>> copy_graph;
    int reg_num_ = -1;
    std::vector<std::pair<std::string, int>> symbol_reg_pairs;

    void extent_by_set(const std::set<std::string> &out) {
        for (const std::string &symbol: out) {
            std::set<std::string> symbol_conflict = out; // copy
            symbol_conflict.erase(symbol);
            // if graph has this symbol, extent its conflict set
            // else set this as its conflict set
            auto it = graph.find(symbol);
            if (it == graph.end()) {
                graph[symbol] = symbol_conflict;
            } else {
                graph[symbol].insert(symbol_conflict.begin(), symbol_conflict.end());
            }
        }
    }

    // @brief: given a symbol, get its reg_id
    // @exception: may not find this symbol
    // @retval: if found
    std::pair<bool, int> SearchSymbolReg(const std::string &symbol) {
        for (const std::pair<std::string, int>& symbol_reg: symbol_reg_pairs) {
            if (symbol_reg.first == symbol) {
                return std::make_pair(true, symbol_reg.second);
            } else {
                continue;
            }
        }
        return std::make_pair(false, -2);
    }

    // @pre: the copy_graph's size > 1
    // @brief: get the min degree symbol and its degree from copy_graph
    std::pair<std::string, int> get_min_degree_symbol() {
        std::string symbol;
        int min_degree = 2147483647;
        auto it = copy_graph.begin();
        while (it != graph.end()) {
            if (it->second.size() < min_degree) {
                min_degree = it->second.size();
                symbol = it->first;
            }
            it++;
        }
        return std::make_pair(symbol, min_degree);
    }


    void remove_from_copy_graph(std::string symbol) {
        copy_graph.erase(symbol);
        auto it = copy_graph.begin();
        while (it != copy_graph.end()) {
            if (it->second.find(symbol) != it->second.end()) {
                it->second.erase(symbol);
            }
            it++;
        }
    }

    // @pre: the symbol not in copy_graph
    // @brief: add a symbol to the cutted copy graph

    void add_symbol_to_copy_graph(std::string symbol) {
        std::set<std::string> link_symbols = graph[symbol];
        if (copy_graph.find(symbol) != copy_graph.end()) {
            std::cerr << "add an already exist symbol " + symbol + " to copy graph" << std::endl;
        }
        copy_graph[symbol] = {};
        for (std::string link_symbol: link_symbols) {
            if (copy_graph.find(link_symbol) != copy_graph.end()) {
                copy_graph[link_symbol].insert(symbol);
                copy_graph[symbol].insert(link_symbol);
            }
        }
    }

    // Color symbol in copy_graph
    // @pre: pair.second == -2
    void color_symbol(std::string symbol) {
        auto it = copy_graph.find(symbol);
        if (it == copy_graph.end()) {
            std::cerr << "can't find symbol " + symbol + " in copy_graph";
        }
        std::set<int> free_regs;
        for (int i = 0; i < reg_num_; i++) free_regs.insert(i);

        for (const std::string &link_symbol: it->second) {
            // get symbol_reg_pair
            for (const auto &symbol_reg_pair: symbol_reg_pairs) {
                if (symbol_reg_pair.first == link_symbol) {
                    free_regs.erase(symbol_reg_pair.second);
                }
            }
        }
        if (free_regs.empty()) std::cerr << "no reg can assign" << std::endl;
        int assign_reg = *free_regs.begin();
        for (auto &symbol_reg_pair: symbol_reg_pairs) {
            if (symbol_reg_pair.first == symbol) {
                symbol_reg_pair.second = assign_reg;
                break;
            }
        }
    }

    // @brief: given the reg number,
    //         save each symbol, reg_no
    // @note:
    // 1.	找到第一个连接边数目小于 K 的结点，将它从图 G 中移走，形成图G’
    // 2.	重复步骤 1 ，直到无法再移走结点，现在，图中每个顶点的度数都大于等于Ｋ
    // 3.	在图中选取“适当”的结点，将它记录为“不分配全局寄存器”的结点，并从图中移走
    // 4.	重复步骤 1~ 步骤 3 ，直到图中仅剩余 1 个结点
    // 5.	给剩余的最后一个结点选取一种颜色，然后按照结点被移走的顺序，反向将结点和边添加进去，并依次给新加入的结点选取颜色。
    //      （保证有链接边的结点着不同的颜色）
    void Color(int reg_num) {
        this->reg_num_ = reg_num;
        copy_graph = graph;
        // -1: save to memo, never Color
        // -2: waiting for coloring
        while (copy_graph.size() > 1) {
            std::pair<std::string, int> symbol_degree = get_min_degree_symbol();
            int sign_reg_id = -2;
            if (symbol_degree.second >= reg_num) {
                // stay in memory
                sign_reg_id = -1;
            } else {
                // the degree is small, so we can assign
                sign_reg_id = -2;
            }
            symbol_reg_pairs.emplace_back(symbol_degree.first, sign_reg_id);
            // remove from the copy graph
            remove_from_copy_graph(symbol_degree.first);
        }
        if (copy_graph.size() != 1) return; // no symbols

        // now there is only one symbol in the graph
        symbol_reg_pairs.emplace_back(copy_graph.begin()->first, 0);
        // reverse add
        auto it = symbol_reg_pairs.rbegin();
        it++;
        while (it != symbol_reg_pairs.rend()) {
            // waiting for assign
            if (it->second == -2) {
                add_symbol_to_copy_graph(it->first);
                color_symbol(it->first);
            }
                // stay in memo, never assign
            else {
                add_symbol_to_copy_graph(it->first);
            }
            it++;
        }
    }

    std::vector<int> GetUsedRegs() {
        std::vector<int> used_regs = {};
        for (const auto& symbol_reg: symbol_reg_pairs) {
            if (symbol_reg.second != -1) {
                used_regs.push_back(symbol_reg.second);
            }
        }
        std::sort(used_regs.begin(), used_regs.end());
        auto it = std::unique(used_regs.begin(), used_regs.end());
        used_regs.erase(it, used_regs.end());
        return used_regs;
    }
};

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
    std::set<std::string> in_;
    std::set<std::string> out_;
    std::set<std::string> new_in_;
    std::set<std::string> new_out_;

    std::vector<IntermCode> label_codes_;
    std::vector<IntermCode> jb_codes_; // jump and branch codes
    std::vector<IntermCode> codes_;

    explicit BasicBlock(int id) {
        this->id_ = id;
    }

    void AddToDef(std::string symbol) {
        this->def_.insert(symbol);
    }

    void AddToUse(std::string symbol) {
        this->use_.insert(symbol);
    }

    bool ContainsDef(std::string symbol) {
        auto it = def_.find(symbol);
        if (it == def_.end()) {
            return false;
        } else {
            return true;
        }
    }

    bool ContainsUse(std::string symbol) {
        auto it = use_.find(symbol);
        if (it == use_.end()) {
            return false;
        } else {
            return true;
        }
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

    // out = Union (in_succ)
    void extend_new_out(std::set<std::string> &set_input) {
        new_out_.insert(set_input.begin(), set_input.end());
    }

    // in = use + (out - def)
    void cal_new_in() {
        // new_in = new_out - def
        str_set_diff(new_in_, new_out_, def_);

        // new_in += use
        new_in_.insert(use_.begin(), use_.end());

    }

    bool in_out_not_change() {
        bool in_same = str_set_equal(in_, new_in_);
        bool out_same = str_set_equal(out_, new_out_);
        return in_same & out_same;
    }

    // move new_in and new_out to orginal in and out
    void sync_in_out() {
        in_.insert(new_in_.begin(), new_in_.end());
        out_.insert(new_out_.begin(), new_out_.end());
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

        out << std::endl << "--------" << std::endl;
        out << "def before use: ";
        for (const std::string &def_sym: this->def_) {
            out << def_sym << ", ";
        }
        out << std::endl;

        out << "use before def: ";
        for (const std::string &use_sym: this->use_) {
            out << use_sym << ", ";
        }
        out << std::endl;

        out << "in: ";
        for (const std::string &in_sym: this->in_) {
            out << in_sym << ", ";
        }
        out << std::endl;

        out << "out: ";
        for (const std::string &out_sym: this->out_) {
            out << out_sym << ", ";
        }
        out << std::endl;

        out << "--------" << std::endl << std::endl;
    }
};

class FuncBlock {
public:
    std::set<std::string> modified_global_symbols_;
    std::set<int> modified_param_orders = {};
    std::set<std::string> read_global_symbols_;
    std::set<int> read_param_orders = {};
    std::unordered_map<std::string, int> param_arr_name_to_order = {};
    bool has_print_ = false;
    bool has_getint_ = false;
    bool write_memo_ = false;

    std::vector<int> block_ids_; // block id
    std::string func_name_;

    ConflictGraph conflict_graph_;

    explicit FuncBlock(std::string func_name) {
        this->func_name_ = std::move(func_name);
    }

    void AddBlock(int block_id) {
        this->block_ids_.push_back(block_id);
    }

    // @brief: add those INT_ARR param to map
    void AddArrParam(const std::string &arr_name, int i) {
        this->param_arr_name_to_order[arr_name] = i;
    }

    // @brief: given an array name, check whether it is the function's param
    bool ContainsParamArr(const std::string &arr_name) {
        auto it = this->param_arr_name_to_order.find(arr_name);
        return it == param_arr_name_to_order.end();
    }

    // @brief: given a modified param name,
    //         transfer into its param order and add to modified order
    void AddModifiedParam(const std::string &name) {
        auto it = param_arr_name_to_order.find(name);
        if (it == param_arr_name_to_order.end()) {
            std::cerr << "this param name can't be found" << std::endl;
        } else {
            this->modified_param_orders.insert(it->second);
        }
    }

    void AddModifiedSymbol(const std::string &symbol) {
        this->modified_global_symbols_.insert(symbol);
    }

    void AddReadSymbol(const std::string &symbol) {
        this->read_global_symbols_.insert(symbol);
    }

    bool ContainsModifiedSymbol(const std::string &name) {
        auto it = this->modified_global_symbols_.find(name);
        return it == modified_global_symbols_.end();
    }

    // @brief: given a local symbol, return its reg no
    std::pair<bool, int> SearchSymbolReg(std::string symbol) {
        std::pair<bool, int> search_res = conflict_graph_.SearchSymbolReg(symbol);
        return search_res ;
    }

    std::vector<int> GetUsedRegs() {
        return conflict_graph_.GetUsedRegs();
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
            if (it != symbol_to_node_id.end()) {
                nodes[it->second].RemoveSymbol(symbol);
                symbol_to_node_id.erase(it);
                get_symbol_node_require_new(symbol);
            }
        }
    }

    // remove the symbol from the node manager
    void remove_symbol(const std::string &symbol) {
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
                // ARR_SAVE arr_name index value
                int id = get_symbol_node_require_new(src2);
                std::string re_src2 = nodes[id].GetSymbolName();
                id = get_symbol_node_require_new(src1);
                std::string re_src1 = nodes[id].GetSymbolName();
                return {op, dst, re_src1, re_src2};
            } else {
                // PRINT, remove %ret
                remove_symbol("%RET");
                int id = get_symbol_node_require_new(dst);
                std::string re_dst = nodes[id].GetSymbolName();
                return {op, re_dst, src1, src2};
            }
        }
            // is written op
        else if (op_modify_dst(op)) {
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
            if (op == IntermOp::GETINT) {
                remove_symbol(dst);
                int id = get_symbol_node_require_new(dst);
                // symbol_to_node_id[dst] = id;
                return code;
            }
            // arr_load value arr_name index,
            // index is src2
            if (op == IntermOp::ARR_LOAD) {
                int id = get_symbol_node_require_new(src2);
                std::string re_src2 = nodes[id].GetSymbolName();
                remove_symbol(dst);
                get_symbol_node_require_new(dst);
                return {op, dst, src1, re_src2};
            }

            if (op == IntermOp::INIT_ARR_PTR) {
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

    bool enable_inline_ = false;
    bool enable_peephole_ = true;
    bool enable_common_expr_ = true;
    bool enable_const_prop_ = true;
    bool enable_copy_prop_ = true;
    bool enable_DCE_ = true; // dead code elimination

    void peephole_optimize();

    void new_basic_block();

    void new_func_block(std::string func_name);

    void reset_blocks();

    void divide_blocks();

    void construct_flow_rel();

    void add_modified_symbols();

    void add_read_symbols();

    void check_print_getint_memo(); // check each function will print sth or not

    void common_expr();

    void sync_codes();

    void handle_error(std::string msg);

    void gen_def_and_use();

    void gen_in_and_out();

    void dead_code_elimination();

    void delete_useless_loop_in_main();

    void gen_func_conflict_graph();

    void remove_global_symbols(std::set<std::string> &s);

    std::pair<bool, int> search_func_block_by_name(const std::string &func_name);

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
