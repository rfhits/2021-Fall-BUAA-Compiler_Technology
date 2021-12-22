//
// Created by WYSJ6174 on 2021/11/8.
//

#include "MipsGenerator.h"

#include <utility>

#define MIPS_DBG true

MipsGenerator::MipsGenerator(SymbolTable &symbol_table, std::vector<IntermCode> &interm_codes,
                             std::vector<FuncBlock> &func_blocks, std::vector<BasicBlock> &basic_blocks,
                             std::ofstream &out) :
        symbol_table_(symbol_table), interm_codes_(interm_codes), func_blocks_(func_blocks),
        basic_blocks_(basic_blocks), out_(out) {

}

// @brief: search a symbol in s regs and t regs,
//         if in regs, return <true, reg_name>
//         else return <false, "">.
//         this function won't change the order, it's a lower abstract action
std::pair<bool, std::string> MipsGenerator::search_in_st_regs(const std::string &symbol) {
    for (int i = 0; i < s_regs_table_.size(); i++) {
        if (s_regs_table_[i] == symbol) {
            return std::make_pair(true, "$s" + std::to_string(i));
        }
    }
    for (int i = 0; i < t_regs_table_.size(); i++) {
        if (t_regs_table_[i] == symbol) {
            return std::make_pair(true, "$t" + std::to_string(i));
        }
    }
    return std::make_pair(false, "");
}

void MipsGenerator::move_reg_no_to_order_end(std::string table_name, int reg_no) {
    if (table_name == "s") {
        auto it = std::find(s_order_.begin(), s_order_.end(), reg_no);
        if (it != s_order_.end()) {
            s_order_.erase(it);
        } else {
            add_error("reg no can't be find in s order while moving to end");
        }
        s_order_.push_back(reg_no);
    } else if (table_name == "t") {
        auto it = std::find(t_order_.begin(), t_order_.end(), reg_no);
        if (it != t_order_.end()) {
            t_order_.erase(it);
        } else {
            add_error("reg no can't be find in t order while moving to end");
        }
        t_order_.push_back(reg_no);
    } else {
        add_error("undefined table name while moving reg no to order end");
    }
}

// @brief: release a reg without writing back,
//         will change the order and table
// @param: a string like "$s1", "$t2", ...
// @note: put the reg number to fifo end
void MipsGenerator::remove_from_reg(std::string reg_name) {
    if (reg_name[1] == 's') {
        int reg_no = reg_name[2] - '0';
        s_regs_table_[reg_no] = "";
    } else if (reg_name[1] == 't') {
        int reg_no = reg_name[2] - '0';
        t_regs_table_[reg_no] = "";
    } else {
        add_error("release failed: input reg name '" + reg_name + "' can't be recognize");
    }
}

// @brief: given a symbol, return its offset to its base pointer $sp or $gp
//         we have already considered the function call stack size
//         so u don't need to add the frame size back
// @note: in PUSH_VAL, the offset need to change, because the sp has been subtracted
// @exec: may can find the symbol in symbol table
// @retval: <int offset, string pointer>, like <4, "$gp">
std::pair<int, std::string> MipsGenerator::get_memo_addr(const std::string &symbol) {
    std::pair<bool, TableEntry *> table_search_res =
            symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, symbol);
    if (!table_search_res.first)
        add_error("symbol " + symbol + " can't be found in symbol table while getting its memo");

    if (table_search_res.second->level == 0) {
        return std::make_pair(table_search_res.second->addr, "$gp");
    } else {
        if (frame_size_stack_.empty()) {
            return std::make_pair(table_search_res.second->addr, "$sp");
        } else {
            return std::make_pair(sum(frame_size_stack_) + table_search_res.second->addr, "$sp");
        }
    }
}

// @brief: given a symbol name, check in current function assigned regs,
//         if not, load to the back_reg, then return the reg
//          this function is called when trying to read some symbol
std::string MipsGenerator::get_reg_with_fail_load(std::string symbol, const std::string &back_reg_name) {
    std::pair<int, std::string> search_res = get_running_addr(std::move(symbol));
    if (search_res.first == -1) {
        return search_res.second;
    } else {
        add_code("lw", back_reg_name, search_res.first, search_res.second);
        return back_reg_name;
    }
}

// @brief: given a symbol name, check in current function assigned regs,
//         if not, return the back_reg_name
std::string MipsGenerator::get_reg_without_fail_load(std::string symbol, const std::string &back_reg_name) {
    std::pair<int, std::string> search_res = get_running_addr(std::move(symbol));
    if (search_res.first == -1) {
        return search_res.second;
    } else {
        return back_reg_name;
    }
}


// this function is called when executing a function
// we wont call this function when init the global variables
//@retval: if in stack, return <offset, $sp> or <offset, $gp>
//         if in reg, return <-1, reg_name>
std::pair<int, std::string> MipsGenerator::get_running_addr(std::string symbol) {
    if (symbol == "%RET") return std::make_pair(-1, "$v0");

    if (symbol_table_.is_global_symbol(symbol)) {
        return get_memo_addr(symbol);
    } else {
        // local variable
        // 1. search in the func conflict graph
        // 2. if the result is -1, means stay in memory, then use get_memo_addr
        std::pair<bool, int> search_func_res = search_func_block_by_name(cur_func_name_);
        if (!search_func_res.first) add_error("func not found");
        FuncBlock &funcBlock = func_blocks_[search_func_res.second];
        std::pair<bool, int> search_reg_res = funcBlock.SearchSymbolReg(symbol);
        if (!search_reg_res.first) {
            add_error(symbol + " not found in reg when get_running_addr");
        }
        if (search_func_res.second == -1) {
            return get_memo_addr(symbol);
        } else {
            return std::make_pair(-1, reg_pool_[search_reg_res.second]);
        }

    }
}


// @brief: copy the reg value to memory,
//         only generates the MIPS code, not change the order and table
// @pre: this symbol can be found in t-regs table or s-regs table
// @param[table_name]: "s" or "t"
void MipsGenerator::save_to_memo(const std::string &table_name, const std::string &symbol) {
    std::pair<bool, TableEntry *> search_res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, symbol);
    if (table_name != "s" && table_name != "t") add_error("table name error");
    int reg_no = -1;
    if (table_name == "s") {
        for (int i = 0; i < s_order_.size(); i++) {
            if (s_regs_table_[i] == symbol) {
                reg_no = i;
                break;
            }
        }
    } else {
        for (int i = 0; i < t_regs_table_.size(); i++) {
            if (t_regs_table_[i] == symbol) {
                reg_no = i;
                write_back_symbols_.push_back(symbol);
                break;
            }
        }
    }
    std::pair<int, std::string> pop_addr = get_memo_addr(symbol);
    add_code("sw", "$" + table_name + std::to_string(reg_no), pop_addr.first, pop_addr.second);
}

// @brief: write the reg value back to memory, and release the reg
//         will change the order and table
// @param[table_name]: "s" or "t"
void MipsGenerator::remove_from_reg_save_to_memo(const std::string &table_name, const std::string &symbol) {
    save_to_memo(table_name, symbol);
    std::pair<bool, std::string> search_res = search_in_st_regs(symbol);
    if (!search_res.first) add_error("symbol not in st regs");
    remove_from_reg(search_res.second);
}

// @brief: move the s regs to the memo,
//         this function is called when entering a basic block
//
void MipsGenerator::remove_s_regs_save_to_memo() {
    for (int i = 0; i < s_order_.size(); i++) {
        if (!s_regs_table_[i].empty()) {
            save_to_memo("s", s_regs_table_[i]);
            s_regs_table_[i].clear();
        }
    }
}

