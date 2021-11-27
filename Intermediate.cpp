//
// Created by WYSJ6174 on 2021/10/17.
//

#include "Intermediate.h"

#define INTERM_DUG true

bool is_arith(IntermOp op) {
    if (op == IntermOp::ADD || op == IntermOp::SUB || op == IntermOp::MUL || op == IntermOp::DIV ||
        op == IntermOp::MOD) {
        return true;
    } else {
        return false;
    }
};

bool is_bitwise(IntermOp op) {
    if (op == IntermOp::AND || op == IntermOp::OR || op == IntermOp::NOT) {
        return true;
    } else {
        return false;
    }
}

bool is_cmp(IntermOp op) {
    if (op == IntermOp::EQ || op == IntermOp::NEQ || op == IntermOp::LSS || op == IntermOp::LEQ ||
        op == IntermOp::GRE || op == IntermOp::GEQ) {
        return true;
    } else {
        return false;
    }
}

Intermediate::Intermediate(SymbolTable &symbol_table, std::ofstream &out) : symbol_table_(symbol_table), out_(out) {
}

// generate temp variable into symbol_table
std::string Intermediate::GenTmpVar(const std::string &func_name, DataType data_type, int level, unsigned int addr) {
    std::string name = "#Tmp" + std::to_string(tmp_cnt_++);
    symbol_table_.AddSymbol(func_name, data_type, SymbolType::VAR, name, name,
                            0, level, 0, 0, 0, addr);
    return name;
}

// generate tmp array variable into symbol table
// when calling a function which using array as parameter
std::string Intermediate::GenTmpArr(const std::string &func_name, DataType data_type,
                                    int level, int dims, int dim0_size, int dim1_size, unsigned int addr) {

    std::string name = "@Arr" + std::to_string(param_arr_cnt_++);
    symbol_table_.AddSymbol(func_name, data_type, SymbolType::VAR, name, name,
                            0, level, dims, dim0_size, dim1_size, addr);
    return name;
}

void Intermediate::AddMidCode(const std::string &dst, IntermOp op, const std::string &src1, const std::string &src2) {
    IntermCode interm_code;
    interm_code.dst = dst;
    interm_code.op = op;
    interm_code.src1 = src1;
    interm_code.src2 = src2;
    if (INTERM_DUG) {
        out_ << interm_code_to_string(interm_code, true) << std::endl;
        interm_codes_.push_back(interm_code);
    } else {
        interm_codes_.push_back(interm_code);
    }

}

void Intermediate::AddMidCode(const std::string &dst, IntermOp op, int src1, const std::string &src2) {
    std::string str_src1 = std::to_string(src1);
    AddMidCode(dst, op, str_src1, src2);
}

void Intermediate::AddMidCode(const std::string &dst, IntermOp op, const std::string &src1, int src2) {
    std::string str_src2 = std::to_string(src2);
    AddMidCode(dst, op, src1, str_src2);
}

void Intermediate::AddMidCode(const std::string &dst, IntermOp op, int src1, int src2) {
    std::string str_src1 = std::to_string(src1);
    std::string str_src2 = std::to_string(src2);
    AddMidCode(dst, op, str_src1, str_src2);
}

std::string Intermediate::GenLabel() {
    std::string label = "Label_" + std::to_string(label_cnt_++);
    return label;
}

std::string Intermediate::GenWhileBeginLabel() {
    std::string label = "While_Begin_Label_" + std::to_string(while_label_cnt_);
    return label;
}

std::string Intermediate::GenWhileEndLabel() {
    std::string label = "While_End_Label_" + std::to_string(while_label_cnt_++);
    return label;
}

std::string Intermediate::GenCondEndLabel() {
    std::string label = "Cond_End_Label_" + std::to_string(cond_label_cnt_++);
    return label;
}

std::string Intermediate::GenLAndEndLabel() {
    std::string label = "LAnd_End_Label_" + std::to_string(land_label_cnt++);
    return label;
}


void Intermediate::codes_to_string() {
    for (auto &interm_code: interm_codes_) {
        out_ << interm_code_to_string(interm_code, true) << std::endl;
    }
}

