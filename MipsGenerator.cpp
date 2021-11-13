//
// Created by WYSJ6174 on 2021/11/8.
//

#include "MipsGenerator.h"

#define MIPS_DBG true

MipsGenerator::MipsGenerator(SymbolTable &symbol_table, std::vector<IntermCode> &interm_codes, std::ofstream &out) :
        symbol_table_(symbol_table), interm_codes_(interm_codes), out_(out) {

}

// @brief: given a symbol, return its offset to its base pointer $sp or $gp
//         we have already considered the function call stack size
//         so u don't need to add the frame size back
// @note: in PUSH_VAL, the offset need to change, because the sp has been subtracted
// @exec: may can find the symbol in symbol table
// @retval: <int offset, string pointer>, like <4, "($gp)">
std::pair<int, std::string> MipsGenerator::get_memo_addr(const std::string &symbol) {
    std::pair<bool, TableEntry *> table_search_res =
            symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, symbol);
    if (!table_search_res.first) add_error("symbol can't be found in symbol table");
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

// @brief: write back to memory, only generate the MIPS code
// @pre: this symbol can be found in t-regs table or s-regs table
void MipsGenerator::save_to_memo(const std::string &table_name, const std::string &symbol) {
    if (table_name != "s" && table_name != "t") add_error("table name error");
    int reg_no = -1;
    if (table_name == "s") {
        for (int i = 0; i < s_fifo_order_.size(); i++) {
            if (s_regs_table_[i] == symbol) {
                reg_no = i;
                break;
            }
        }
    } else {
        for (int i = 0; i < t_regs_table_.size(); i++) {
            if (t_regs_table_[i] == symbol) {
                reg_no = i;
                break;
            }
        }
    }
    std::pair<int, std::string> pop_addr = get_memo_addr(symbol);
    add_code("sw", "$" + table_name + std::to_string(reg_no), pop_addr.first, pop_addr.second);
}


// @brief: load a symbol to a reg without writing back, it's dangerous
void MipsGenerator::load_to_reg(std::string symbol, std::string reg) {
    if (is_integer(symbol)) {
        add_code("add", reg, "$zero", symbol);
    } else {
        std::pair<bool, std::string> search_res = search_in_st_regs(symbol);
        if (search_res.first) {
            add_code("move", reg, search_res.second);
        } else {
            std::pair<int, std::string> symbol_addr = get_memo_addr(symbol);
            add_code("lw", reg, symbol_addr.first, symbol_addr.second);
        }
    }

}

// @brief: symbol need to in a reg,
//         so search in regs, if already in, just return,
//         if not, assign a reg for it
// @note: before u use this function,
//        make sure this symbol really should be put into reg,
//        because
std::string MipsGenerator::get_a_reg_for(std::string symbol) {
    if (symbol == "%RET") return "$v0";
    std::pair<bool, std::string> search_res = search_in_st_regs(symbol);
    if (search_res.first) {
        int reg_no = search_res.second[2] - '0';
        // adjust the fifo
        if (search_res.second[1] == 's') {
            int fifo_idx = std::find(s_fifo_order_.begin(), s_fifo_order_.end(), reg_no)-s_fifo_order_.begin();
            for (int i = fifo_idx + 1; i < s_fifo_order_.size(); i++) {
                s_fifo_order_[i - 1] = s_fifo_order_[i];
            }
            s_fifo_order_.back() = reg_no;
        } else {
            int fifo_idx = std::find(t_fifo_order_.begin(), t_fifo_order_.end(), reg_no)-t_fifo_order_.begin();
            for (int i = fifo_idx + 1; i < t_fifo_order_.size(); i++) {
                t_fifo_order_[i - 1] = t_fifo_order_[i];
            }
            t_fifo_order_.back() = reg_no;
        }
        return search_res.second;
    } else {
        if (symbol[0] == '#') {
            return assign_t_reg(symbol);
        } else {
            return assign_s_reg(symbol);
        }
    }
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
    for (int i: t_fifo_order_) {
        if (t_regs_table_[i].empty()) {
            has_empty = true;
            reg_no = i;
            break;
        }
    }
    auto it = std::find(write_back_symbols_.begin(), write_back_symbols_.end(), symbol);
    if (it != write_back_symbols_.end()) need_load = true;

    if (!has_empty) {
        // pop the first of fifo order, move it to the end
        // maybe assign in a prepared call, check the frame_stack_size_
        reg_no = t_fifo_order_[0];
        std::string pop_symbol_name = t_regs_table_[reg_no];
        save_to_memo("t", pop_symbol_name);
        for (int i = 1; i < t_fifo_order_.size(); i++) {
            t_fifo_order_[i - 1] = t_fifo_order_[i];
        }
        t_fifo_order_.back() = reg_no;
        t_regs_table_[reg_no] = "";
        write_back_symbols_.push_back(pop_symbol_name);
    }

    // now the reg_no is correct
    if (need_load) {
        std::pair<int, std::string> push_addr = get_memo_addr(symbol);
        add_code("lw", "$t" + std::to_string(reg_no), push_addr.first, push_addr.second);
    }
    t_regs_table_[reg_no] = symbol;
    return "$t" + std::to_string(reg_no);
}