void MipsGenerator::remove_t_regs_save_to_memo() {
    for (int i = 0; i < t_order_.size(); i++) {
        if (!t_regs_table_[i].empty()) {
            save_to_memo("t", t_regs_table_[i]);
            write_back_symbols_.push_back(t_regs_table_[i]);
            t_regs_table_[i].clear();
        }
    }
}

// @brief: this function is called while return a value,
//         load a symbol to a reg without writing the content of the reg back,
//         it's dangerous.
void MipsGenerator::save_symbol_to_the_reg(std::string symbol, const std::string &reg_name) {
    if (is_integer(symbol)) {
        add_code("add", reg_name, "$zero", symbol); // do not change the order, or the Mars error
    } else if (symbol == "%RET") {
        add_code("add", reg_name, "$v0", "$0");
    } else {
        std::pair<bool, std::string> search_res = search_in_st_regs(symbol);
        if (search_res.first) {
            add_code("move", reg_name, search_res.second);
        } else {
            std::pair<int, std::string> symbol_addr = get_memo_addr(symbol);
            add_code("lw", reg_name, symbol_addr.first, symbol_addr.second);
        }
    }
}

// @brief: symbol need to in a reg,
//         so search in regs, if already in, just return,
//         if not, load to reg from the stack.
//         this function will change the order
// @note: before u use this function,
//        make sure this symbol really should be put into reg,
//        because u don't always need to load some symbol to reg
std::string MipsGenerator::get_reg_require_load_from_memo(std::string symbol) {
    if (symbol == "%RET") return "$v0";
    std::pair<bool, std::string> search_res = search_in_st_regs(symbol);
    if (search_res.first) {
        int reg_no = search_res.second[2] - '0';
        if (search_res.second[1] == 's') {
            move_reg_no_to_order_end("s", reg_no);
        } else {
            move_reg_no_to_order_end("t", reg_no);
        }
        return search_res.second;
    } else {
        if (symbol[0] == '#') {
            return assign_t_reg(symbol);
        } else {
            return assign_s_reg_require_load_from_memo(symbol);
        }
    }
}

std::string MipsGenerator::get_reg_without_load_from_memo(std::string symbol) {
    if (symbol == "%RET") return "$v0";
    std::pair<bool, std::string> search_res = search_in_st_regs(symbol);
    if (search_res.first) {
        int reg_no = search_res.second[2] - '0';
        if (search_res.second[1] == 's') {
            move_reg_no_to_order_end("s", reg_no);
        } else {
            move_reg_no_to_order_end("t", reg_no);
        }
        return search_res.second;
    } else {
        if (symbol[0] == '#') {
            return assign_t_reg(symbol);
        } else {
            return assign_s_reg_without_load_from_memo(symbol);
        }
    }
}

// @brief: get an empty s reg, not change the order
//         if there is empty reg, use it,
//         else use the order first
std::string MipsGenerator::get_empty_s_reg() {
    int reg_no = -1;
    bool has_empty = false;
    for (int i = 0; i < s_order_.size(); i++) {
        if (s_regs_table_[i].empty()) {
            reg_no = i;
            has_empty = true;
            break;
        }
    }
    if (!has_empty) {
        reg_no = s_order_[0];
        save_to_memo("s", s_regs_table_[reg_no]);
        s_regs_table_[reg_no].clear();
    }
    return "$s" + std::to_string(reg_no);
}

// @brief: this function is called when do arith instruction, so will change the order
// @pre: the symbol not in the s regs
std::string MipsGenerator::assign_s_reg_without_load_from_memo(const std::string &symbol) {
    std::string reg_name = get_empty_s_reg();
    int reg_no = reg_name[2] - '0';
    move_reg_no_to_order_end("s", reg_no);
    s_regs_table_[reg_no] = symbol;
    return reg_name;
}

// @brief: this function is called to assign a t-reg for the temp symbol input
//         temp symbol wouldn't be init, so we can just mark a reg for it,
//         but if the write-back happened, we need to reload it from the stack
// @pre: the symbol not in the t-regs
// @retval: reg name, like "$t0"
std::string MipsGenerator::assign_t_reg(std::string symbol) {
    bool has_empty = false; // need to pop out?
    bool need_load = false; // symbol has been written back to stack, need to load
    int reg_no = -1;

    if (symbol[0] != '#') add_error("assign a none temp var to t reg");
    for (int i: t_order_) {
        if (t_regs_table_[i].empty()) {
            has_empty = true;
            reg_no = i;
            break;
        }
    }
    auto it = std::find(write_back_symbols_.begin(), write_back_symbols_.end(), symbol);
    if (it != write_back_symbols_.end()) need_load = true;

    if (!has_empty) {
        // pop the first of order, move it to the end
        reg_no = t_order_[0];
        std::string pop_symbol_name = t_regs_table_[reg_no];
        save_to_memo("t", pop_symbol_name);
//        for (int i = 1; i < t_order_.size(); i++) {
//            t_order_[i - 1] = t_order_[i];
//        }
//        t_order_.back() = reg_no;
        t_regs_table_[reg_no] = "";
        write_back_symbols_.push_back(pop_symbol_name);
    }

    // now the reg_no is correct
    if (need_load) {
        std::pair<int, std::string> push_addr = get_memo_addr(symbol);
        add_code("lw", "$t" + std::to_string(reg_no), push_addr.first, push_addr.second);
    }
    t_regs_table_[reg_no] = symbol;
    move_reg_no_to_order_end("t", reg_no);
    return "$t" + std::to_string(reg_no);
}

// @brief: load a symbol from stack to reg
// @pre: the symbol not in the regs
std::string MipsGenerator::assign_s_reg_require_load_from_memo(const std::string &symbol) {
    bool has_empty = false;
    int reg_no = -1;

    for (int i = 0; i < s_order_.size(); i++) {
        if (s_regs_table_[i].empty()) {
            has_empty = true;
            reg_no = i;
            break;
        }
    }

    if (!has_empty) {
        // todo: use dirty bit can save unnecessary save
        reg_no = s_order_[0];
        std::string pop_symbol_name = s_regs_table_[reg_no];
        save_to_memo("s", pop_symbol_name);
        s_regs_table_[reg_no] = "";
    }

    std::pair<int, std::string> push_addr = get_memo_addr(symbol);
    add_code("lw", "$s" + std::to_string(reg_no), push_addr.first, push_addr.second);

    s_regs_table_[reg_no] = symbol;
    move_reg_no_to_order_end("s", reg_no);
    return "$s" + std::to_string(reg_no);
}


void MipsGenerator::add_error(const std::string &error_msg) {
    std::cerr << "!error: " + error_msg << std::endl << std::endl;
}

// @brief: a nick function for add, sub, mul, div, sll
void MipsGenerator::add_code(const std::string &op, const std::string &dst, const std::string &src1, int src2) {
    add_code(op, dst, src1, std::to_string(src2));
}

void MipsGenerator::add_code(const std::string &code) {
    if (MIPS_DBG) {
        out_ << code << std::endl;
    } else {
        mips_codes_.push_back(code);
    }
}

// @brief: given four strings simply ,then generate the most simple instr
// @pre: add, sub, mul
void MipsGenerator::add_code(const std::string &op, const std::string &dst,
                             const std::string &src1, const std::string &src2) {
    std::string code = op + " " + dst + ", " + src1 + ", " + src2;
    if (!MIPS_DBG) {
        mips_codes_.push_back(code);
    } else {
        out_ << code << std::endl;
    }
}