void Intermediate::InlineFunc() {
    std::vector<IntermCode> new_interm_codes;
    std::map<std::string, std::vector<IntermCode>> func_interm_codes;
    std::string cur_func;
    // fill each function codes
    for (int i = 0; i < interm_codes_.size(); i++) {
        if (interm_codes_[i].op == IntermOp::FUNC_BEGIN) {
            cur_func = interm_codes_[i].dst;
            func_interm_codes[cur_func] = std::vector<IntermCode>();
        } else if (interm_codes_[i].op == IntermOp::FUNC_END) {
            cur_func = "";
        } else if (!cur_func.empty()) {
            func_interm_codes[cur_func].push_back(interm_codes_[i]);
        } else {
            // pass
        }
    }

    // inline begin
    std::string caller_name = "";
    std::string inline_callee_name;
    std::vector<std::string> inline_callee_stack;
    for (auto &code: interm_codes_) {
        if (code.op == IntermOp::FUNC_BEGIN) {
            caller_name = code.dst;
        }
            // prepare call, check if is inline-able
        else if (code.op == IntermOp::PREPARE_CALL) {
            std::pair<bool, TableEntry *> search_res = symbol_table_.SearchFunc(code.dst);
            if (!search_res.first) handle_error("inline function not defined");
            std::string callee_name = search_res.second->name;
            if (!search_res.second->is_recur_func && func_interm_codes[callee_name].size() <= 30) {
                inline_callee_name = code.dst;
                inline_callee_stack.push_back(code.dst);
                inline_times += 1;
            } else {
                // pass
            }
        }
            // inline PUSH_VAL
        else if (code.op == IntermOp::PUSH_VAL && !inline_callee_stack.empty()) {
            int param_order = std::stoi(code.src1);
            TableEntry *param = symbol_table_.GetKthParam(inline_callee_name, param_order);
            std::string new_param_name = param->name + "_" + std::to_string(inline_times);
            int caller_stack_size = symbol_table_.GetFuncStackSize(caller_name);
            int addr = caller_stack_size + 4;
            symbol_table_.AddSymbol(caller_name, param->data_type, SymbolType::VAR, new_param_name, new_param_name,
                                    param->value, param->level, param->dims, param->dim0_size, param->dim1_size, addr);
            new_interm_codes.emplace_back(IntermOp::ADD, new_param_name, code.dst, "0");
        }
            // inline PUSH_ARR
            // todo: is push_arr and push_val different?
        else if (code.op == IntermOp::PUSH_ARR && !inline_callee_stack.empty()) {
            int param_order = std::stoi(code.src1);
            TableEntry *param = symbol_table_.GetKthParam(inline_callee_name, param_order);
            std::string new_param_name = param->name + "_" + std::to_string(inline_times);
            int caller_stack_size = symbol_table_.GetFuncStackSize(caller_name);
            int addr = caller_stack_size + 4;
            symbol_table_.AddSymbol(caller_name, param->data_type, SymbolType::VAR, new_param_name, new_param_name,
                                    param->value, param->level, param->dims, param->dim0_size, param->dim1_size, addr);
            new_interm_codes.emplace_back(IntermOp::ADD, new_param_name, code.dst, "0");
        }
            // inline call
        else if (code.op == IntermOp::CALL) {
            std::string label = inline_callee_name + "_INLINE_END_" + std::to_string(inline_times);
            for (auto &func_code: func_interm_codes[caller_name]) {
                std::string renamed_dst = "";
            }
        } else {

        }
    }


}

// @brief: rename the callee function symbols into caller function symbol table
//         if the symbol is param, just add the inline_times, we promise it has been inject into the caller symbol table
//         else we need to check if it has been written to the caller symbol table
// @attention: u need to check if it is renamed into the caller's symbol table
std::string
Intermediate::rename_inline_symbol(std::string caller_name, std::string callee_name, std::string symbol_name) {

    // label, integer, global, ret
    if (symbol_name.find("Label") != std::string::npos) return symbol_name + "_" + std::to_string(inline_times);
    if (is_integer(symbol_name) || symbol_table_.is_global_symbol(symbol_name)) return symbol_name;
    if (symbol_name == "%RET") return symbol_name;

    std::pair<bool, TableEntry *> callee_search_res = symbol_table_.SearchNearestSymbolNotFunc(callee_name,
                                                                                               symbol_name);
    if (!callee_search_res.first) handle_error("can't find inline symbol in the callee symbol table while renaming");
    TableEntry symbol_in_callee = TableEntry(callee_search_res.second);
    // search in the caller's symbol table,
    // if not in it, append
    std::string re_name = symbol_name + "_" + std::to_string(inline_times);
    std::pair<bool, TableEntry *> caller_search_res = symbol_table_.SearchNearestSymbolNotFunc(caller_name, re_name);
    // not in caller, append to end
    if (!caller_search_res.first) {
        int addr = symbol_table_.GetFuncStackSize(caller_name);
        addr += callee_search_res.second->size;
        symbol_table_.AddSymbol(caller_name, symbol_in_callee.data_type, SymbolType::VAR, re_name, re_name,
                                symbol_in_callee.value, symbol_in_callee.level,
                                symbol_in_callee.dims, symbol_in_callee.dim0_size, symbol_in_callee.dim1_size,
                                addr);
    } else {
        // pass
    }
    return re_name;
}

//
void Intermediate::handle_error(std::string msg) {
    std::cout << msg << std::endl;
}


std::string get_op_string(IntermOp op) {
    std::string str_op;
    auto it = op_to_str.find(op);
    if (it != op_to_str.end()) {
        str_op = it->second;
    } else {
        str_op = "undefined_operator";
    }
    return str_op;
}


std::string interm_code_to_string(const IntermCode &code, bool tab) {
    std::string output;
    std::string indent = (tab) ? "    " : "";
    if (code.op == IntermOp::LABEL || code.op == IntermOp::FUNC_BEGIN || code.op == IntermOp::FUNC_END)
        indent = "";

    output += indent;
    if (code.op == IntermOp::LABEL) {
        output += code.dst;
        output += ":";
    } else {
        output += get_op_string(code.op);
        output += " ";
        output += code.dst;
        output += " ";
        output += code.src1;
        output += " ";
        output += code.src2;
    }
    return output;
}