// @brief: assign an s-reg for the symbol and load the symbol from stack
//         so u don't need to reload after calling this function
// @pre: the symbol not in the regs
std::string MipsGenerator::assign_s_reg(std::string symbol) {
    bool has_empty = false;
    int reg_no = -1;

    for (int i = 0; i < s_fifo_order_.size(); i++) {
        if (s_regs_table_[i].empty()) {
            has_empty = true;
            reg_no = i;
            break;
        }
    }

    if (!has_empty) {
        // todo: use dirty bit can save unnecessary save
        reg_no = s_fifo_order_[0];
        std::string pop_symbol_name = s_regs_table_[reg_no];
        std::pair<bool, TableEntry*> search_pop = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, pop_symbol_name);
        if (search_pop.second->data_type == DataType::INT_ARR) {
            // array addr do not need to write back
        } else {
            save_to_memo("s", pop_symbol_name);
        }
        // maintain the table and fifo_order
        for (int i = 1; i < s_fifo_order_.size(); i++) {
            s_fifo_order_[i - 1] = s_fifo_order_[i];
        }
        s_fifo_order_.back() = reg_no;
        s_regs_table_[reg_no] = "";
    }

    std::pair<bool, TableEntry *> search_res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, symbol);
    std::pair<int, std::string> push_addr = get_memo_addr(symbol);
    if (search_res.second->data_type == DataType::INT_ARR &&
        (search_res.second->symbol_type == SymbolType::VAR || search_res.second->symbol_type == SymbolType::CONST)) {
        // let $s equals the array address
        add_code("add", "$s" + std::to_string(reg_no),  push_addr.second, push_addr.first);
    } else {
        add_code("lw", "$s" + std::to_string(reg_no), push_addr.first, push_addr.second);
    }
    s_regs_table_[reg_no] = symbol;
    return "$s" + std::to_string(reg_no);
}