// @brief:
void MipsGenerator::add_code(const std::string &op, const std::string &dst, const std::string &src1) {
    std::string code;
    if (op == "sw" || op == "lw" ||
        op == "div" || op == "mul" || op == "mult" || op == "multu" ||
        op == "move" || op == "la" || op == "li" || op == "not" ||
        op == "bgtz") {
        code = op + " " + dst + ", " + src1;
    } else {
        std::cout << "add_code doesn't support op: " + op << std::endl;
    }
    if (!MIPS_DBG) {
        mips_codes_.push_back(code);
    } else {
        out_ << code << std::endl;
    }
}

// @brief: this function is called when lw or sw
//         the base_addr should be
void MipsGenerator::add_code(const std::string op, const std::string &reg_name, int off, std::string base_addr) {
    if (op != "sw" && op != "lw") {
        add_error("expect lw or sw in memory access code");
    }
    std::string code = op;

    code += " ";
    code += reg_name;
    code += ", ";
    code += std::to_string(off);
    code += ("(" + base_addr + ")");
    if (!MIPS_DBG) {
        mips_codes_.push_back(code);
    } else {
        out_ << code << std::endl;
    }
}

// @brief: Color the conflict graph in the func_block
void MipsGenerator::color_func_graph() {
    for (FuncBlock &func_block: func_blocks_) {
        func_block.conflict_graph_.Color(reg_pool_.size());
        // output the Color
        ConflictGraph &graph = func_block.conflict_graph_;
        std::cout << func_block.func_name_ << std::endl;
        for (auto &symbol_reg_pair: graph.symbol_reg_pairs) {
            std::cout << symbol_reg_pair.first << ": " << symbol_reg_pair.second << std::endl;
        }
        std::cout << "----" << std::endl;
    }
}

