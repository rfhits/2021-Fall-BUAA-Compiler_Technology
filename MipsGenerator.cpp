//
// Created by WYSJ6174 on 2021/11/8.
//

#include "MipsGenerator.h"

#define MIPS_DBG true

MipsGenerator::MipsGenerator(SymbolTable &symbol_table, std::vector<IntermCode> &interm_codes,
                             std::vector<std::string> &strcons, std::ofstream &out) :
        symbol_table_(symbol_table), interm_codes_(interm_codes), strcons_(strcons), out_(out) {
    for (int i = 0; i < t_regs_table_.size(); i++) {
        t_fifo_order_[i] = i;
    }
}


// t reg can
std::string MipsGenerator::assign_t_reg(std::string symbol) {
    for (int i = 0; i < t_regs_table_.size(); i++) {
        if (t_regs_table_[i].empty()) {
            t_regs_table_[i] = symbol;
            return "$t" + std::to_string(i);
        }
    }
    add_error("can't assign t reg for symbol: " + symbol);
    return "";
}

//std::string MipsGenerator::assign_s_reg(std::string symbol) {
//    // check if there is empty
//    for (int i = 0; i < s_regs_table_.size(); i++) {
//        if (s_regs_table_[i].empty()) {
//            return "$s" + std::to_string(i);
//        }
//    }
//
//    // min use reg, save its value to its addr
//    // a function's view is itself and the global
//    // so search in these two scope
//    // todo: add a dirty bit
//    int min_times = s_use_times_[0];
//    int index = 0;
//    std::string out_symbol_name;
//    std::string reg_name;
//    for (int i = 1; i < s_use_times_.size(); i++) {
//        if (s_use_times_[i] < min_times) {
//            min_times = s_use_times_[i];
//            index = i;
//            out_symbol_name = s_regs_table_[i];
//        }
//    }
//    reg_name = "$s" + std::to_string(index);
//    std::pair<bool, TableEntry *> res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, out_symbol_name);
//    if (!res.first) add_error("out s reg can't find its address");
//    if (res.second->level == 0) {
//        add_code("sw", reg_name, res.second->addr, "($gp)");
//    } else {
//        add_code("sw", reg_name, res.second->addr, "($sp)");
//    }
//    return reg_name;
//}

void MipsGenerator::add_error(const std::string &error_msg) {
    std::cout << error_msg << std::endl;
}


// @brief: a nick function for add
void MipsGenerator::add_code(const std::string &op, const std::string &dst, const std::string &src1, int src2) {
    add_code(op, dst, src1, std::to_string(src2));
}


void MipsGenerator::translate() {
    // use the global table in symbol table to assign memory in .data or in $gp
    bool init_main = false;
    add_code(".data");
//    for (auto &i: symbol_table_.global_table_) {
//        if (i.symbol_type == SymbolType::CONST || i.symbol_type == SymbolType::VAR) {
//            std::string code = (i.alias + ":" + tab + tab);
//            if (i.data_type == DataType::INT) {
//                code += (".word 0");
//            } else if (i.data_type == DataType::INT_ARR) {
//                code += (".space " + std::to_string(i.size));
//            } else {
//                add_error(".data error");
//            }
//            add_code(code);
//        }
//    }
    for (auto &i: symbol_table_.strcons_) {
        // todo: assign ascii

    }
    for (auto &i_code: interm_codes_) {
        // todo: sub gp at the begin, the all the content can use add to access
        add_code("");
        add_code("# " + interm_code_to_string(i_code));
        IntermOp op = i_code.op;
        std::string dst = i_code.dst, src1 = i_code.src1, src2 = i_code.src2;
        if (op == IntermOp::PREPARE_CALL) {
            // sp minus (context and func size), it's denoted "frame size"
            // then save the context: ra, sp, s_res, t_res
            callee_name_ = dst;
            int func_stack_size = symbol_table_.get_func_stack_size(dst);
            int frame_size = context_size + func_stack_size;
            frame_size_stack.push_back(frame_size);
            add_code("sub $sp, $sp, -" + std::to_string(frame_size));
        }
            // PUSH_ARR
        else if (op == IntermOp::PUSH_ARR) {
            // e.g. PUSH_ARR #Tmp_12
            // MIPS:
            //     add $a0 $sp|$gp off
            //     sw $a0 param_off($sp)
            TableEntry *param = symbol_table_.GetKthParam(callee_name_, param_no_);
            std::pair<bool, TableEntry *> search_res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, dst);
            if (!search_res.first) add_error("can't find the arr param");
            if (search_res.second->level == 0) { // a global arr
                int off_global = search_res.second->addr;
                add_code("add", "$a0", "$gp", std::to_string(off_global));
            } else {
                int off_sp = frame_size_stack.back() + search_res.second->addr;
                add_code("add", "$a0", std::to_string(off_sp), "($sp)");
            }
            int param_off = param->addr;
            add_code("sw", "$t0", param_off, "($sp)");
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
                add_code("add", "$a0", dst, "$v0");
                add_code("sw", "$a0", param_off, "($sp)");
            } else {
                std::pair<bool, std::string> reg_search_res = search_in_st_regs(dst);
                if (reg_search_res.first) {
                    // sw reg_name param_off($sp)
                    add_code("sw", reg_search_res.second, param_off, "($sp)");
                    release_reg(reg_search_res.second);
                } else { // in memory
                    std::pair<bool, TableEntry *> table_search_res =
                            symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, dst);
                    if (!table_search_res.first) add_error("param can't be found in symbol table");
                    std::pair<int, std::string> memo_addr = get_memo_addr(dst);
                    if (table_search_res.second->level == 0) {
                        // lw $a0 off($gp)
                        // sw $a0 param_off($sp)
                        add_code("lw", "$a0", memo_addr.first, memo_addr.second);
                        add_code("sw", "$a0", param_off, "($sp)");
                    } else {
                        // lw $a0, off+stack($sp)
                        add_code("lw", "$a0", frame_size_stack.back() + memo_addr.first, memo_addr.second);
                        add_code("sw", "$a0", param_off, "($sp)");
                    }
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
                    add_code("sw", "$s" + std::to_string(i), func_stack_size + s_regs_off + 4 * i, "($sp)");
                }
            }
            for (int i = 0; i < 10; i++) {
                if (!t_regs_table_[i].empty()) {
                    saved_t_reg_no.push_back(i);
                    add_code("sw", "$t" + std::to_string(i), func_stack_size + t_regs_off + 4 * i, "($sp)");
                }
            }
            add_code("jal " + dst);
            add_code("lw", "$ra", ra_off + func_stack_size, "($sp)");
            for (int i: saved_s_reg_no) {
                add_code("lw", "$s" + std::to_string(i), func_stack_size + s_regs_off + 4 * i, "($sp)");
            }
            for (int i: saved_t_reg_no) {
                add_code("lw", "$t" + std::to_string(i), func_stack_size + t_regs_off + 4 * i, "($sp)");
            }
            add_code("add", "$sp", "$sp", frame_size_stack.back());
            frame_size_stack.pop_back();
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
            for (int i = 0; i < s_regs_table_.size(); i++) {
                s_regs_table_[i] = "";
                s_fifo_order_[i] = i;
                // todo: try to store params into table
            }
        }
            // FUNC_END
        else if (op == IntermOp::FUNC_END) {
            // do not need to do anything
        }
            // add sub mul div
        else if (is_arith(op)) {

        }
    }


}

