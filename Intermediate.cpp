//
// Created by WYSJ6174 on 2021/10/17.
//

#include "Intermediate.h"

#include <utility>


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

bool is_modify_op(IntermOp op) {
    if (is_cmp(op) && is_arith(op) || is_bitwise(op) || op == IntermOp::GETINT || op == IntermOp::ARR_LOAD) {
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

//
void Intermediate::handle_error(std::string msg) {
    std::cout << msg << std::endl;
}

void Intermediate::Optimize() {
    if (enable_peephole_) peephole_optimize();
    divide_basic_block();
    construct_flow_rel();
    add_modified_symbols();
}

void Intermediate::peephole_optimize() {
    if (codes_.size() == 1) return;
    auto it = codes_.begin() + 1;
    while (it != codes_.end()) {
        // DIV temp 5 b <-- pre_it
        // ADD a temp 0 <-- it
        auto pre_it = it - 1;
        bool is_assign = ((it->op) == IntermOp::ADD) & (it->src2 == "0");
        bool dst_eq_src = (pre_it->dst[0] == '#') & (it->src1 == pre_it->dst);
        if (is_assign && dst_eq_src) {
            pre_it->dst = it->dst;
            it = codes_.erase(it);
            continue;
        }
        it++;
    }

    // self assign
    it = codes_.begin();
    while (it != codes_.end()) {
        if ((it->op == IntermOp::ADD || it->op == IntermOp::SUB) && it->dst == it->src1 && it->src2 == "0") {
            it = codes_.erase(it);
            continue;
        }
        if (it->op == IntermOp::MUL || it->op == IntermOp::DIV && it->dst == it->src1 && it->src2 == "1") {
            it = codes_.erase(it);
            continue;
        }
        it++;
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

    // todo: useless label removal

    // todo: assign statement convention
    // like: add a b 0, mult a c 1
    //       mult a 1 c, add a 0 b
    // must sure that src2 is 0

    // todo: const merge, make sure that src2 is 0
    // like: add a 5 2, mult a 3 2
}

void Intermediate::divide_basic_block() {
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

    int i = 0;
    while (i < basic_blocks_.size()) {
        if (!basic_blocks_[i].label_codes_.empty() &&
            basic_blocks_[i].label_codes_[0].op == IntermOp::FUNC_BEGIN) {
            std::string func_name = basic_blocks_[i].label_codes_[0].dst;
            new_func_block(func_name);
            func_blocks_.back().AddBlock(i);
            i += 1;
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


void Intermediate::new_basic_block() {
    cur_block_id_ += 1;
    BasicBlock basic_block = BasicBlock(cur_block_id_);
    this->basic_blocks_.push_back(basic_block);
}

void Intermediate::new_func_block(std::string func_name) {
    FuncBlock func_block = FuncBlock(std::move(func_name));
    this->func_blocks_.push_back(func_block);
}

void Intermediate::add_modified_symbols() {
    for (auto &func_block: func_blocks_) {
        for (int block_id: func_block.block_ids_) {
            BasicBlock &basic_block = basic_blocks_[block_id];
            for (auto &code: basic_block.codes_) {
                if (is_arith(code.op) || is_bitwise(code.op) || is_cmp(code.op) || (code.op == IntermOp::GETINT) ||
                    (code.op == IntermOp::ARR_LOAD)) {
                    func_block.AddModifiedSymbol(code.dst);
                }
            }
        }
    }

}

std::pair<bool, int> search_in_nodes(std::vector<DAGNode> &nodes, IntermCode &code) {
    IntermOp op = code.op;
    std::string dst = code.dst;
    std::string src1 = code.src1;
    std::string src2 = code.src2;

    for (auto &node: nodes) {
        if (node.op_ == op) {
            if (op == IntermOp::ADD || op == IntermOp::MUL) {
                if (nodes[node.sons_[0]].ContainsSymbol(src1) && nodes[node.sons_[1]].ContainsSymbol(src2) ||
                    nodes[node.sons_[0]].ContainsSymbol(src2) && nodes[node.sons_[1]].ContainsSymbol(src1)) {
                    return std::make_pair(true, node.id_);
                } else {
                    continue;
                }
            } else {
                if (nodes[node.sons_[0]].ContainsSymbol(src1) && nodes[node.sons_[1]].ContainsSymbol(src2)) {
                    return std::make_pair(true, node.id_);
                } else {
                    continue;
                }
            }
        } else {
            continue;
        }
    }
}

void Intermediate::common_expr() {
    for (auto &basic_block: basic_blocks_) {
        int node_id = 0;
        std::vector<DAGNode> nodes;
        std::vector<IntermCode> new_codes;
        std::unordered_map<std::string, int> symbol_to_node;

        for (auto &code: basic_block.codes_) {
            IntermOp op = code.op;
            std::string dst = code.dst;
            std::string src1 = code.src1;
            std::string src2 = code.src2;
            int dst_node_id = -1;
            auto it_dst = symbol_to_node.find(src1);
            if (it_dst != symbol_to_node.end()) {
                dst_node_id = it_dst->second; // mark it, may change
            }

            auto search_res = search_in_nodes(nodes, code);
            // there is a pattern for op dst src1 src2, so we can use common expr
            if (search_res.first) {
                std::string copy_name = nodes[search_res.second].GetSymbolName();
                new_codes.emplace_back(IntermOp::ADD, dst, copy_name, "0");
                if (dst_node_id != -1) {
                    nodes[dst_node_id].RemoveSymbol(dst);
                }
                symbol_to_node[dst] = search_res.second;
                nodes[search_res.second].AddSymbol(dst);
            }
            // get a node id for src1, if not exits, generate
            // get a node id for src2, if not exits, generate
            // gen a new node for the dst, then remove from old

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