void MipsGenerator::Translate() {

    color_func_graph();

    // use the global table in symbol table to assign memory in .data or in $gp

    add_code(".data");
    for (int i = 0; i < symbol_table_.strcons_.size(); i++) {
        std::string code = tab;
        code += ("str_" + std::to_string(i) + ":" + tab + ".asciiz ");
        code += ("\"" + symbol_table_.strcons_[i] + "\"");
        add_code(code);
    }
    add_code(".text");

    for (int i = 0; i < interm_codes_.size(); i++) {
        IntermCode &i_code = interm_codes_[i];
        add_code("");
        add_code("# " + interm_code_to_string(i_code, false));
        IntermOp op = i_code.op;
        std::string dst = i_code.dst, src1 = i_code.src1, src2 = i_code.src2;
        // FUNC_BEGIN
        // clear s_regs
        // store params into s_regs
        // do not care t_regs
        if (op == IntermOp::FUNC_BEGIN) {
            add_code("# the first function definition, may store the global variables");
            translate_func();
            return;
        }
            // add sub mul div
        else if (is_arith(op)) {
            // add dst src1 src2
            // assign reg for dst
            std::string dst_reg, src1_reg, src2_reg;
            if (op == IntermOp::ADD) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) + std::stoi(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", dst_reg, src2_reg, std::stoi(src1));
                    if (src2 != dst && src2[0] == '#' && !will_be_used_later(src2, i))
                        remove_from_reg(src2_reg);
                } else if (is_integer(src2)) {
                    src1_reg = get_reg_require_load_from_memo(src1);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", dst_reg, src1_reg, std::stoi(src2));
                    if (src1 != dst && src1[0] == '#' && !will_be_used_later(src1, i))
                        remove_from_reg(src1_reg);
                } else {
                    src1_reg = get_reg_require_load_from_memo(src1);
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", dst_reg, src1_reg, src2_reg);
                    if (src1 != dst && src1[0] == '#' && !will_be_used_later(src1, i))
                        remove_from_reg(src1_reg);
                    if (src2 != dst && src2[0] == '#' && !will_be_used_later(src2, i))
                        remove_from_reg(src2_reg);
                }
            }
                // SUB
            else if (op == IntermOp::SUB) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) - std::stoi(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    // add $a0, $zero, src1; sub dst $a0 src2
                    add_code("add", "$a0", "$zero", src1);
                    add_code("sub", dst_reg, "$a0", src2_reg);
                } else if (is_integer(src2)) {
                    int value = std::stoi(src2);
                    src1_reg = get_reg_require_load_from_memo(src1);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("addi", dst_reg, src1_reg, -value); // faster then sub
                } else {
                    src1_reg = get_reg_require_load_from_memo(src1);
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("sub", dst_reg, src1_reg, src2_reg);
                }
                if (src1 != dst && src1[0] == '#' && !will_be_used_later(src1, i)) remove_from_reg(src1_reg);
                if (src2 != dst && src2[0] == '#' && !will_be_used_later(src2, i)) remove_from_reg(src2_reg);
            }
                // MUL
            else if (op == IntermOp::MUL) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) * std::stoi(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    int value = std::stoi(src1);
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    if (is_2_pow(value)) {
                        int k = get_2_pow(value);
                        add_code("sll", dst_reg, src2_reg, k);
                    } else {
                        add_code("mul", dst_reg, src2_reg, src1);
                    }
                } else if (is_integer(src2)) {
                    int value = std::stoi(src2);
                    src1_reg = get_reg_require_load_from_memo(src1);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    if (is_2_pow(value)) {
                        int k = get_2_pow(value);
                        add_code("sll", dst_reg, src1_reg, k);
                    } else {
                        add_code("mul", dst_reg, src1_reg, src2);
                    }
                } else {
                    src1_reg = get_reg_require_load_from_memo(src1);
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("mul", dst_reg, src1_reg, src2_reg);
                }
                if (src1 != dst && src1[0] == '#' && !will_be_used_later(src1, i)) remove_from_reg(src1_reg);
                if (src2 != dst && src2[0] == '#' && !will_be_used_later(src2, i)) remove_from_reg(src2_reg);
            }
                // DIV
            else if (op == IntermOp::DIV) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) / std::stoi(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    // dst = num / symbol
                    // add $a1 $zero src1; div dst_reg, $a1, src2_reg
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", "$a1", "$0", src1);
                    add_code("div", "$a1", src2_reg);
                    add_code("mflo", dst_reg, "", "");
                } else if (is_integer(src2)) {
                    // MIPS won't judge src2 equals 0 or not, so it won't generate the label
                    // div a b 3
                    // div res, divend, divisor
                    src1_reg = get_reg_require_load_from_memo(src1);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    int divisor = std::stoi(src2);
                    if (can_be_div_opt(divisor)) {
                        if (is_2_pow(divisor)) {
                            int k = get_2_pow(divisor);
                            // dst = src1
                            // if src1 < 0 dst = src1 + (divisor -1) # 为负数，需要加上偏移量
                            // dst >> k
                            std::string label = "correct_shiftend_" + std::to_string(div_opt_times++);
                            add_code("add", dst_reg, src1_reg, "$0"); // if divend > 0, we use this
                            add_code("bgtz", src1_reg, label);
                            add_code("add", dst_reg, src1_reg, divisor - 1);
                            add_code(label + ":");
                            add_code("sra", dst_reg, dst_reg, k);
                        } else {
                            unsigned int multer, shifter;
                            std::pair<unsigned int, unsigned int> mult_shft = get_multer_and_shifter(std::stoi(src2));
                            multer = mult_shft.first;
                            shifter = mult_shft.second;
                            std::string val_reg = "$a0"; // temp result
                            std::string multer_reg = "$a1";
                            // a1 = multer
                            // HI, LO = mult src1, multer
                            // a0 = HI, a0 >> shifter # a0还差符号位就是答案
                            // dst = src1 >> 31 # sign_bit is in dst
                            // dst = a0 - dst
                            add_code("li", multer_reg, std::to_string(multer));
                            add_code("mult", src1_reg, multer_reg); // multer 乘上被除数
                            add_code("mfhi", val_reg, "", ""); // 取高位放到临时结果
                            if (shifter != 0)
                                add_code("sra", val_reg, val_reg, std::to_string(shifter)); // 完成右移，离真正差符号位

                            add_code("sra", dst_reg, src1_reg, "31"); // 求出符号位
                            add_code("sub", dst_reg, val_reg, dst_reg);
                        }
                    } else {
                        add_code("div", src1_reg, src2);
                        add_code("mflo", dst_reg, "", "");
                    }
                } else {
                    src1_reg = get_reg_require_load_from_memo(src1);
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("div", src1_reg, src2_reg);
                    add_code("mflo", dst_reg, "", "");
                }
                if (src1 != dst && src1[0] == '#' && !will_be_used_later(src1, i)) remove_from_reg(src1_reg);
                if (src2 != dst && src2[0] == '#' && !will_be_used_later(src2, i)) remove_from_reg(src2_reg);
            }
                // MOD
            else {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) % std::stoi(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    add_code("add", "$a1", "$zero", src1);
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("div", "$a1", src2_reg);
                    add_code("mfhi " + dst_reg);
                } else if (is_integer(src2)) {
                    // MOD a b 3
                    src1_reg = get_reg_require_load_from_memo(src1);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    int divisor = std::stoi(src2);
                    if (can_be_div_opt(divisor)) {
                        if (is_2_pow(divisor)) {
                            int k = get_2_pow(divisor);
                            std::string src1_copy = "$a0";
                            // a0做符号位，防止 dst_reg == src1_reg
                            // a0 = src1, copy src1
                            // dst = src1
                            // if a0 < 0, dst = -src1
                            // dst = dst & (divisor -1)
                            // if a0 < 0, dst = -dst
                            add_code("add", src1_copy, src1_reg, "$0");
                            add_code("add", dst_reg, src1_reg, "$0");
                            std::string div_ves_label = "div_opt_label_" + std::to_string(div_opt_times++);
                            add_code("bgtz", src1_reg, div_ves_label);
                            add_code("sub", dst_reg, "$0", src1_reg);
                            add_code(div_ves_label + ":");
                            //  now we can &
                            add_code("andi", dst_reg, dst_reg, divisor - 1);

                            std::string res_ves_label = "div_opt_label_" + std::to_string(div_opt_times++);
                            add_code("bgtz", src1_copy, res_ves_label);
                            add_code("sub", dst_reg, "$0", dst_reg);
                            add_code(res_ves_label + ":");
                        } else {
                            // 小心dst 和 src1 被分配到相同的寄存器
                            std::string divisor_reg = "$a0";
                            std::string multer_reg = "$a1";
                            std::string quotient_reg = "$a2";
                            std::string sign_reg = "$a3";
                            unsigned int multer, shifter;
                            std::pair<unsigned int, unsigned int> mul_shft = get_multer_and_shifter(divisor);
                            multer = mul_shft.first;
                            shifter = mul_shft.second;
                            // a2 = divisor
                            // a1 = multer
                            // HI, LO = src1 * a1
                            // a2 = HI
                            // sra a2, a2, shifter
                            // a3 = sign(src1)
                            // a2 = a2 - a3
                            // # now, src1 and dst never change, divisor reg(a0) is useless
                            // mul $a0, $a0, a2
                            // sub a,b , $a2
                            add_code("add", divisor_reg, "$0", std::to_string(divisor));
                            add_code("add", multer_reg, "$0", std::to_string(multer));
                            add_code("mult", src1_reg, multer_reg);
                            add_code("mfhi " + quotient_reg);
                            if (shifter != 0) {
                                add_code("sra", quotient_reg, quotient_reg,
                                         std::to_string(shifter)); // the real quotient
                            }

                            add_code("sra", sign_reg, src1_reg, "31"); // 符号位
                            add_code("sub", quotient_reg, quotient_reg, sign_reg); // the real res of quo
                            // now div res is in divisor
                            add_code("mul", divisor_reg, quotient_reg, divisor_reg);
                            add_code("sub", dst_reg, src1_reg, divisor_reg);
                        }
                    }
                        // can't be optimized
                    else {
                        std::string divisor_reg = "$a0";
                        add_code("add", divisor_reg, "$0", divisor);
                        add_code("div", src1_reg, divisor_reg);
                        add_code("mfhi " + dst_reg);
                    }
                } else {
                    src1_reg = get_reg_require_load_from_memo(src1);
                    src2_reg = get_reg_require_load_from_memo(src2);
                    dst_reg = get_reg_without_load_from_memo(dst);
                    add_code("div", src1_reg, src2_reg);
                    add_code("mfhi " + dst_reg);
                }
                if (src1 != dst && src1[0] == '#' && !will_be_used_later(src1, i)) remove_from_reg(src1_reg);
                if (src2 != dst && src2[0] == '#' && !will_be_used_later(src2, i)) remove_from_reg(src2_reg);
            }

        }

            // INIT ARR PTR
        else if (op == IntermOp::INIT_ARR_PTR) {
            // todo: ptr value do not need to load from stack
            if (dst == "des_7") {
                int i = 0;
            }

            std::string dst_reg = get_reg_require_load_from_memo(dst);
            std::pair<int, std::string> ptr_addr = get_memo_addr(dst);
            int arr_offset = ptr_addr.first + 4;
            add_code("add", dst_reg, ptr_addr.second, arr_offset);
        }
            // ARR_LOAD, fetch a value from array
            // ARR_LOAD var_1 arr_name idx
        else if (op == IntermOp::ARR_LOAD) {
            std::string dst_reg = get_reg_require_load_from_memo(dst);
            std::string arr_addr_reg = get_reg_require_load_from_memo(src1);
            if (is_integer(src2)) {
                // ARR_LOAD #tmp arr_name 5
                int element_off = 4 * std::stoi(src2);
                add_code("lw", dst_reg, element_off, arr_addr_reg);
            } else {
                // ARR_LOAD #tmp arr_name #tmp15
                std::string idx_reg = get_reg_require_load_from_memo(src2);
                add_code("sll", "$a0", idx_reg, 2); // the index offset in $a0 now
                add_code("add", "$a0", "$a0", arr_addr_reg); // the offset to pointer in $a0 now
                add_code("lw", dst_reg, 0, "$a0");
            }
            // src1 and src2 may be use in steps after, so don't release
        }
            // ARR_SAVE, save a value into memory
        else if (op == IntermOp::ARR_SAVE) {
            // todo: the param and var/const are different, can optimize
            // scr1 is index, src2 is value
            // index use $a0, value use $a1
            std::string off_reg = "$a0";
            std::string val_reg = "$a1";
            std::string src1_reg, src2_reg;
            std::pair<int, std::string> arr_addr = get_memo_addr(dst);
            std::string arr_addr_reg = get_reg_require_load_from_memo(dst);
            // ARR_SAVE arr_1 1 10
            if (is_integer(src1) && is_integer(src2)) {
                add_code("add", val_reg, "$0", src2);
                int element_off = 4 * std::stoi(src1);
                add_code("sw", val_reg, element_off, arr_addr_reg);
            } else if (is_integer(src1)) {
                // index/src1 is integer, val/src2 is a symbol
                src2_reg = get_reg_require_load_from_memo(src2);
                int element_off = 4 * std::stoi(src1);
                add_code("sw", src2_reg, element_off, arr_addr_reg);
            } else if (is_integer(src2)) {
                // index/src1 is symbol, value is integer
                src1_reg = get_reg_require_load_from_memo(src1);
                add_code("add", off_reg, "$zero", src1_reg); // $a0 = src1
                add_code("sll", off_reg, off_reg, 2); // $a0 *= 4
                add_code("add", off_reg, off_reg, arr_addr_reg); // $a0 += arr_addr_reg
                add_code("add", val_reg, "$zero", src2);
                add_code("sw", val_reg, 0, off_reg);
            } else {
                src1_reg = get_reg_require_load_from_memo(src1); // index
                src2_reg = get_reg_require_load_from_memo(src2); // value
                add_code("add", off_reg, "$zero", src1_reg); // $a0 = src1
                add_code("sll", off_reg, off_reg, 2); // $a0 *= 4
                add_code("add", off_reg, off_reg, arr_addr_reg); // $a0 += arr_addr
                add_code("sw", src2_reg, 0, off_reg);
            }
            // todo: check if src2 can be release
            // if (src2[0] == '#') remove_from_reg(src2_reg);
        }
            // undefined instruction
        else {
            add_error("undefined instruction");
        }
    }
}

