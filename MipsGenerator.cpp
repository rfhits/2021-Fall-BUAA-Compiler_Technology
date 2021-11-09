//
// Created by WYSJ6174 on 2021/11/8.
//

#include "MipsGenerator.h"

#define MIPS_DBG true

MipsGenerator::MipsGenerator(SymbolTable &symbol_table, std::vector<IntermCode> &interm_codes,
                             std::vector<std::string> &strcons, std::ofstream &out) :
        symbol_table_(symbol_table), interm_codes_(interm_codes), strcons_(strcons), out_(out) {}


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

std::string MipsGenerator::assign_s_reg(std::string symbol) {
    // check if there is empty
    for (int i = 0; i < s_regs_table_.size(); i++) {
        if (s_regs_table_[i].empty()) {
            return "$s" + std::to_string(i);
        }
    }

    // min use reg, save its value to its addr
    // a function's view is itself and the global
    // so search in these two scope
    // todo: add a dirty bit
    int min_times = s_use_times_[0];
    int index = 0;
    std::string out_symbol_name;
    std::string reg_name;
    for (int i = 1; i < s_use_times_.size(); i++) {
        if (s_use_times_[i] < min_times) {
            min_times = s_use_times_[i];
            index = i;
            out_symbol_name = s_regs_table_[i];
        }
    }
    reg_name = "$s" + std::to_string(index);
    std::pair<bool, TableEntry *> res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, out_symbol_name);
    if (!res.first) add_error("out s reg can't find its address");
    if (res.second->level == 0) {
        add_code("sw", reg_name, res.second->addr, "($gp)");
    } else {
        add_code("sw", reg_name, res.second->addr, "($sp)");
    }
    return reg_name;
}

void MipsGenerator::add_error(const std::string &error_msg) {
    std::cout << error_msg << std::endl;
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
        add_code("");
        add_code("# " + interm_code_to_string(i_code));
        IntermOp op = i_code.op;
        std::string dst = i_code.dst, src1 = i_code.src1, src2 = i_code.src2;
        if (op == IntermOp::PREPARE_CALL) {
            // sp minus (context and func size), it's denoted "frame size"
            // then save the context: ra, sp, s_res, t_res

            callee_name_ = dst;
            int frame_size = context_size + symbol_table_.get_func_stack_size(dst);
            frame_size_stack.push_back(frame_size);
            add_code("sub $sp, $sp, -" + std::to_string(frame_size));
            add_code("sw $ra, " + std::to_string(ra_off) + "($sp)");
            // we don't save sp into the context, we use the frame_size to resume the thread
            for (int i = 0; i < 8; i++) {
                s_old_table_ = s_regs_table_;
                add_code("sw", "$s" + std::to_string(i), s_regs_off + 4 * i, "($sp)");
                s_regs_table_[i] = "";
            }
            for (int i = 0; i < 10; i++) {
                t_old_table_ = t_regs_table_;
                add_code("sw", "$t" + std::to_string(i), t_regs_off + 4 * i, "($sp)");
                t_regs_table_[i] = "";
            }
        } else if (op == IntermOp::PUSH_ARR) {
            // eg: PUSH_ARR Tmp_12
            TableEntry *param = symbol_table_.GetKthParam(callee_name_, param_no_);
            int off = param->addr + context_size;


        } else if (op == IntermOp::FUNC_BEGIN) {
            if (!init_main) { // save a stack for main function
                add_code("addi $sp, $sp, -" + std::to_string(symbol_table_.get_func_stack_size(dst)));
                add_code("j main");
                init_main = true;
            }
            cur_func_name_ = dst;
            add_code(dst + " :");
        } else if (op == IntermOp::FUNC_END) {

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

void MipsGenerator::add_code(const std::string &op, const std::string &dst,
                             const std::string &src1, const std::string &src2) {
    std::string code = op + " " + dst + " " + src1 + " " + src2;
    if (!MIPS_DBG) {
        mips_codes_.push_back(code);
    } else {
        out_ << code << std::endl;
    }
}

void MipsGenerator::add_code(const std::string &op, const std::string &dst, const std::string &src1) {

}

// @brief:
void MipsGenerator::add_code(const std::string op, const std::string &reg_name, int off, std::string addr) {
    if (op != "sw" || op != "lw") {
        add_error("expect lw or sw in memory access code");
    }

    std::string code = op;
    code += " ";
    code += reg_name;
    code += ", ";
    code += std::to_string(off);
    code += addr;
    if (!MIPS_DBG) {
        mips_codes_.push_back(code);
    } else {
        out_ << code << std::endl;
    }

}