void MipsGenerator::add_code(const std::string &code) {
    if (MIPS_DBG) {
        out_ << code << std::endl;
    } else {
        mips_codes_.push_back(code);
    }
}

// @brief: given four strings simply ,then generate the most simple instr
// @pre: add, sub,
void MipsGenerator::add_code(const std::string &op, const std::string &dst,
                             const std::string &src1, const std::string &src2) {
    std::string code = op + " " + dst + ", " + src1 + ", " + src2;
    if (!MIPS_DBG) {
        mips_codes_.push_back(code);
    } else {
        out_ << code << std::endl;
    }
}

// @brief: sw reg off($gp)
void MipsGenerator::add_code(const std::string &op, const std::string &dst, const std::string &src1) {
    if (op == "sw" || op == "lw") {
        std::string code = op + " " + dst + ", " + src1;
        if (!MIPS_DBG) {
            mips_codes_.push_back(code);
        } else {
            out_ << code << std::endl;
        }
    } else {
        std::cout << "add_code doesn't support this instr op" << std::endl;
    }
}

// @brief: this function is called when lw or sw
//         the base_addr should be '($sp)' or ($gp)
void MipsGenerator::add_code(const std::string op, const std::string &reg_name, int off, std::string base_addr) {
    if (op != "sw" || op != "lw") {
        add_error("expect lw or sw in memory access code");
    }

    std::string code = op;
    code += " ";
    code += reg_name;
    code += ", ";
    code += std::to_string(off);
    code += base_addr;
    if (!MIPS_DBG) {
        mips_codes_.push_back(code);
    } else {
        out_ << code << std::endl;
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


// @brief: given a symbol, return its offset to its base pointer
//         if it's a global symbol, pointer is ($gp), else pointer is ($sp)
// @note: in PUSH_VAL, the offset need to change, because the sp has been subtracted
std::pair<int, std::string> MipsGenerator::get_memo_addr(std::string symbol) {
    std::pair<bool, TableEntry *> table_search_res =
            symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, symbol);
    if (!table_search_res.first) add_error("symbol can't be found in symbol table");
    if (table_search_res.second->level == 0) {
        return std::make_pair(table_search_res.second->addr, "($gp)");
    } else {
        return std::make_pair(table_search_res.second->addr, "($sp)");
    }
}


// @brief: release a reg, no matter what its content is
// @param: "$s1", "$t2", ...
void MipsGenerator::release_reg(std::string reg_name) {
    if (reg_name[1] == 's') {
        int reg_no = reg_name[2] - '0';
        auto it = std::find(s_fifo_order_.begin(), s_fifo_order_.end(), reg_no);
        if (it != s_fifo_order_.end()) s_fifo_order_.erase(it);
        s_fifo_order_.push_back(reg_no);
        s_regs_table_[reg_no] = "";
    } else if (reg_name[1] == 't') {
        int reg_no = reg_name[2] = '0';
        auto it = std::find(t_fifo_order_.begin(), t_fifo_order_.end(), reg_no);
        if (it != t_fifo_order_.end()) t_fifo_order_.erase(it);
        t_fifo_order_.push_back(reg_no);
        t_regs_table_[reg_no] = "";
    } else {
        add_error("release a reg can't be recognize");
    }
}