// @brief: translate the function of the program
// @pre: the first statement is "PREPARE_CALL"
void MipsGenerator::translate_func() {
    bool init_main = false;
    int i = 0;
    for (; i < interm_codes_.size(); i++) {
        IntermCode &i_code = interm_codes_[i];
        if (i_code.op != IntermOp::FUNC_BEGIN) {
            continue;
        } else {
            break;
        }
    }
    // now i is pointed to FUNC_BEGIN
    // save global variables to memo
    remove_s_regs_save_to_memo();
    // now let's begin translate
    for (; i < interm_codes_.size(); i++) {
        IntermCode &i_code = interm_codes_[i];
        add_code("");
        add_code("# " + interm_code_to_string(i_code, false));
        IntermOp op = i_code.op;
        std::string dst = i_code.dst, src1 = i_code.src1, src2 = i_code.src2;

        // FUNC_BEGIN
        if (op == IntermOp::FUNC_BEGIN) {
            if (!init_main) { // save a stack for main function
                add_code("addi $sp, $sp, -" + std::to_string(symbol_table_.GetFuncStackSize("main")));
                add_code("j main");
                init_main = true;
            }
            cur_func_name_ = dst;
            add_code(dst + " :");
            // if params in the reg_pairs and has a reg assign, init them
            std::vector<std::string> params = symbol_table_.GetFuncParams(dst);
            for (std::string param_name: params) {
                std::pair<bool, int> search_reg_res = search_symbol_reg(param_name);
                if (search_reg_res.first && search_reg_res.second != -1) {
                    // store to memo
                    int reg_id = search_reg_res.second;
                    std::pair<int, std::string> addr = get_memo_addr(param_name);
                    add_code("lw", reg_pool_[reg_id], addr.first, addr.second);
                } else {
                    // pass
                }
            }
        }
            // RET
        else if (op == IntermOp::RET) {
            if (!dst.empty()) {
                if (is_integer(dst)) {
                    add_code("add", "$v0", "$0", dst);
                } else {
                    std::pair<int, std::string> addr_res = get_running_addr(dst);
                    if (addr_res.first != -1) {
                        add_code("lw", "$v0", addr_res.first, addr_res.second);
                    } else {
                        add_code("move", "$v0", addr_res.second);
                    }
                }

            }
            if (cur_func_name_ == "main") {
                add_code("li $v0, 10");
                add_code("syscall");
            } else {
                add_code("jr $ra");
            }
        }
            // FUNC_END
        else if (op == IntermOp::FUNC_END) {
            if (cur_func_name_ != "main") {
                add_code("jr $ra");
            }
        }
            // PREPARE_CALL
        else if (op == IntermOp::PREPARE_CALL) {
            // $sp minus (context and func size), it's denoted "frame size"
            // then save the context: ra, sp, s_res, t_res
            callee_name_stack_.push_back(cur_callee_name_);
            cur_callee_name_ = dst;
            int func_stack_size = symbol_table_.GetFuncStackSize(dst);
            int frame_size = context_size + func_stack_size;
            frame_size_stack_.push_back(frame_size);
            add_code("addi", "$sp", "$sp",  -frame_size);
        }
            // PUSH_ARR
        else if (op == IntermOp::PUSH_ARR) {
            // e.g. PUSH_ARR @Tmp_12 4
            // MIPS:
            //     add $a0 $sp|$gp off
            //     sw $a0 param_off($sp)
            int param_off = std::stoi(src1) * 4;
            std::pair<int, std::string> addr = get_running_addr(dst);
            // the dst in reg
            if (addr.first == -1) {
                add_code("sw", addr.second, param_off, "$sp");
            }
                // the dst in memo, put to $a0
            else {
                std::string tmp_val_reg = "$a0";
                add_code("lw", tmp_val_reg, addr.first, addr.second);
                add_code("sw", tmp_val_reg, param_off, "$sp");
            }
        }
            // PUSH_VAL
        else if (op == IntermOp::PUSH_VAL) {
            // number or symbol? reg or memory? global or local?
            int param_off = std::stoi(src1) * 4;
            if (is_integer(dst)) {
                // PUSH_VAL 1 1
                //      add $a0 1 $0
                //      sw $a0 param_off($sp)
                add_code("add", "$a0", "$zero", dst);
                add_code("sw", "$a0", param_off, "$sp");
            }
                // VAL is symbol
            else {
                std::pair<int, std::string> addr = get_running_addr(dst);
                // the dst in reg
                if (addr.first == -1) {
                    add_code("sw", addr.second, param_off, "$sp");
                }
                    // the dst in memo, put to $a0
                else {
                    std::string tmp_val_reg = "$a0";
                    add_code("lw", tmp_val_reg, addr.first, addr.second);
                    add_code("sw", tmp_val_reg, param_off, "$sp");
                }
            }
        }
            // CALL FOO
        else if (op == IntermOp::CALL) {
            // global symbols not in regs, so we don't care
            // we don't save sp into the context, we use the frame_size to remember how much to return
            int func_stack_size = symbol_table_.GetFuncStackSize(dst);
            add_code("sw $ra, " + std::to_string(ra_off + func_stack_size) + "($sp)");
            std::vector<int> saved_s_reg_no = {};
            std::vector<int> saved_t_reg_no = {};
            // save the symbols in reg_pool to memo
            std::pair<bool, int> search_res = search_func_block_by_name(cur_func_name_);
            if (!search_res.first) add_error("func block not found in CALL");
            FuncBlock &func_block = func_blocks_[search_res.second];
            std::vector<int> used_regs = func_block.GetUsedRegs();

            // todo: track symbols that being used by the reg pool
            for (int i = 0; i < used_regs.size(); i++) {
                std::string reg_name = reg_pool_[used_regs[i]];
                if (reg_name[1] == 's') {
                    int s_id = reg_name[2] - '0';
                    add_code("sw", reg_name, func_stack_size + s_regs_off + 4 * s_id, "$sp");
                } else { // 't' reg
                    int t_id = reg_name[2] - '0';
                    add_code("sw", reg_name, func_stack_size + t_regs_off + 4 * t_id, "$sp");
                }
            }

            add_code("jal " + dst);
            add_code("lw", "$ra", ra_off + func_stack_size, "$sp");


            for (int i = 0; i < used_regs.size(); i++) {
                std::string reg_name = reg_pool_[used_regs[i]];
                if (reg_name[1] == 's') {
                    int s_id = reg_name[2] - '0';
                    add_code("lw", reg_name, func_stack_size + s_regs_off + 4 * s_id, "$sp");
                } else { // 't' reg
                    int t_id = reg_name[2] - '0';
                    add_code("lw", reg_name, func_stack_size + t_regs_off + 4 * t_id, "$sp");
                }
            }

            add_code("add", "$sp", "$sp", *(frame_size_stack_.end() - 1));
            frame_size_stack_.pop_back();
            if (!callee_name_stack_.empty()) {
                cur_callee_name_ = callee_name_stack_.back();
                callee_name_stack_.pop_back();
            }
        }
            // GETINT
        else if (op == IntermOp::GETINT) {
            add_code("li $v0, 5");
            add_code("syscall");
            std::pair<int, std::string> addr = get_running_addr(dst);
            if (addr.first == -1) {
                add_code("move", addr.second, "$v0");
            } else {
                std::string val_reg = "$a0";
                add_code("move", val_reg, "$v0");
                add_code("sw", val_reg, addr.first, addr.second);
            }
        }
            // PRINT
        else if (op == IntermOp::PRINT) {
            if (src1 == "str") {
                int idx = symbol_table_.find_str_idx(dst);
                add_code("la", "$a0", "str_" + std::to_string(idx));
                add_code("li $v0, 4");
            } else {
                std::string val_reg = "$a0";
                if (is_integer(dst)) {
                    add_code("li", val_reg, dst);
                } else {
                    std::pair<int, std::string> addr = get_running_addr(dst);
                    if (addr.first == -1) {
                        add_code("move", val_reg, addr.second);
                    } else {
                        add_code("lw", val_reg, addr.first, addr.second);
                    }
                }
                add_code("li $v0, 1");
            }
            add_code("syscall");
        }
            // LABEL
        else if (op == IntermOp::LABEL) {
            add_code(dst + ":");
        }
            // Jump
        else if (op == IntermOp::JUMP) {
            add_code("j " + dst);
        }
            // BEQ
            // @pre: the src2 must be 0
        else if (op == IntermOp::BEQ) {
            if (!(src2 == "0")) add_error("BEQ src2 is not 0");

            if (is_integer(src1) && is_integer(src2)) {
                if (std::stoi(src1) == std::stoi(src2)) {
                    add_code("j " + dst);
                } else {
                    add_code("# two src not equal, ignore jump");
                }
            } else {
                std::pair<int, std::string> addr = get_running_addr(src1);
                if (addr.first == -1) {
                    add_code("beq", addr.second, "$0", dst);
                } else {
                    // load to $a0
                    add_code("lw", "$a0", addr.first, addr.second);
                    add_code("beq", "$a0", "$0", dst);
                }


            }
        }
            // BNE
        else if (op == IntermOp::BNE) {
            if (!(src2 == "0")) add_error("BNE src2 is not 0");
            if (is_integer(src1) && is_integer(src2)) {
                if (std::stoi(src1) != std::stoi(src2)) {
                    add_code("j " + dst);
                } else {
                    add_code("# two src equal, ignore jump");
                }
            } else {
                std::pair<int, std::string> addr = get_running_addr(src1);
                if (addr.first == -1) {
                    add_code("bne", addr.second, "$0", dst);
                } else {
                    // load to $a0
                    add_code("lw", "$a0", addr.first, addr.second);
                    add_code("bne", "$a0", "$0", dst);
                }
            }
        }
            // add sub mul div
        else if (is_arith(op)) {
            // add dst src1 src2
            // assign reg for dst
            std::pair<int, std::string> dst_addr = get_running_addr(dst);
            std::string dst_reg, src1_reg, src2_reg;

            if ((op == IntermOp::MUL || op == IntermOp::ADD) &&
                is_integer(src1) && !is_integer(src2)) {
                std::string temp = src2;
                src2 = src1;
                src1 = temp;
            }
            // ADD
            if (op == IntermOp::ADD) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) + std::stoi(src2);
                    dst_reg = get_reg_without_fail_load(dst, "$a0");
                    add_code("add", dst_reg, "$0", res);
                } else if (is_integer(src2)) {
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    if (src1 == dst) {
                        dst_reg = src1_reg;
                    } else {
                        dst_reg = get_reg_without_fail_load(dst, "$a1");
                    }
                    add_code("add", dst_reg, src1_reg, src2);
                } else {
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    if (src2 == src1) {
                        src2_reg = src1_reg;
                    } else {
                        src2_reg = get_reg_with_fail_load(src2, "$a1");
                    }
                    if (src1 == dst) {
                        dst_reg = src1_reg;
                    } else if (src2 == dst) {
                        dst_reg = src2_reg;
                    } else {
                        dst_reg = get_reg_without_fail_load(dst, "$a2");
                    }
                    add_code("add", dst_reg, src1_reg, src2_reg);
                }
                if (dst_reg[1] == 'a') {
                    add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
                }
            }
                // SUB
            else if (op == IntermOp::SUB) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) - std::stoi(src2);
                    dst_reg = get_reg_without_fail_load(dst, "$a0");
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    // SUB dst, 5, src2
                    // add $a0, $zero, src1
                    // sub dst $a0 src2
                    add_code("add", "$a0", "$zero", src1);
                    src2_reg = get_reg_with_fail_load(src2, "$a1");
                    dst_reg = get_reg_without_fail_load(dst, "$a2");
                    add_code("sub", dst_reg, "$a0", src2_reg);
                } else if (is_integer(src2)) {
                    int value = std::stoi(src2);
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    if (src1 == dst) {
                        dst_reg = src1_reg;
                    } else {
                        dst_reg = get_reg_without_fail_load(dst, "$a1");
                    }
                    add_code("addi", dst_reg, src1_reg, -value);
                } else {
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    if (src2 == src1) {
                        src2_reg = src1_reg;
                    } else {
                        src2_reg = get_reg_with_fail_load(src2, "$a1");
                    }
                    if (dst == src1) {
                        dst_reg = src1_reg;
                    } else if (dst == src2) {
                        dst_reg = src2_reg;
                    } else {
                        dst_reg = get_reg_without_fail_load(dst, "$a2");
                    }
                    add_code("sub", dst_reg, src1_reg, src2_reg);
                }
                if (dst_reg[1] == 'a') {
                    add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
                }
            }
                // MUL
            else if (op == IntermOp::MUL) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) * std::stoi(src2);
                    dst_reg = get_reg_without_fail_load(dst, "$a0");
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src2)) {
                    int value = std::stoi(src2);
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    if (src1 == dst) {
                        dst_reg = src1_reg;
                    } else {
                        dst_reg = get_reg_without_fail_load(dst, "$a1");
                    }

                    if (is_2_pow(value)) {
                        int k = get_2_pow(value);
                        add_code("sll", dst_reg, src1_reg, k);
                    }  else {
                        add_code("mul", dst_reg, src1_reg, src2);
                    }
                } else {
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    if (src1 == src2) {
                        src2_reg = src1_reg;
                    } else {
                        src2_reg = get_reg_with_fail_load(src2, "$a1");
                    }

                    if (dst == src1) {
                        dst_reg = src1_reg;
                    } else if (dst == src2) {
                        dst_reg = src2_reg;
                    } else {
                        dst_reg = get_reg_without_fail_load(dst, "$a2");
                    }
                    add_code("mul", dst_reg, src1_reg, src2_reg);
                }
                if (dst_reg[1] == 'a') add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
            }
                // DIV
            else if (op == IntermOp::DIV) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) / std::stoi(src2);
                    dst_reg = get_reg_without_fail_load(dst, "$a0");
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    // dst = num / symbol
                    // add $a1 $zero src1
                    // div dst_reg, $a1, src2_reg
                    add_code("add", "$a0", "$0", src1);
                    src2_reg = get_reg_with_fail_load(src2, "$a1");
                    dst_reg = get_reg_without_fail_load(dst, "$a2");
                    add_code("div", "$a0", src2_reg);
                    add_code("mflo", dst_reg, "", "");
                } else if (is_integer(src2)) {
                    // MIPS won't judge src2 equals 0 or not, so it won't generate the label
                    // div a b 3
                    // div res, divend, divisor
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    dst_reg = get_reg_without_fail_load(dst, "$a1");
                    int divisor = std::stoi(src2);
                    // 只允许使用a2 和 a3
                    if (can_be_div_opt(divisor)) {
                        if (is_2_pow(divisor)) {
                            int k = get_2_pow(divisor);
                            std::string label = "correct_shiftend_" + std::to_string(div_opt_times++);
                            add_code("add", dst_reg, src1_reg, "$0"); // if divend > 0, we use this
                            add_code("bgtz", src1_reg, label);
                            add_code("add", dst_reg, src1_reg, divisor-1);
                            add_code(label + ":");
                            add_code("sra", dst_reg, dst_reg, k);
                        } else {
                            unsigned int multer, shifter;
                            std::pair<unsigned int, unsigned int> mult_shft = get_multer_and_shifter(std::stoi(src2));
                            multer = mult_shft.first;
                            shifter = mult_shft.second;
                            std::string val_reg = "$a2"; // temp result
                            std::string multer_reg = "$a3";
                            // a3 = multer
                            // HI, LO = src1 * a3
                            // a2 = HI
                            // a2 >> shifter
                            // dst = src1 >> 31
                            // dst = a2 - dst
                            add_code("li", multer_reg, std::to_string(multer));
                            add_code("mult", src1_reg, multer_reg); // multer 乘上被除数
                            add_code("mfhi", val_reg, "", ""); // 取高位放到临时结果
                            if (shifter != 0)
                                add_code("sra", val_reg, val_reg, std::to_string(shifter)); // 完成右移，离真正差符号位

                            add_code("sra", dst_reg, src1_reg, "31"); // 求出符号位
                            add_code("sub", dst_reg, val_reg, dst_reg);
                        }
                    } else {
                        add_code("div", dst_reg, src1_reg, src2);
                    }
                } else {
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    src2_reg = get_reg_with_fail_load(src2, "$a1");
                    dst_reg = get_reg_without_fail_load(dst, "$a2");
                    add_code("div", src1_reg, src2_reg);
                    add_code("mflo", dst_reg, "", "");
                }
                if (dst_reg[1] == 'a') add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
            }
                // MOD
            else {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) % std::stoi(src2);
                    dst_reg = get_reg_without_fail_load(dst, "$a0");
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    add_code("add", "$a0", "$zero", src1);
                    src2_reg = get_reg_with_fail_load(src2, "$a1");
                    dst_reg = get_reg_without_fail_load(dst, "$a2");
                    add_code("div", "$a0", src2_reg);
                    add_code("mfhi " + dst_reg);
                }
                    // MOD a b 3
                else if (is_integer(src2)) {
                    std::string src1_back_reg = "$a0", dst_back_reg = "$a1";
                    src1_reg = get_reg_with_fail_load(src1, src1_back_reg);
                    dst_reg = get_reg_without_fail_load(dst, dst_back_reg);
                    int divisor = std::stoi(src2);
                    if (can_be_div_opt(divisor)) {
                        // 2 power
                        if (is_2_pow(divisor)) {
                            int k = get_2_pow(divisor);
                            std::string src1_copy = "$a2";
                            // a2做符号位，防止 dst_reg == src1_reg
                            // a2 = src1, copy src1
                            // dst = src1
                            // if a2 < 0, dst = -src1
                            // dst = dst & (divisor -1)
                            // if a2 < 0, dst = -dst
                            add_code("add", src1_copy, src1_reg, "$0");
                            add_code("add", dst_reg, src1_reg, "$0");
                            std::string div_ves_label = "div_opt_label_" + std::to_string(div_opt_times++);
                            add_code("bgtz", src1_reg, div_ves_label);
                            add_code("sub", dst_reg, "$0", src1_reg);
                            add_code(div_ves_label + ":");
                            //  now we can &
                            add_code("andi", dst_reg, dst_reg, divisor - 1);
                            std::string res_ves_label = "div_opt_label_" + std::to_string(div_opt_times++);
                            add_code("bgtz", src1_copy, res_ves_label);
                            add_code("sub", dst_reg, "$0", dst_reg);
                            add_code(res_ves_label + ":");
                        }
                            // not 2 power
                        else {
                            // 小心dst 和 src1 被分配到相同的寄存器
                            std::string divisor_reg = "$a2";
                            std::string multer_reg = "$a3";
                            std::string quotient_reg = "$a3";
                            std::string sign_reg = "$a2";
                            unsigned int multer, shifter;
                            std::pair<unsigned int, unsigned int> mul_shft = get_multer_and_shifter(divisor);
                            multer = mul_shft.first;
                            shifter = mul_shft.second;

                            // a3 = multer
                            // HI, LO = src1 * a3
                            // a3 = HI
                            // sra a3, a3, shifter
                            // a2 = sign(src1)
                            // a3 = a3 - a2
                            // # now, src1 and dst never change, divisor reg(a0) is useless
                            // a2 = divisor
                            // mul $a2, $a3, a2
                            // sub a,b , $a2
                            add_code("add", multer_reg, "$0", std::to_string(multer));
                            add_code("mult", src1_reg, multer_reg);
                            add_code("mfhi " + quotient_reg);
                            if (shifter != 0) {
                                add_code("sra", quotient_reg, quotient_reg,
                                         std::to_string(shifter)); // the real quotient
                            }

                            add_code("sra", sign_reg, src1_reg, "31"); // 符号位
                            add_code("sub", quotient_reg, quotient_reg, sign_reg); // the real res of quo
                            // now div res is in divisor
                            add_code("add", divisor_reg, "$0", std::to_string(divisor));
                            add_code("mul", divisor_reg, quotient_reg, divisor_reg);
                            add_code("sub", dst_reg, src1_reg, divisor_reg);
                        }
                    } else {
                        std::string divisor_reg = "$a2";
                        add_code("add", divisor_reg, "$0", divisor);
                        add_code("div", src1_reg, divisor_reg);
                        add_code("mfhi " + dst_reg);
                    }
                }
                    // MOD a b c
                else {
                    src1_reg = get_reg_with_fail_load(src1, "$a0");
                    src2_reg = get_reg_with_fail_load(src2, "$a1");
                    dst_reg = get_reg_without_fail_load(dst, "$a2");
                    add_code("div", src1_reg, src2_reg);
                    add_code("mfhi " + dst_reg);
                }
                if (dst_reg[1] == 'a') add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
            }
        }
            // ==, !=, <, <=, >, >=
        else if (is_cmp(op)) {
            std::string dst_reg, src1_reg, src2_reg;
            std::string src1_back_reg = "$a0", src2_back_reg = "$a1", dst_back_reg = "$a2";
            std::string instr = interm_op_to_instr.find(op)->second;
            std::pair<int, std::string> dst_addr = get_running_addr(dst);
            if (is_integer(src1) && is_integer(src2)) {
                int res = 0;
                if (op == IntermOp::EQ) res = (std::stoi(src1) == std::stoi(src2));
                else if (op == IntermOp::NEQ) res = (std::stoi(src1) != std::stoi(src2));
                else if (op == IntermOp::LSS) res = (std::stoi(src1) < std::stoi(src2));
                else if (op == IntermOp::LEQ) res = (std::stoi(src1) <= std::stoi(src2));
                else if (op == IntermOp::GRE) res = (std::stoi(src1) > std::stoi(src2));
                else res = (std::stoi(src1) >= std::stoi(src2));
                dst_reg = get_reg_without_fail_load(dst, dst_back_reg);
                add_code("add", dst_reg, "$0", res);
            } else if (is_integer(src1)) {
                src2_reg = get_reg_with_fail_load(src2, src2_back_reg);
                dst_reg = get_reg_without_fail_load(dst, dst_back_reg);
                // slt dst 5 src2 -> sgt dst src2 5
                if (instr == "slt") { // <
                    instr = "sgt";
                } else if (instr == "sle") { // <=
                    instr = "sge";
                } else if (instr == "sgt") { // >
                    instr = "slti";
                } else if (instr == "sge") { // >=
                    instr = "sle";
                } else {}
                add_code(instr, dst_reg, src2_reg, src1);
            } else if (is_integer(src2)) {
                if (instr == "slt") instr = "slti";
                src1_reg = get_reg_with_fail_load(src1, src1_back_reg);
                dst_reg = get_reg_without_fail_load(dst, dst_back_reg);
                add_code(instr, dst_reg, src1_reg, src2);
            } else {
                src1_reg = get_reg_with_fail_load(src1, src1_back_reg);
                src2_reg = get_reg_with_fail_load(src2, src2_back_reg);
                dst_reg = get_reg_without_fail_load(dst, dst_back_reg);
                add_code(instr, dst_reg, src1_reg, src2_reg);
            }
            if (dst_reg[1] == 'a') add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
        } else if (op == IntermOp::INIT_ARR_PTR) {
            std::pair<int, std::string> dst_addr = get_running_addr(dst);
            std::string dst_reg = get_reg_without_fail_load(dst, "$a0");
            std::pair<int, std::string> ptr_addr = get_memo_addr(dst);
            int arr_offset = ptr_addr.first + 4;
            add_code("add", dst_reg, ptr_addr.second, arr_offset);
            if (dst_reg[1] == 'a') add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
        }
            // ARR_LOAD, fetch a value from array, assign the value to a variable
            // ARR_LOAD var_1 arr_name idx
        else if (op == IntermOp::ARR_LOAD) {
            std::pair<int, std::string> dst_addr = get_running_addr(dst);
            std::string arr_addr_reg = get_reg_with_fail_load(src1, "$a0");
            std::string dst_reg = get_reg_without_fail_load(dst, "$a2");
            if (is_integer(src2)) {
                // ARR_LOAD #tmp arr_name 5
                int element_off = 4 * std::stoi(src2);
                add_code("lw", dst_reg, element_off, arr_addr_reg);
            } else {
                // ARR_LOAD #tmp arr_name #tmp15
                std::string idx_reg = get_reg_with_fail_load(src2, "$a1");
                add_code("sll", "$a2", idx_reg, 2); // the index offset in $a2 now
                add_code("add", "$a2", "$a2", arr_addr_reg); // the offset to pointer in $a2 now
                add_code("lw", dst_reg, 0, "$a2");
            }
            if (dst_reg[1] == 'a') add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
        }
            // ARR_SAVE, save a value into memory
            // ARR_SAVE arr_name, index, value
        else if (op == IntermOp::ARR_SAVE) {
            // scr1 is index, src2 is value
            // index use $a0, value use $a1
            std::string off_reg = "$a0";
            std::string val_reg = "$a1";
            std::string dst_back_reg = "$a2";
            std::string src2_back_reg = "$a3", src1_back_reg = "$a0";

            std::string src1_reg, src2_reg;
            std::pair<int, std::string> arr_addr = get_memo_addr(dst);
            std::string arr_addr_reg = get_reg_with_fail_load(dst, dst_back_reg);
            // ARR_SAVE arr_1 1 10
            if (is_integer(src1) && is_integer(src2)) {
                add_code("add", val_reg, "$0", src2);
                int element_off = 4 * std::stoi(src1);
                add_code("sw", val_reg, element_off, arr_addr_reg);
            } else if (is_integer(src1)) {
                // index/src1 is integer, val/src2 is a symbol
                src2_reg = get_reg_with_fail_load(src2, src2_back_reg);
                int element_off = 4 * std::stoi(src1);
                add_code("sw", src2_reg, element_off, arr_addr_reg);
            } else if (is_integer(src2)) {
                // index/src1 is symbol, value is integer
                src1_reg = get_reg_with_fail_load(src1, off_reg); // index in reg_pool or a0
                add_code("add", off_reg, "$zero", src1_reg); // $a0 = src1/index
                add_code("sll", off_reg, off_reg, 2); // $a0 *= 4
                add_code("add", off_reg, off_reg, arr_addr_reg); // $a0 += arr_addr_reg
                add_code("add", val_reg, "$zero", src2);
                add_code("sw", val_reg, 0, off_reg);
            } else {
                src1_reg = get_reg_with_fail_load(src1, src1_back_reg); // index
                if (src2 == src1) {
                    src2_reg = src1_reg;
                } else {
                    src2_reg = get_reg_with_fail_load(src2, src2_back_reg); // value
                }
                add_code("add", off_reg, "$zero", src1_reg); // $a0 = src1
                add_code("sll", off_reg, off_reg, 2); // $a0 *= 4
                add_code("add", off_reg, off_reg, arr_addr_reg); // $a0 += arr_addr
                add_code("sw", src2_reg, 0, off_reg);
            }
        }
            // NOT dst, src1
            // @pre: src1 is a symbol
        else if (op == IntermOp::NOT) {
            std::pair<int, std::string> dst_addr = get_running_addr(dst);
            std::string src1_reg = get_reg_with_fail_load(src1, "$a0");
            std::string dst_reg = get_reg_without_fail_load(dst, "$a1");
            add_code("seq", dst_reg, src1_reg, "0");
            if (dst_reg[1] == 'a') add_code("sw", dst_reg, dst_addr.first, dst_addr.second);
        } else {
            add_error("undefined op");
        }
    }
}


