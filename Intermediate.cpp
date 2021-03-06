//
// Created by WYSJ6174 on 2021/10/17.
//

#include "Intermediate.h"


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

bool is_read_op(IntermOp op) {
    if (op == IntermOp::ARR_SAVE || op == IntermOp::PRINT) {
        return true;
    } else {
        return false;
    }
}

bool op_modify_dst(IntermOp op) {
    if (is_arith(op) || is_bitwise(op) || is_cmp(op) ||
        op == IntermOp::GETINT || op == IntermOp::ARR_LOAD || op == IntermOp::INIT_ARR_PTR) {
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
    codes_.push_back(interm_code);
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

std::string Intermediate::GenWhileHeadLabel() {
    std::string label = "While_Head_Label_" + std::to_string(while_label_cnt_);
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


void Intermediate::OutputCodes() {
    for (auto &interm_code: codes_) {
        out_ << interm_code_to_string(interm_code, true) << std::endl;
    }
}


void Intermediate::OutputCodes(std::ofstream &out) {
    for (auto &code: codes_) {
        out << interm_code_to_string(code, true) << std::endl;
    }
}


void Intermediate::OutputBasicBlocks(std::ofstream &out) {
    for (auto &basic_block: basic_blocks_) {
        basic_block.OutputBasicBlock(out);
    }
}

void Intermediate::OutputFuncBlocks(std::ofstream &out) {
    basic_blocks_[0].OutputBasicBlock(out);
    for (int i = 0; i < func_blocks_.size(); i++) {
        out << "---- Func " << func_blocks_[i].func_name_ << " Begin ----" << std::endl;
        out << "has print: " << func_blocks_[i].has_print_ << std::endl;
        out << "has getint: " << func_blocks_[i].has_getint_ << std::endl;
        out << "write memo: " << func_blocks_[i].write_memo_ << std::endl;
        out << "---- conflict graph ----" << std::endl;
        for (auto &it: func_blocks_[i].conflict_graph_.graph) {
            out << it.first + ": ";
            for (const std::string &symbol: it.second) {
                out << symbol + " ";
            }
            out << std::endl;
        }
        for (auto id: func_blocks_[i].block_ids_) {
            basic_blocks_[id].OutputBasicBlock(out);
        }
        out << "---- " << "Func End" << " ----" << std::endl << std::endl;
    }
}


void Intermediate::InlineFunc() {
    std::vector<IntermCode> new_codes;
    std::map<std::string, std::vector<IntermCode>> func_interm_codes;
    std::string cur_func;

    // fill each function's codes
    for (int i = 0; i < codes_.size(); i++) {
        if (codes_[i].op == IntermOp::FUNC_BEGIN) {
            cur_func = codes_[i].dst;
            func_interm_codes[cur_func] = std::vector<IntermCode>();
        } else if (codes_[i].op == IntermOp::FUNC_END) {
            cur_func = "";
        } else if (!cur_func.empty()) {
            func_interm_codes[cur_func].push_back(codes_[i]);
        } else {
            // pass
        }
    }

    // inline begin
    std::string caller_name;
    std::string inline_callee_name;
    std::vector<std::string> inline_callee_stack;
    for (auto &code: codes_) {
        if (code.op == IntermOp::FUNC_BEGIN) {
            caller_name = code.dst;
            new_codes.emplace_back(code);
        }
            // prepare call, check if is inline-able
        else if (code.op == IntermOp::PREPARE_CALL) {
            std::pair<bool, TableEntry *> search_res = symbol_table_.SearchFunc(code.dst);
            if (!search_res.first) handle_error("inline function not defined");
            std::string callee_name = search_res.second->name;
            if (!search_res.second->is_recur_func && func_interm_codes[caller_name].size() <= 30) {
                std::cout << "function " + callee_name + " is inlined to " + caller_name << std::endl;
                inline_callee_name = code.dst;
                inline_callee_stack.push_back(code.dst);
                inline_times_ += 1;
            } else {
                // pass
                new_codes.emplace_back(code);
            }
        }
            // inline PUSH_VAL
        else if (code.op == IntermOp::PUSH_VAL && !inline_callee_stack.empty()) {
            int param_order = std::stoi(code.src1);
            TableEntry *param = symbol_table_.GetKthParam(inline_callee_name, param_order);
            std::string new_param_name = param->name + "_" + std::to_string(inline_times_);
            int caller_stack_size = symbol_table_.GetFuncStackSize(caller_name);
            int addr = caller_stack_size;
            symbol_table_.AddSymbol(caller_name, param->data_type, SymbolType::VAR, new_param_name, new_param_name,
                                    param->value, param->level, param->dims, param->dim0_size, param->dim1_size, addr);
            new_codes.emplace_back(IntermOp::ADD, new_param_name, code.dst, "0");
        }
            // inline PUSH_ARR
            // todo: is push_arr and push_val different?
        else if (code.op == IntermOp::PUSH_ARR && !inline_callee_stack.empty()) {
            int param_order = std::stoi(code.src1);
            TableEntry *param = symbol_table_.GetKthParam(inline_callee_name, param_order);
            std::string new_param_name = param->name + "_" + std::to_string(inline_times_);
            int caller_stack_size = symbol_table_.GetFuncStackSize(caller_name);
            int addr = caller_stack_size;
            symbol_table_.AddSymbol(caller_name, param->data_type, SymbolType::VAR, new_param_name, new_param_name,
                                    param->value, param->level, param->dims, param->dim0_size, param->dim1_size, addr);
            new_codes.emplace_back(IntermOp::ADD, new_param_name, code.dst, "0");
        }
            // inline call
        else if (code.op == IntermOp::CALL && !inline_callee_stack.empty()) {
            std::string inline_func_end_label = inline_callee_name + "_INLINE_END_" + std::to_string(inline_times_);
            for (auto &func_code: func_interm_codes[inline_callee_name]) {
                std::string re_dst, re_src1, re_src2;
                IntermOp op = func_code.op;
                std::string dst = func_code.dst;
                std::string src1 = func_code.src1;
                std::string src2 = func_code.src2;
                if (op == IntermOp::PRINT) {
                    if (src1 == "int") {
                        re_dst = rename_inline_symbol(caller_name, inline_callee_name, dst);
                        new_codes.emplace_back(IntermOp::PRINT, re_dst, "int", "");
                    } else {
                        new_codes.emplace_back(func_code);
                    }
                }
                    // LABEL, direct rename here, because label do not in symbol table
                else if (op == IntermOp::LABEL || op == IntermOp::JUMP) {
                    re_dst = dst + "_" + std::to_string(inline_times_);
                    new_codes.emplace_back(op, re_dst, "", "");
                } else if (op == IntermOp::BEQ || op == IntermOp::BNE) {
                    re_dst = dst + "_" + std::to_string(inline_times_);
                    re_src1 = rename_inline_symbol(caller_name, inline_callee_name, src1);
                    re_src2 = rename_inline_symbol(caller_name, inline_callee_name, src2);
                    new_codes.emplace_back(op, re_dst, re_src1, re_src2);
                } else if (op == IntermOp::PREPARE_CALL || op == IntermOp::CALL) {
                    new_codes.emplace_back(func_code);
                }
                    // RET
                else if (op == IntermOp::RET) {
                    if (dst.empty()) { // return void;
                        new_codes.emplace_back(IntermOp::JUMP, inline_func_end_label, "", "");
                    } else {
                        re_dst = rename_inline_symbol(caller_name, inline_callee_name, dst);
                        new_codes.emplace_back(IntermOp::ADD, "%RET", re_dst, "0");
                        new_codes.emplace_back(IntermOp::JUMP, inline_func_end_label, "", "");
                    }
                }
                    // arith, rel, INIT, arr
                else {
                    re_dst = rename_inline_symbol(caller_name, inline_callee_name, dst);
                    re_src1 = rename_inline_symbol(caller_name, inline_callee_name, src1);
                    re_src2 = rename_inline_symbol(caller_name, inline_callee_name, src2);
                    new_codes.emplace_back(op, re_dst, re_src1, re_src2);
                }
            }
            new_codes.emplace_back(IntermOp::JUMP, inline_func_end_label, "", "");
            new_codes.emplace_back(IntermOp::LABEL, inline_func_end_label, "", "");
            inline_callee_stack.pop_back();
            if (inline_callee_stack.empty()) {
                inline_callee_name = "";
            } else {
                inline_callee_name = inline_callee_stack.back();
            }
        }
            //
        else {
            new_codes.emplace_back(code);
        }
    }
    codes_ = new_codes;
}

// @brief: rename the callee function symbols into caller function symbol table
//         if the symbol is param, just add the inline_times_, we promise it has been inject into the caller symbol table
//         else we need to check if it has been written to the caller symbol table
// @attention: u need to check if it is renamed into the caller's symbol table
std::string
Intermediate::rename_inline_symbol(const std::string &caller_name, const std::string &callee_name,
                                   std::string symbol_name) {
    if (symbol_name.empty()) return "";

    // label, integer, global, ret
    if (symbol_name.find("Label") != std::string::npos) return symbol_name + "_" + std::to_string(inline_times_);
    if (is_integer(symbol_name) || symbol_table_.is_global_symbol(symbol_name)) return symbol_name;
    if (symbol_name == "%RET") return symbol_name;

    std::pair<bool, TableEntry *> callee_search_res = symbol_table_.SearchNearestSymbolNotFunc(callee_name,
                                                                                               symbol_name);
    if (!callee_search_res.first)
        handle_error("can't find inline symbol '" + symbol_name + "' in the callee symbol table while renaming");
    TableEntry symbol_in_callee = TableEntry(callee_search_res.second);
    // search in the caller's symbol table,
    // if not in it, append
    std::string re_name = symbol_name + "_" + std::to_string(inline_times_);
    std::pair<bool, TableEntry *> caller_search_res = symbol_table_.SearchNearestSymbolNotFunc(caller_name, re_name);
    // not in caller, append to end
    if (!caller_search_res.first) {
        int addr = symbol_table_.GetFuncStackSize(caller_name);
//        addr += callee_search_res.second->size;
        symbol_table_.AddSymbol(caller_name, symbol_in_callee.data_type, SymbolType::VAR, re_name, re_name,
                                symbol_in_callee.value, symbol_in_callee.level,
                                symbol_in_callee.dims, symbol_in_callee.dim0_size, symbol_in_callee.dim1_size,
                                addr);
    } else {
        // pass
    }
    return re_name;
}

void Intermediate::handle_error(std::string msg) {
    std::cout << msg << std::endl;
}

// @brief: reset the basic_blocks and func_blocks,
//         loop optimize
void Intermediate::reset_blocks() {
    cur_block_id_ = -1;
    basic_blocks_.clear();
    func_blocks_.clear();
}

void Intermediate::new_basic_block() {
    cur_block_id_ += 1;
    BasicBlock basic_block = BasicBlock(cur_block_id_);
    this->basic_blocks_.push_back(basic_block);
}

void Intermediate::new_func_block(std::string func_name) {
    FuncBlock func_block = FuncBlock(std::move(func_name));
    this->func_blocks_.push_back(func_block);
}

void Intermediate::Optimize() {
    std::string bf_dce_path = "interm_bf-dce.txt";
    std::ofstream bf_dce_out(bf_dce_path);

    int opt_times = 10;
    for (int i = 0; i < opt_times; ++i) {
        reset_blocks();
        peephole_optimize();
        divide_blocks();
        construct_flow_rel();
        add_modified_symbols();
        add_read_symbols();
        check_print_getint_memo();
        common_expr();
        gen_def_and_use();
        gen_in_and_out();
        if (i == 0) {
            OutputFuncBlocks(bf_dce_out);
            bf_dce_out.close();
        }
        dead_code_elimination();
        delete_useless_loop_in_main();
        sync_codes();
    }
    InlineFunc();
    for (int i = 0; i < opt_times; ++i) {
        reset_blocks();
        peephole_optimize();
        divide_blocks();
        construct_flow_rel();
        add_modified_symbols();
        add_read_symbols();
        check_print_getint_memo();
        common_expr();
        gen_def_and_use();
        gen_in_and_out();
        if (i == 0) {
            OutputFuncBlocks(bf_dce_out);
            bf_dce_out.close();
        }
        dead_code_elimination();
        delete_useless_loop_in_main();
        sync_codes();
    }
    peephole_optimize();
    gen_func_conflict_graph();
}

void Intermediate::peephole_optimize() {
    if (codes_.size() <= 1) return;

    // remove useless labels
    // 1. collect useful label
    // 2. for each jump, bne and beq, check if it is necessary
    auto it = codes_.begin();
    std::set<std::string> useful_labels = {};
    while (it != codes_.end()) {
        IntermOp &op = it->op;
        if (op == IntermOp::JUMP || op == IntermOp::BNE || op == IntermOp::BEQ) {
            useful_labels.insert(it->dst);
        }
        it++;
    }

    it = codes_.begin();
    while (it != codes_.end()) {
        IntermOp op = it->op;
        if (op == IntermOp::LABEL) {
            std::string dst_label = it->dst;
            if (useful_labels.find(dst_label) == useful_labels.end()) {
                // useless LABEL
                it = codes_.erase(it);
                continue;
            }
        }
        it++;
    }

    // temp elimination
    it = codes_.begin() + 1;
    while (it != codes_.end()) {
        // DIV temp 5 b <-- pre_it
        // ADD a temp 0 <-- it
        auto pre_it = it - 1;
        bool is_assign = ((it->op) == IntermOp::ADD) & (it->src2 == "0");
        std::string temp_name = pre_it->dst;
        bool dst_eq_src = (pre_it->dst[0] == '#') & (it->src1 == pre_it->dst);
//        auto it_check_use = it + 1;
//        bool temp_wont_use = true;
//        while (it_check_use != codes_.end()) {
//            if (it_check_use->src1 == temp_name || it_check_use->src2 == temp_name) {
//                temp_wont_use = false;
//                break;
//            }
//            it_check_use++;
//        }

        if (is_assign && dst_eq_src) {
            pre_it->dst = it->dst;
            it = codes_.erase(it);
            continue;
        }
        it++;
    }

    // remove self-assign
    // add a a 0, add a 0 a
    // sub a a 0
    it = codes_.begin();
    while (it != codes_.end()) {
        std::string dst = it->dst;
        IntermOp op = it->op;
        std::string src1 = it->src1;
        std::string src2 = it->src2;
        // add a a 0, add a 0 a
        if ((op == IntermOp::ADD) &&
            ((dst == src1 && src2 == "0") || (dst == src2 && src1 == "0"))) {
            it = codes_.erase(it);
        }
            // sub a a 0
        else if (op == IntermOp::SUB && (dst == src1 && src2 == "0")) {
            it = codes_.erase(it);
        }
            // mul a a 1, mul a 1 a
        else if ((op == IntermOp::MUL) &&
                 ((dst == src1 && src2 == "1") || (dst == src2 && src2 == "1"))) {
            it = codes_.erase(it);
        }
            // div a a 1
        else if (op == IntermOp::DIV && (dst == src1 && src2 == "1")) {
            it = codes_.erase(it);
        } else {
            it++;
        }
    }

    // JUMP, JUMP
    it = codes_.begin() + 1;
    while (it != codes_.end()) {
        auto pre_it = it - 1;
        if (it->op == IntermOp::JUMP && pre_it->op == IntermOp::JUMP) {
            it = codes_.erase(it);
            continue;
        }
        it++;
    }

    // assign statement convention
    // like: add a b 0, multi a c 1
    //       multi a 1 c, add a 0 b
    // must sure that src2 is 0
    it = codes_.begin();
    while (it != codes_.end()) {
        if (it->op == IntermOp::ADD && it->src1 == "0") {
            std::string re_src1 = it->src2;
            std::string re_src2 = it->src1;
            it->src1 = re_src2;
            it->src2 = re_src1;
        } else if (it->op == IntermOp::SUB && it->src2 == "0") {
            it->op = IntermOp::ADD;
        } else if (it->op == IntermOp::MUL) {
            // MUL dst 1 src2
            if (it->src1 == "1") {
                it->op = IntermOp::ADD;
                it->src1 = it->src2;
                it->src2 = "0";
            }
                // MUL dst src1 1
            else if (it->src2 == "1") {
                it->op = IntermOp::ADD;
                it->src2 = "0";
            } else {
                // pass
            }
        } else if (it->op == IntermOp::DIV && it->src2 == "1") {
            // DIV dst src1 1
            it->op = IntermOp::ADD;
            it->src2 = "0";
        }
        it++;
    }

    // const merge, make sure that src2 is 0
    // like: add a 5 2, mul a 3 2
    it = codes_.begin();
    while (it != codes_.end()) {
        IntermOp op = it->op;
        std::string src1 = it->src1;
        std::string src2 = it->src2;
        if ((is_arith(op) || is_cmp(op)) && is_integer(src1) && is_integer(src2)) {
            int value = 0;
            int src1_value = std::stoi(src1);
            int src2_value = std::stoi(src2);
            if (op == IntermOp::ADD) value = src1_value + src2_value;

            else if (op == IntermOp::SUB) value = src1_value - src2_value;

            else if (op == IntermOp::MUL) value = src1_value * src2_value;

            else if (op == IntermOp::DIV) value = src1_value / src2_value;

            else if (op == IntermOp::MOD) value = src1_value % src2_value;

            else if (op == IntermOp::EQ) value = (src1_value == src2_value);

            else if (op == IntermOp::NEQ) value = (src1_value != src2_value);

            else if (op == IntermOp::LSS) value = src1_value < src2_value;

            else if (op == IntermOp::LEQ) value = src1_value <= src2_value;

            else if (op == IntermOp::GRE) value = src1_value > src2_value;

            else if (op == IntermOp::GEQ) value = src1_value >= src2_value;
            it->op = IntermOp::ADD;
            it->src1 = std::to_string(value);
            it->src2 = "0";
        }

        if (op == IntermOp::SUB && (src1 == src2)) {
            it->op = IntermOp::ADD;
            it->src1 = "0";
            it->src2 = "0";
        }

        if (op == IntermOp::MUL && ((src1 == "0") || (src2 == "0"))) {
            it->op = IntermOp::ADD;
            it->src1 = "0";
            it->src2 = "0";
        }
        if (op == IntermOp::DIV && src1 == src2) {
            it->op = IntermOp::ADD;
            it->src1 = "1";
            it->src2 = "0";
        }

        if (op == IntermOp::MOD && src1 == src2) {
            it->op = IntermOp::ADD;
            it->src1 = "0";
            it->src2 = "0";
        }
        it++;
    }

    // swap integer order
    // make sure add a b 2 not add a 2 b
    it = codes_.begin();
    while (it != codes_.end()) {
        IntermOp op = it->op;
        std::string src1 = it->src1;
        std::string src2 = it->src2;
        if ((op == IntermOp::MUL || op == IntermOp::ADD)
            && (is_integer(src1) && !is_integer(src2))) {
            const std::string &tmp = src1;
            it->src1 = it->src2;
            it->src2 = tmp;
        }
        it++;
    }

    // NOT dst, src1
    it = codes_.begin();
    while (it != codes_.end()) {
        IntermOp op = it->op;
        std::string src1 = it->src1;
        std::string src2 = it->src2;
        if (op == IntermOp::NOT && is_integer(src1)) {
            int value = !(std::stoi(src1));
            it->op = IntermOp::ADD;
            it->src1 = std::to_string(value);
            it->src2 = "0";
        }
        it++;
    }

    // cmp when src1 == src2
    it = codes_.begin();
    while (it != codes_.end()) {
        IntermOp op = it->op;
        std::string src1 = it->src1, src2 = it->src2;
        int value = -1;
        if (is_cmp(op) && (src1 == src2)) {
            if (op == IntermOp::EQ || op == IntermOp::LEQ || op == IntermOp::GEQ) value = 1;
            else value = 0;

            it->op = IntermOp::ADD;
            it->src1 = std::to_string(value);
            it->src2 = "0";
        }
        it++;
    }
}


// @brief: divide codes into basic blocks and function blocks
void Intermediate::divide_blocks() {
    // divide basic blocks

    new_basic_block();
    for (int i = 0; i < codes_.size(); i++) {
        if (codes_[i].op == IntermOp::FUNC_BEGIN) {
            new_basic_block();
            basic_blocks_[cur_block_id_].AddLabelCode(codes_[i]);
        }
            // label
        else if (codes_[i].op == IntermOp::LABEL) {
            if (basic_blocks_[cur_block_id_].HasNoCodes()) {
                basic_blocks_[cur_block_id_].AddLabelCode(codes_[i]);
            } else {
                new_basic_block();
                basic_blocks_[cur_block_id_].AddLabelCode(codes_[i]);
            }
        }
            // JUMP, BNE, BEQ
        else if (codes_[i].op == IntermOp::JUMP || codes_[i].op == IntermOp::BNE || codes_[i].op == IntermOp::BEQ) {
            basic_blocks_[cur_block_id_].AddJBCode(codes_[i]);
            new_basic_block();
        }
            // RET
        else if (codes_[i].op == IntermOp::RET) {
            if (basic_blocks_.back().HasNoCodes()) {

            } else {
                new_basic_block();
            }
            basic_blocks_.back().AddCode(codes_[i]);
        } else if (codes_[i].op == IntermOp::FUNC_END) {
            basic_blocks_[cur_block_id_].AddJBCode(codes_[i]);
        } else {
            basic_blocks_.back().AddCode(codes_[i]);
        }
    }

    // divide function blocks
    int i = 0;
    while (i < basic_blocks_.size()) {
        if (!basic_blocks_[i].label_codes_.empty() &&
            basic_blocks_[i].label_codes_[0].op == IntermOp::FUNC_BEGIN) {
            std::string func_name = basic_blocks_[i].label_codes_[0].dst;
            new_func_block(func_name);
            FuncBlock &func_block = func_blocks_.back();

            // construct it's params
            std::pair<bool, TableEntry *> search_res = symbol_table_.SearchFunc(func_name);
            if (!search_res.first) handle_error("func can't be found when add modified symbols");
            int param_num = search_res.second->value;
            for (int j = 0; j < param_num; ++j) {
                TableEntry *param_entry = symbol_table_.GetKthParam(func_name, j);
                if (param_entry->data_type == DataType::INT_ARR) {
                    func_block.AddArrParam(param_entry->name, j);
                } else {
                    continue;
                }
            }

            // add the basic block id to function block
            while (!(!basic_blocks_[i].jb_codes_.empty() &&
                     basic_blocks_[i].jb_codes_.back().op == IntermOp::FUNC_END)) {
                func_blocks_.back().AddBlock(i);
                i += 1;
            }
            func_blocks_.back().AddBlock(i);
        }
        i += 1;
    }
}

void Intermediate::construct_flow_rel() {
    //
    BasicBlock &block = basic_blocks_[0];
    if (block.label_codes_.empty()) {
        std::pair<bool, int> search_res = search_func_block_by_name("main");
        FuncBlock &func_block = func_blocks_[search_res.second];
        int main_block_id = func_block.block_ids_[0];
        BasicBlock &main_first_block = basic_blocks_[main_block_id];
        block.AddSucc(main_block_id);
        main_first_block.AddPred(0);
    }


    for (const auto &func_block: func_blocks_) {
        for (int id: func_block.block_ids_) {
            // now we have a basic block

            // ret block don't have succ
            if (basic_blocks_[id].IsRetBlock()) {
                continue;
            }

            // the jb codes is empty
            if (basic_blocks_[id].jb_codes_.empty()) {
                basic_blocks_[id].AddSucc(id + 1);
                basic_blocks_[id + 1].AddPred(id);
            }
                // the jb codes length must be one
            else {
                if (basic_blocks_[id].jb_codes_.size() != 1) {
                    std::string msg = "jb_code of block #" + std::to_string(id) + "is not 1";
                    handle_error(msg);
                }

                    // JUMP, BNE, BEQ
                else {
                    std::string label = basic_blocks_[id].jb_codes_[0].dst;
                    for (int j: func_block.block_ids_) {
                        if (basic_blocks_[j].ContainsLabel(label)) {
                            basic_blocks_[id].AddSucc(j);
                            basic_blocks_[j].AddPred(id);
                        }
                    }
                    if (basic_blocks_[id].jb_codes_[0].op == IntermOp::BNE ||
                        basic_blocks_[id].jb_codes_[0].op == IntermOp::BEQ) {
                        basic_blocks_[id].AddSucc(id + 1);
                        basic_blocks_[id + 1].AddPred(id);
                    }
                }
            }
        }
    }
}

// @brief: add the global variables that the function modified,
//         add the param array order that the function modified
void Intermediate::add_modified_symbols() {
    for (auto &func_block: func_blocks_) {
        std::string &func_name = func_block.func_name_;

        std::pair<bool, TableEntry *> search_res = symbol_table_.SearchFunc(func_name);
        if (search_res.second->data_type != DataType::VOID) {
            func_block.AddModifiedSymbol("%RET");
        }

        for (int block_id: func_block.block_ids_) {
            BasicBlock &basic_block = basic_blocks_[block_id];
            for (int i = 0; i < basic_block.codes_.size(); i++) {
                auto &code = basic_block.codes_[i];
                IntermOp op = code.op;

                std::string dst = code.dst;
                if ((is_arith(code.op) || is_bitwise(code.op) || is_cmp(code.op) || (code.op == IntermOp::GETINT) ||
                     (code.op == IntermOp::ARR_LOAD)) && symbol_table_.is_global_symbol(code.dst)) {
                    func_block.AddModifiedSymbol(code.dst);
                }
                    // PRINT, GETINT both load an op_number to %RET but not read it
                else if (code.op == IntermOp::PRINT || code.op == IntermOp::GETINT) {
                    func_block.AddModifiedSymbol("%RET");
                }
                    // ARR_SAVE
                else if (false && code.op == IntermOp::ARR_SAVE) {
                    // the dst may be global or param
                    if (symbol_table_.is_global_symbol(dst)) func_block.AddModifiedSymbol(dst);
                    if (func_block.ContainsParamArr(dst)) func_block.AddModifiedParam(dst);
                }
                    // PREPARE_CALL
                else if (code.op == IntermOp::PREPARE_CALL) {
                    std::string callee_name = code.dst;
                    std::pair<bool, int> search_callee_res = search_func_block_by_name(callee_name);
                    if (!search_callee_res.first)
                        handle_error("can't find func " + callee_name + " in get modified param");
                    FuncBlock &callee_block = func_blocks_[search_callee_res.second];

                    func_block.modified_global_symbols_.insert(callee_block.modified_global_symbols_.begin(),
                                                               callee_block.modified_global_symbols_.end());
                } else if (op == IntermOp::INIT_ARR_PTR && symbol_table_.is_global_symbol(dst)) {
                    // dead code, in a function block, if init occur, it is the local arr
                    func_block.AddModifiedSymbol(dst);
                } else {
                    continue;
                }
            }
        }
    }
}

// @brief: the function is use while data-flowing and occurring a call
void Intermediate::add_read_symbols() {
    for (auto &func_block: func_blocks_) {
        std::string &func_name = func_block.func_name_;

        std::pair<bool, TableEntry *> search_res = symbol_table_.SearchFunc(func_name);

        for (int block_id: func_block.block_ids_) {
            BasicBlock &basic_block = basic_blocks_[block_id];
            for (int i = 0; i < basic_block.codes_.size(); i++) {
                auto &code = basic_block.codes_[i];
                std::string dst = code.dst;
                IntermOp op = code.op;
                std::string src1 = code.src1;
                std::string src2 = code.src2;

                if ((is_arith(code.op) || is_bitwise(code.op) || is_cmp(code.op))) {
                    // these instructions won't read an array, which means we don't need to
                    if (symbol_table_.is_global_symbol(src1)) func_block.AddReadSymbol(code.src1);
                    if (symbol_table_.is_global_symbol(src2)) func_block.AddReadSymbol(code.src2);

                }
                    // PRINT
                else if (code.op == IntermOp::PRINT && symbol_table_.is_global_symbol(dst)) {
                    func_block.AddReadSymbol(dst);
                }
                    // ARR_SAVE arr_name, index, value, save the value to arr in memo
                    // read the arr_name, index and value
                else if (code.op == IntermOp::ARR_SAVE) {
                    if (symbol_table_.is_global_symbol(dst)) func_block.AddReadSymbol(dst);
                    if (symbol_table_.is_global_symbol(src1)) func_block.AddReadSymbol(src1);
                    if (symbol_table_.is_global_symbol(src2)) func_block.AddReadSymbol(src2);
                }
                    // ARR_LOAD value, arr_name, index
                else if (op == IntermOp::ARR_LOAD) {
                    if (symbol_table_.is_global_symbol(src1)) func_block.AddReadSymbol(code.src1);
                    if (symbol_table_.is_global_symbol(src2)) func_block.AddReadSymbol(code.src2);
                }
                    // PREPARE_CALL
                else if (code.op == IntermOp::PREPARE_CALL) {
                    std::string callee_name = code.dst;
                    std::pair<bool, int> search_callee_res = search_func_block_by_name(callee_name);
                    if (!search_callee_res.first)
                        handle_error("can't find func " + callee_name + " in get modified param");
                    FuncBlock &callee_block = func_blocks_[search_callee_res.second];
                    func_block.read_global_symbols_.insert(callee_block.read_global_symbols_.begin(),
                                                           callee_block.read_global_symbols_.end());
                } else if ((op == IntermOp::PUSH_ARR || op == IntermOp::PUSH_VAL) &&
                           symbol_table_.is_global_symbol(dst)) {
                    func_block.AddReadSymbol(dst);
                } else {
                    continue;
                }
            }
        }
    }
}


void Intermediate::check_print_getint_memo() {
    for (auto &func_block: func_blocks_) {
        std::string func_name = func_block.func_name_;
        for (int block_id: func_block.block_ids_) {
            BasicBlock &block = basic_blocks_[block_id];
            for (IntermCode &code: block.codes_) {
                if (code.op == IntermOp::PRINT) {
                    func_block.has_print_ = true;

                } else if (code.op == IntermOp::GETINT) {
                    func_block.has_getint_ = true;

                } else if (code.op == IntermOp::ARR_SAVE) {
                    func_block.write_memo_ = true;
                } else if (code.op == IntermOp::CALL) {
                    std::string callee_name = code.dst;
                    std::pair<bool, int> search_res = search_func_block_by_name(callee_name);
                    if (!search_res.first) std::cerr << "func " + callee_name + " not found" << std::endl;
                    FuncBlock &callee_block = func_blocks_[search_res.second];
                    if (callee_block.has_print_) {
                        func_block.has_print_ = true;
                    }
                    if (callee_block.has_getint_) {
                        func_block.has_getint_ = true;
                    }
                    if (callee_block.write_memo_) {
                        func_block.write_memo_ = true;
                    }
                } else {
                    continue;
                }
            }
            if (func_block.has_print_ && func_block.has_getint_) {
                break; // next func
            } else {
                continue; // next block
            }
        }
    }
}

void Intermediate::common_expr() {
    for (auto &basic_block: basic_blocks_) {
        int node_id = 0;
        std::vector<DAGNode> nodes;
        std::vector<IntermCode> new_codes;
        std::unordered_map<std::string, int> symbol_to_node;
        BasicBlockDAGManager manager = BasicBlockDAGManager{};

        for (auto &code: basic_block.codes_) {
            IntermOp op = code.op;
            std::string dst = code.dst;
            std::string src1 = code.src1;
            std::string src2 = code.src2;
            if (op == IntermOp::CALL) {
                const std::string &func_name = dst;
                for (auto func_block: func_blocks_) {
                    if (func_block.func_name_ == func_name) {
                        manager.RemoveNodes(func_block.modified_global_symbols_);
                        break;
                    }
                }
            } else {
                code = manager.GetEvalCode(code);
            }
        }
    }

}

// @brief: after DAG, the codes in blocks is different from the codes in Intermediate,
//         so we need to sync them, then we can do the peephole optimize again
void Intermediate::sync_codes() {
    codes_.clear();
    for (auto &basic_block: basic_blocks_) {
        for (auto &code: basic_block.label_codes_) codes_.emplace_back(code);
        for (auto &code: basic_block.codes_) codes_.emplace_back(code);
        for (auto &code: basic_block.jb_codes_) codes_.emplace_back(code);
    }
}

// @brief: for each basic block, generate its def and use set.
void Intermediate::gen_def_and_use() {
    for (int i = 0; i < basic_blocks_.size(); i++) {
        BasicBlock &block = basic_blocks_[i];
        for (int j = 0; j < block.codes_.size(); j++) {
            IntermCode &code = block.codes_[j];
            IntermOp op = code.op;
            std::string dst = code.dst, src1 = code.src1, src2 = code.src2;
            if (is_arith(op) || is_bitwise(op) || is_cmp(op) || op == IntermOp::ARR_LOAD) {
                if (!src1.empty() && !is_integer(src1) && !block.ContainsDef(src1)) {
                    block.AddToUse(src1);
                }

                if (!src2.empty() && !is_integer(src2) && !block.ContainsDef(src2)) {
                    block.AddToUse(src2);
                }

                if (!block.ContainsUse(dst)) {
                    block.AddToDef(dst);
                }
            }
                // GETINT, INIT_ARR
            else if ((op == IntermOp::GETINT || op == IntermOp::INIT_ARR_PTR)
                     && !block.ContainsUse(dst)) {
                block.AddToDef(dst);
            }
                // PRINT
            else if (op == IntermOp::PRINT && !is_integer(dst) && src1 == "int"
                     && !block.ContainsDef(dst)) {
                block.AddToUse(dst);
            }
                // ARR_SAVE arr_name, index, value
            else if (op == IntermOp::ARR_SAVE) {
                if (!dst.empty() && !is_integer(dst) && !block.ContainsDef(dst)) {
                    block.AddToUse(dst);
                }

                if (!src1.empty() && !is_integer(src1) && !block.ContainsDef(src1)) {
                    block.AddToUse(src1);
                }

                if (!src2.empty() && !is_integer(src2) && !block.ContainsDef(src2)) {
                    block.AddToUse(src2);
                }
            }
                // PUSH
            else if ((op == IntermOp::PUSH_VAL || op == IntermOp::PUSH_ARR)
                     && !is_integer(dst) && !block.ContainsDef(dst)) {
                block.AddToUse(dst);
            }
                // global = func(global, input)
            else if (op == IntermOp::CALL) {
                std::pair<bool, int> search_res = search_func_block_by_name(dst);
                if (!search_res.first) handle_error("func " + dst + " not found while get def and use");
                FuncBlock &callee_block = func_blocks_[search_res.second];
                for (const std::string &read_symbol: callee_block.read_global_symbols_) {
                    if (!block.ContainsDef(read_symbol)) {
                        block.AddToUse(read_symbol);
                    }
                }
                for (const std::string &modified_symbol: callee_block.modified_global_symbols_) {
                    if (!block.ContainsUse(modified_symbol)) {
                        block.AddToDef(modified_symbol);
                    }
                }

            }
                // RET
            else if (op == IntermOp::RET) {
                if (!dst.empty() && !is_integer(dst) && !block.ContainsDef(dst)) {
                    block.AddToUse(dst);
                }
            } else {
                continue;
            }
        }

        // u need to for-each the jb code as well, especially the bne, beq
        for (int j = 0; j < block.jb_codes_.size(); j++) {
            IntermCode &code = block.jb_codes_[j];
            IntermOp op = code.op;
            std::string dst = code.dst, src1 = code.src1, src2 = code.src2;
            if (op == IntermOp::BNE || op == IntermOp::BEQ) {
                if (!src1.empty() && !is_integer(src1) && !block.ContainsDef(src1)) {
                    block.AddToUse(src1);
                }

                if (!src2.empty() && !is_integer(src2) && !block.ContainsDef(src2)) {
                    block.AddToUse(src2);
                }
            } else {
                continue;
            }
        }
    }
}

// @brief: for each basic block, generate its in and out,
//         out = U (in_succ)
//         in = use + (out - def)
void Intermediate::gen_in_and_out() {
    bool set_not_change = false;
    for (int i = 0; i < func_blocks_.size(); i++) {
        FuncBlock &func_block = func_blocks_[i];
        bool continue_cal_in_out = true;
        while (continue_cal_in_out) {
            continue_cal_in_out = false;
            for (int j = 0; j < func_block.block_ids_.size(); j++) {
                BasicBlock &block = basic_blocks_[func_block.block_ids_[j]];
                for (int succ_i: block.succ_blocks_) {
                    // for the succ blocks, we need their in to construct our new_out
                    block.extend_new_out(basic_blocks_[succ_i].in_);
                }
                block.cal_new_in();
                bool in_out_change = !block.in_out_not_change();
                if (in_out_change) block.sync_in_out();
                // once an in_out change, u need to
                continue_cal_in_out |= in_out_change;
            }
        }
    }

    // there is global variables
//    if (basic_blocks_[0].label_codes_[0].op != IntermOp::FUNC_BEGIN) {
//        std::pair<bool, int> search_res = search_func_block_by_name("main");
//        FuncBlock& main_block = func_blocks_[search_res.second];
//        int main_id = main_block.block_ids_[0];
//        basic_blocks_[0].out_ = basic_blocks_[main_id].in_;
//    }

}

// @note: ???????????????????????????????????????????????????**???????????????**???????????????????????????(???????????????)??????$out_S$???????????????????????????????????????
//        ???????????????????????????????????????"????????????????????????"?????????????????????????????????
//        ???????????????????????????????????????`a = call b`??????????????????????????????????????????????????????????????????
void Intermediate::dead_code_elimination() {
    // for each basic block, not the global block
    // from the jb codes end to code end
    // cal each statement's use and def
    // some statement mustn't be deleted, direct add to new out
    // check if def in the out, and def is not the global symbol
    // then we delete it, don't update the new and out, just continue
    for (int i = 0; i < func_blocks_.size(); i++) {
        FuncBlock &func_block = func_blocks_[i];
        for (int j = 0; j < func_block.block_ids_.size(); j++) {
            BasicBlock &block = basic_blocks_[func_block.block_ids_[j]];
            std::set<std::string> out = block.out_; // a copy for the block's out

            auto jb_it = block.jb_codes_.rbegin();
            for (; jb_it != block.jb_codes_.rend(); jb_it++) {
                IntermOp op = jb_it->op;
                std::string src1 = jb_it->src1, src2 = jb_it->src2;
                // extent the out
                if (op == IntermOp::BNE || op == IntermOp::BEQ) {
                    if (!src1.empty() && !is_integer(src1)) out.insert(src1);
                    if (!src2.empty() && !is_integer(src2)) out.insert(src2);
                }
            }

            auto code_it = block.codes_.rbegin();
            for (; code_it != block.codes_.rend();) {
                IntermOp op = code_it->op;
                std::string dst = code_it->dst, src1 = code_it->src1, src2 = code_it->src2;

                // ARR_LOAD
                // arr_load value, arr_name, index
                if (is_arith(op) || is_bitwise(op) || is_cmp(op) ||
                    op == IntermOp::ARR_LOAD || op == IntermOp::INIT_ARR_PTR) {
                    if (out.find(dst) == out.end() && !symbol_table_.is_global_symbol(dst)) {
                        // erase this code
                        code_it = std::vector<IntermCode>::reverse_iterator(block.codes_.erase((++code_it).base()));
                    } else {
                        // extent the out
                        // erase the def
                        out.erase(dst);
                        // involve the use
                        if (!src1.empty() && !is_integer(src1)) out.insert(src1);
                        if (!src2.empty() && !is_integer(src2)) out.insert(src2);
                        code_it++;
                    }
                }
                    // GETINT
                else if (op == IntermOp::GETINT && !symbol_table_.is_global_symbol(dst)) {
                    out.erase(dst);
                    code_it++;
                }
                    // PRINT
                    // be care for print %RET, through %RET is change after PRINT,
                    // we need to add %RET, out bigger won't wrong
                else if (op == IntermOp::PRINT) {
                    if (src1 == "int" && !dst.empty() && !is_integer(dst)) {
                        out.insert(dst);
                    } else {
                        // print an integer or string, pass
                    }
                    code_it += 1;
                }
                    // ARR_SAVE, change the memory
                    // arr_save arr_name, index, value
                    // the three symbols are all we need, so extent the use
                else if (op == IntermOp::ARR_SAVE) {
                    if (!src1.empty() && !is_integer(src1)) out.insert(src1);
                    if (!src2.empty() && !is_integer(src2)) out.insert(src2);
                    if (!dst.empty() && !is_integer(dst)) out.insert(dst);
                    code_it += 1;
                }


                    // CALL occurs, we need to consider the call block as a whole
                    // @pre: we promise in parser that param have already been calculated, so "push" is linked
                    // PUSH_ARR, PUSH_VAL
                    // todo: if a function not modified the global symbol, not arr_save, not print, we remove the function
                else if (op == IntermOp::CALL) {
                    std::pair<bool, int> search_res = search_func_block_by_name(dst);
                    if (!search_res.first) std::cerr << "CALL func not found in dead code elimination" << std::endl;
                    FuncBlock &callee_block = func_blocks_[search_res.second];
                    out.insert(callee_block.read_global_symbols_.begin(),
                               callee_block.read_global_symbols_.end());
                    // read the PUSH_VAL and PUSH_ARR
                    code_it++;
                    while (code_it->op != IntermOp::PREPARE_CALL) {
                        std::string push_dst = code_it->dst;
                        if (code_it->op == IntermOp::PUSH_VAL || code_it->op == IntermOp::PUSH_ARR) {
                            if (!dst.empty() && !is_integer(push_dst)) out.insert(push_dst);
                            code_it++;
                        }
                    }
                    // now op == prepare call
                    // remove the func change symbols from out
                    str_set_diff(out, out, callee_block.modified_global_symbols_);
                    code_it++; // jump out the call-statement
                }
                    // RET, we can't delete return statement
                else if (op == IntermOp::RET) {
                    if (!dst.empty() && !is_integer(dst)) out.insert(dst);
                    code_it++;
                } else {
                    code_it++;
                }
            }
        }
    }
}

std::pair<bool, int> Intermediate::search_func_block_by_name(const std::string &func_name) {
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

// @brief: delete useless loop in main
// @note: if the loop block does not modify the in of next block and
//        this block don't print and
//        this block don't getint,
//        we can delete this loop block, i.e. empty the codes and jb_codes
void Intermediate::delete_useless_loop_in_main() {
    std::pair<bool, int> search_res = search_func_block_by_name("main");
    FuncBlock &func_block = func_blocks_[search_res.second];
    for (int block_id: func_block.block_ids_) {
        BasicBlock &basic_block = basic_blocks_[block_id];

        int id = basic_block.id_;
        bool self_loop = !(basic_block.succ_blocks_.find(id) == basic_block.succ_blocks_.end());
        if (!self_loop) continue;
        if (basic_block.succ_blocks_.size() != 2) continue;
        // now we can sure that it is self-loop and has a succ
        std::vector<int> succ_ids;
        int next_id = -1;
        succ_ids.assign(basic_block.succ_blocks_.begin(), basic_block.succ_blocks_.end());
        if (succ_ids[0] == id) {
            next_id = succ_ids[1];
        } else {
            next_id = succ_ids[0];
        }
        // now we can sure it is self-loop and succ is its next
        BasicBlock &next_block = basic_blocks_[next_id];
        bool modified_next_in = false;
        std::set<std::string> next_block_in = basic_blocks_[next_id].in_;
        std::set<std::string> block_modified_symbols = {};
        bool has_print = false, has_getint = false, write_memo = false;

        for (auto &code: basic_block.codes_) {
            std::string dst = code.dst;
            IntermOp op = code.op;
            if (op_modify_dst(op)) {
                block_modified_symbols.insert(dst);
                if (op == IntermOp::GETINT) {
                    has_getint = true;
                    break;
                }
            } else if (op == IntermOp::PRINT) {
                has_print = true;
                break;
            } else if (op == IntermOp::ARR_SAVE) {
                write_memo = true;
                break;
            } else if (op == IntermOp::CALL) {
                const std::string &callee_name = dst;
                std::pair<bool, int> search_callee_res = search_func_block_by_name(callee_name);
                if (!search_callee_res.first)
                    std::cerr << "func " + callee_name + " not found in delete useless loop" << std::endl;
                FuncBlock &callee_block = func_blocks_[search_res.second];
                block_modified_symbols.insert(callee_block.modified_global_symbols_.begin(),
                                              callee_block.modified_global_symbols_.end());
                if (callee_block.has_print_) {
                    has_print = true;
                    break;
                }
                if (callee_block.has_getint_) {
                    has_getint = true;
                    break;
                }

                if (callee_block.write_memo_) {
                    write_memo = true;
                    break;
                }
            }
        }
        // now we know whether this block modify and has getint and print
        for (const std::string &symbol_next_in: next_block_in) {
            if (block_modified_symbols.find(symbol_next_in) != block_modified_symbols.end()) {
                modified_next_in = true;
                break;
            }
        }
        if (!has_getint && !has_print && !write_memo && !modified_next_in) {
            std::cout << "the loop block get delete" << std::endl;
            basic_block.codes_.clear();
            basic_block.jb_codes_.clear();
        }
    }


}

void Intermediate::remove_global_symbols(std::set<std::string> &s) {
    auto it = s.begin();
    while (it != s.end()) {
        if (symbol_table_.is_global_symbol(*it)) {
            it = s.erase(it);
        } else {
            it++;
        }
    }

}

// @brief: generate conflict graph for each function
void Intermediate::gen_func_conflict_graph() {
    for (FuncBlock &func_block: func_blocks_) {
        // the conflict graph is ready
        ConflictGraph &graph = func_block.conflict_graph_;
        for (int block_id: func_block.block_ids_) {
            BasicBlock &block = basic_blocks_[block_id];
            std::set<std::string> out = block.out_;
            remove_global_symbols(out);
            graph.extent_by_set(out);

            auto jb_it = block.jb_codes_.rbegin();
            for (; jb_it != block.jb_codes_.rend(); jb_it++) {
                IntermOp op = jb_it->op;
                std::string src1 = jb_it->src1, src2 = jb_it->src2;
                // extent the out
                if (op == IntermOp::BNE || op == IntermOp::BEQ) {
                    if (!src1.empty() && !is_integer(src1)) out.insert(src1);
                    if (!src2.empty() && !is_integer(src2)) out.insert(src2);
                    remove_global_symbols(out);
                    graph.extent_by_set(out);
                }
            }

            auto code_it = block.codes_.rbegin();
            for (; code_it != block.codes_.rend();) {
                IntermOp op = code_it->op;
                std::string dst = code_it->dst, src1 = code_it->src1, src2 = code_it->src2;

                // ARR_LOAD
                // arr_load value, arr_name, index
                if (is_arith(op) || is_bitwise(op) || is_cmp(op) || op == IntermOp::ARR_LOAD) {
                    // extent the out
                    // erase the def
                    out.erase(dst);
                    // involve the use
                    if (!src1.empty() && !is_integer(src1)) out.insert(src1);
                    if (!src2.empty() && !is_integer(src2)) out.insert(src2);
                }
                    // GETINT
                else if (op == IntermOp::GETINT && !symbol_table_.is_global_symbol(dst)) {
                    out.insert(dst);
                    graph.extent_by_set(out);
                }
                    // PRINT
                    // be care for print %RET, through %RET is change after PRINT,
                    // we need to add %RET, out bigger won't wrong
                else if (op == IntermOp::PRINT) {
                    if (src1 == "int" && !dst.empty() && !is_integer(dst)) {
                        out.insert(dst);
                    } else {
                        // print an integer or string, pass
                    }
                }
                    // ARR_SAVE, change the memory
                    // arr_save arr_name, index, value
                    // the three symbols are all we need, so extent the use
                else if (op == IntermOp::ARR_SAVE) {
                    if (!src1.empty() && !is_integer(src1)) out.insert(src1);
                    if (!src2.empty() && !is_integer(src2)) out.insert(src2);
                    if (!dst.empty() && !is_integer(dst)) out.insert(dst);
                }

                    // CALL occurs, we need to consider the call block as a whole
                    // @pre: we promise in parser that param have already been calculated, so "push" is linked
                    // PUSH_ARR, PUSH_VAL
                else if (op == IntermOp::CALL) {
                    std::pair<bool, int> search_res = search_func_block_by_name(dst);
                    if (!search_res.first) std::cerr << "CALL func not found in construct conflict graph" << std::endl;
                    FuncBlock &callee_block = func_blocks_[search_res.second];
                    out.insert(callee_block.read_global_symbols_.begin(),
                               callee_block.read_global_symbols_.end());
                    // read the PUSH_VAL and PUSH_ARR
                    code_it++;
                    while (code_it->op != IntermOp::PREPARE_CALL) {
                        std::string push_dst = code_it->dst;
                        if (code_it->op == IntermOp::PUSH_VAL || code_it->op == IntermOp::PUSH_ARR) {
                            if (!dst.empty() && !is_integer(push_dst)) out.insert(push_dst);
                            code_it++;
                        }
                    }
                    // now op == prepare call
                    // remove the func change symbols from out
                    str_set_diff(out, out, callee_block.modified_global_symbols_);
                }
                    // RET, we can't delete return statement
                else if (op == IntermOp::RET) {
                    if (!dst.empty() && !is_integer(dst)) out.insert(dst);
                }
                    // INIT_ARR_PTR, pass
                    // @pre: we promise if an array_init, i will be used after that
                else {
                    out.erase(dst);
                }
                remove_global_symbols(out);
                graph.extent_by_set(out);
                code_it++;
            }
        }
    }
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

std::string interm_code_to_string(const IntermCode &code, bool auto_indent) {
    std::string output;
    std::string indent = (auto_indent) ? "    " : "";
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