void MipsGenerator::add_error(const std::string &error_msg) {
    std::cout << "!error: " + error_msg << std::endl;
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

// @brief: "sw", "reg", "off($gp)"
void MipsGenerator::add_code(const std::string &op, const std::string &dst, const std::string &src1) {
    std::string code;
    if (op == "sw" || op == "lw" || op == "div" || op == "mul" || op == "move" || op == "la") {
        code = op + " " + dst + ", " + src1;
    } else {
        std::cout << "add_code doesn't support this instr op" << std::endl;
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

void MipsGenerator::translate() {
    // use the global table in symbol table to assign memory in .data or in $gp
    bool init_main = false;
    add_code(".data");
    for (int i = 0; i < symbol_table_.strcons_.size(); i++) {
        std::string code = tab;
        code += ("str_" + std::to_string(i) + ":" + tab + ".asciiz ");
        code += ("\"" + symbol_table_.strcons_[i] + "\"");
        add_code(code);
    }
    add_code(".text");
//    add_code("sub", "$gp", "$gp", symbol_table_.get_global_data_size());
    for (auto &i_code: interm_codes_) {
        // todo: sub gp at the begin, the all the content can use add to access
        add_code("");
        add_code("# " + interm_code_to_string(i_code, false));
        IntermOp op = i_code.op;
        std::string dst = i_code.dst, src1 = i_code.src1, src2 = i_code.src2;
        if (op == IntermOp::PREPARE_CALL) {
            // sp minus (context and func size), it's denoted "frame size"
            // then save the context: ra, sp, s_res, t_res
            callee_name_ = dst;
            int func_stack_size = symbol_table_.get_func_stack_size(dst);
            int frame_size = context_size + func_stack_size;
            frame_size_stack_.push_back(frame_size);
            add_code("sub $sp, $sp, " + std::to_string(frame_size));
        }
            // PUSH_ARR
        else if (op == IntermOp::PUSH_ARR) {
            // e.g. PUSH_ARR @Tmp_12 4
            // MIPS:
            //     add $a0 $sp|$gp off
            //     sw $a0 param_off($sp)
            TableEntry *param = symbol_table_.GetKthParam(callee_name_, param_no_);

            std::pair<int, std::string> arr_addr = get_memo_addr(dst);
            if (arr_addr.second == "$gp") {
                add_code("add", "$a0", "$gp", arr_addr.first);
            } else {
                add_code("add", "$a0", "$sp", arr_addr.first);
            }
            int param_off = param->addr;
            add_code("sw", "$a0", param_off, "$sp");
        }
            // PUSH_VAL
        else if (op == IntermOp::PUSH_VAL) {
            // number or symbol? reg or memory? global or local?
            TableEntry *param_entry = symbol_table_.GetKthParam(callee_name_, std::stoi(src1));
            int param_off = param_entry->addr;
            if (is_integer(dst)) {
                // PUSH_VAL 1 1
                //      add $a0 1 $0
                //      sw $a0 param_off($sp)
                add_code("add", "$a0","$zero", dst);
                add_code("sw", "$a0", param_off, "$sp");
            } else {
                std::pair<bool, std::string> reg_search_res = search_in_st_regs(dst);
                if (reg_search_res.first) {
                    // sw reg_name param_off($sp)
                    add_code("sw", reg_search_res.second, param_off, "$sp");
                    release_reg_without_write_back(reg_search_res.second);
                } else { // in memory
                    std::pair<bool, TableEntry *> table_search_res =
                            symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, dst);
                    if (!table_search_res.first) add_error("param can't be found in symbol table");
                    std::pair<int, std::string> memo_addr = get_memo_addr(dst);
                    add_code("lw", "$a0", memo_addr.first, memo_addr.second);
                    add_code("sw", "$a0", 4 * std::stoi(src1), "$sp");

                }
            }
        }
            // CALL FOO
        else if (op == IntermOp::CALL) {
            // we don't save sp into the context, we use the frame_size to remember how much to return
            int func_stack_size = symbol_table_.get_func_stack_size(dst);
            add_code("sw $ra, " + std::to_string(ra_off + func_stack_size) + "($sp)");
            std::vector<int> saved_s_reg_no = {};
            std::vector<int> saved_t_reg_no = {};
            for (int i = 0; i < 8; i++) {
                if (!s_regs_table_[i].empty()) {
                    saved_s_reg_no.push_back(i);
                    add_code("sw", "$s" + std::to_string(i), func_stack_size + s_regs_off + 4 * i, "$sp");
                }
            }
            for (int i = 0; i < 10; i++) {
                if (!t_regs_table_[i].empty()) {
                    saved_t_reg_no.push_back(i);
                    add_code("sw", "$t" + std::to_string(i), func_stack_size + t_regs_off + 4 * i, "$sp");
                }
            }
            add_code("jal " + dst);
            add_code("lw", "$ra", ra_off + func_stack_size, "$sp");
            for (int i: saved_s_reg_no) {
                add_code("lw", "$s" + std::to_string(i), func_stack_size + s_regs_off + 4 * i, "$sp");
            }
            for (int i: saved_t_reg_no) {
                add_code("lw", "$t" + std::to_string(i), func_stack_size + t_regs_off + 4 * i, "$sp");
            }
            add_code("add", "$sp", "$sp", *(frame_size_stack_.end() - 1));
            frame_size_stack_.pop_back();
        }
            // FUNC_BEGIN
            // clear s_regs
            // store params into s_regs
            // do not care t_regs
        else if (op == IntermOp::FUNC_BEGIN) {
            if (!init_main) { // save a stack for main function
                add_code("addi $sp, $sp, -" + std::to_string(symbol_table_.get_func_stack_size("main")));
                add_code("j main");
                init_main = true;
            }
            cur_func_name_ = dst;
            add_code(dst + " :");
            if (cur_func_name_ != "main") {
                // the non-main function's view of the s-t regs are empty,
                // so save the current regs table and order until the end
                saved_s_fifo_order = s_fifo_order_;
                saved_s_regs_table_ = s_regs_table_;
                saved_t_fifo_order = t_fifo_order_;
                saved_t_regs_table_ = t_regs_table_;
                for (int i = 0; i < s_regs_table_.size(); i++) {
                    s_regs_table_[i] = "";
                    s_fifo_order_[i] = i;
                    // todo: try to store params into table
                }
            }
        }
            // RET
        else if (op == IntermOp::RET) {
            if (!dst.empty()) {
                load_to_reg(dst, "$v0");
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
            s_regs_table_ = saved_s_regs_table_;
            s_fifo_order_ = saved_s_fifo_order;
            t_fifo_order_ = saved_s_fifo_order;
            t_regs_table_ = saved_t_regs_table_;
        }
            // add sub mul div
        else if (is_arith(op)) {
            // add dst src1 src2
            // assign reg for dst
            std::string dst_reg = get_a_reg_for(dst);
            std::string src1_reg, src2_reg;
            if (op == IntermOp::ADD) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) + std::stoi(src2);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    src2_reg = get_a_reg_for(src2);
                    add_code("add", dst_reg, src2_reg, std::stoi(src1));
                    if (src2 != dst && src2[0] == '#')
                        release_reg_without_write_back(src2_reg);
                } else if (is_integer(src2)) {
                    src1_reg = get_a_reg_for(src1);
                    add_code("add", dst_reg, src1_reg, std::stoi(src2));
                    if (src1 != dst && src1[0] == '#')
                        release_reg_without_write_back(src1_reg);
                } else {
                    src1_reg = get_a_reg_for(src1);
                    src2_reg = get_a_reg_for(src2);
                    add_code("add", dst_reg, src1_reg, src2_reg);
                    if (src1 != dst && src1[0] == '#')
                        release_reg_without_write_back(src1_reg);
                    if (src2 != dst && src2[0] == '#')
                        release_reg_without_write_back(src2_reg);
                }
            }
                // SUB
            else if (op == IntermOp::SUB) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) - std::stoi(src2);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    src2_reg = get_a_reg_for(src2);
                    // add $a0, $zero, src1; sub dst $a0 src2
                    add_code("add", "$a1", "$zero", src1);
                    add_code("sub", dst_reg, "$a0", src2_reg);
                } else if (is_integer(src2)) {
                    src1_reg = get_a_reg_for(src1);
                    add_code("sub", dst_reg, src1_reg, src2);
                } else {
                    src1_reg = get_a_reg_for(src1);
                    src2_reg = get_a_reg_for(src2);
                    add_code("sub", dst_reg, src1_reg, src2_reg);
                }
                if (src1 != dst && src1[0] == '#') release_reg_without_write_back(src1_reg);
                if (src2 != dst && src2[0] == '#') release_reg_without_write_back(src2_reg);
            }
                // MUL
            else if (op == IntermOp::MUL) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) * std::stoi(src2);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    src2_reg = get_a_reg_for(src2);
                    add_code("mul", dst_reg, src2_reg, src1);
                } else if (is_integer(src2)) {
                    src1_reg = get_a_reg_for(src1);
                    add_code("mul", dst_reg, src1_reg, src2);
                } else {
                    src1_reg = get_a_reg_for(src1);
                    src2_reg = get_a_reg_for(src2);
                    add_code("mul", dst_reg, src1_reg, src2_reg);
                }
                if (src1 != dst && src1[0] == '#') release_reg_without_write_back(src1_reg);
                if (src2 != dst && src2[0] == '#') release_reg_without_write_back(src2_reg);
            }
                // DIV
            else if (op == IntermOp::DIV) {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) / std::stoi(src2);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    // dst = num / symbol
                    // add $a1 $zero src1; div dst_reg, $a1, src2_reg
                    src2_reg = get_a_reg_for(src2);
                    add_code("add", "$a1", src1, "$zero");
                    add_code("div", dst_reg, "$a1", src2_reg);
                } else if (is_integer(src2)) {
                    src1_reg = get_a_reg_for(src1);
                    add_code("div", dst_reg, src1_reg, src2);
                } else {
                    src1_reg = get_a_reg_for(src1);
                    src2_reg = get_a_reg_for(src2);
                    add_code("div", dst_reg, src1_reg, src2_reg);
                }
                if (src1 != dst && src1[0] == '#') release_reg_without_write_back(src1_reg);
                if (src2 != dst && src2[0] == '#') release_reg_without_write_back(src2_reg);
            }
                // MOD
            else {
                if (is_integer(src1) && is_integer(src2)) {
                    int res = std::stoi(src1) % std::stoi(src2);
                    add_code("add", dst_reg, "$zero", res);
                } else if (is_integer(src1)) {
                    add_code("add", "$a1", "$zero", src1);
                    src2_reg = get_a_reg_for(src2);
                    add_code("div", "$a1", src2_reg);
                    add_code("mfhi " + dst_reg);
                } else if (is_integer(src2)) {
                    add_code("add", "$a1", "$zero", src2);
                    src1_reg = get_a_reg_for(src1);
                    add_code("div", src1_reg, "$a1");
                    add_code("mfhi " + dst_reg);
                } else {
                    src1_reg = get_a_reg_for(src1);
                    src2_reg = get_a_reg_for(src2);
                    add_code("div", src1_reg, src2_reg);
                    add_code("mfhi " + dst_reg);
                }
                if (src1 != dst && src1[0] == '#') release_reg_without_write_back(src1_reg);
                if (src2 != dst && src2[0] == '#') release_reg_without_write_back(src2_reg);
            }
        }
            // ARR_LOAD, fetch a value from array
            // ARR_LOAD var_1 arr_name idx
        else if (op == IntermOp::ARR_LOAD) {
            std::string dst_reg = get_a_reg_for(dst);
            std::string arr_addr_reg = get_a_reg_for(src1);
            if (is_integer(src2)) {
                // ARR_LOAD #tmp arr_name 5
                int element_off = 4 * std::stoi(src2);
                add_code("lw", dst_reg, element_off, arr_addr_reg);
            } else {
                // ARR_LOAD #tmp arr_name #tmp15
                std::string idx_reg = get_a_reg_for(src2);
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
            std::string arr_addr_reg = get_a_reg_for(dst);
            // ARR_SAVE arr_1 1 10
            if (is_integer(src1) && is_integer(src2)) {
                add_code("add", val_reg, "$zero", src2);
                int element_off = 4 * std::stoi(src1);
                add_code("sw", val_reg, element_off, arr_addr_reg );
            } else if (is_integer(src1)) {
                // index/src1 is integer, val/src2 is a symbol
                src2_reg = get_a_reg_for(src2);
                int element_off = 4 * std::stoi(src1);
                add_code("sw", src2_reg, element_off,  arr_addr_reg);
            } else if (is_integer(src2)) {
                // index/src1 is symbol, value is integer
                src1_reg = get_a_reg_for(src1);
                add_code("add", off_reg, "$zero", src1_reg); // $a0 = src1
                add_code("sll", off_reg, off_reg, 2); // $a0 *= 4
                add_code("add", off_reg, off_reg, arr_addr_reg); // $a0 += arr_addr_reg
                add_code("add", val_reg, "$zero", src2);
                add_code("sw", val_reg, 0, off_reg );
            } else {
                src1_reg = get_a_reg_for(src1); // index
                src2_reg = get_a_reg_for(src2); // value
                add_code("add", off_reg, "$zero", src1_reg); // $a0 = src1
                add_code("sll", off_reg, off_reg, 2); // $a0 *= 4
                add_code("add", off_reg, off_reg, arr_addr_reg); // $a0 += arr_addr
                add_code("sw", src2_reg, 0,  off_reg);
            }
            // todo: check if src2 can be release
            // if (src2[0] == '#') release_reg_without_write_back(src2_reg);
        }
            // GETINT
        else if (op == IntermOp::GETINT) {
            add_code("li $v0, 5");
            add_code("syscall");
            std::pair<bool, std::string> search_res = search_in_st_regs(dst);
            if (search_res.first) {
                add_code("move", search_res.second, "$v0");
            } else {
                std::pair<int, std::string> dst_addr = get_memo_addr(dst);
                add_code("sw", "$v0", dst_addr.first, dst_addr.second);
            }
        }
            // PRINT
        else if (op == IntermOp::PRINT) {
            if (src1 == "str") {
                int idx = symbol_table_.find_str_idx(dst);
                add_code("la", "$a0", "str_" + std::to_string(idx));
                add_code("li $v0, 4");
            } else {
                load_to_reg(dst, "$a0");
                add_code("li $v0, 1");
            }
            add_code("syscall");
        }

    }


}

// @brief: search a symbol in s regs and t regs,
//         if in regs, return <true, reg_name>
//         else return <false, "">
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


// @brief: release a reg without writing back
// @param: a string like "$s1", "$t2", ...
// @note: put the reg number to fifo end
void MipsGenerator::release_reg_without_write_back(std::string reg_name) {
    if (reg_name[1] == 's') {
        int reg_no = reg_name[2] - '0';
        auto it = std::find(s_fifo_order_.begin(), s_fifo_order_.end(), reg_no);
        if (it != s_fifo_order_.end()) s_fifo_order_.erase(it);
        s_fifo_order_.push_back(reg_no);
        s_regs_table_[reg_no] = "";
    } else if (reg_name[1] == 't') {
        int reg_no = reg_name[2] - '0';
        auto it = std::find(t_fifo_order_.begin(), t_fifo_order_.end(), reg_no);
        if (it != t_fifo_order_.end()) t_fifo_order_.erase(it);
        t_fifo_order_.push_back(reg_no);
        t_regs_table_[reg_no] = "";
    } else {
        add_error("release failed: input reg name can't be recognize");
    }
}