// @brief:
// @exception: may not find this symbol
std::pair<bool, int> MipsGenerator::search_symbol_reg(std::string symbol) {
    std::pair<bool, int> search_res = search_func_block_by_name(cur_func_name_);
    if (!search_res.first) add_error("func " + cur_func_name_ + " not found in search_symbol_reg()");
    FuncBlock &func_block = func_blocks_[search_res.second];
    std::pair<bool, int> search_reg_res = func_block.SearchSymbolReg(symbol);
    return search_reg_res;
}

std::pair<bool, int> MipsGenerator::search_func_block_by_name(const std::string &func_name) {
    for (int i = 0; i < func_blocks_.size(); i++) {
        auto &func_block = func_blocks_[i];
        if (func_block.func_name_ == func_name) {
            return std::make_pair(true, i);
        } else {
            continue;
        }
    }
    return std::make_pair(false, -1);
}

// @brief: check the symbol will be used after interm_codes[i] in the same block,
//         to check if a temp symbol can be released
bool MipsGenerator::will_be_used_later(const std::string &symbol, int i) {
    // for those temp generated in the long expression like: a = b + c * d / e - f % d
    // the common may spread it to other code in the same block
    // end marks: FUNC_END, FUNC_BEGIN, Label, JUMP, BNE, BEQ
    i += 1;
    if (i >= interm_codes_.size()) return false;
    bool will_be_used = false;
    while (i < interm_codes_.size()) {
        IntermCode &code = interm_codes_[i];
        IntermOp op = code.op;
        if (op == IntermOp::FUNC_BEGIN || op == IntermOp::FUNC_END ||
            op == IntermOp::LABEL || op == IntermOp::JUMP ||
            op == IntermOp::BNE || op == IntermOp::BEQ) {
            return will_be_used;
        }

        if (code.src1 == symbol || code.src2 == symbol) {
            will_be_used = true;
            return true;
        } else {
            i++;
        }
    }
    return will_be_used;
}

