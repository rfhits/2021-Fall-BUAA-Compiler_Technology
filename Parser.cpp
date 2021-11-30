//
// Created by WYSJ6174 on 2021/10/3.
//

#include "Parser.h"

#define DBG true

Parser::Parser(SymbolTable &symbol_table, Lexer &lexer, ErrorHandler &error_handler, Intermediate &intermediate,
               bool print_mode, std::ofstream &out) :
        symbol_table_(symbol_table), lexer_(lexer), error_handler_(error_handler),
        intermediate_(intermediate), print_mode_(print_mode), out_(out) {}


// if pos is behind read_tokens
// then read from it
// else use get_token() to get a token from src
void Parser::next_sym() {
    // May have retracted
    if (pos_ < read_tokens_.size()) {
        token_ = read_tokens_[pos_];
    } else {
        token_ = lexer_.get_token();
        read_tokens_.push_back(token_);
    }
    type_code_ = token_.get_type_code();
    pos_ += 1;
    out_strings_.push_back(token_.to_string());
}

int Parser::get_prev_sym_line_no() {
    Token prev_token = read_tokens_[pos_ - 1];
    return prev_token.get_line_no();
}

// change pos and output_strings
void Parser::retract() {
    pos_ -= 1;
    token_ = read_tokens_[pos_ - 1];
    type_code_ = token_.get_type_code();

    // erase the output until meets
    for (unsigned int i = out_strings_.size() - 1; i >= 0; i--) {
        if (out_strings_[i][0] != '<') {
            out_strings_.erase(out_strings_.begin() + i);
            break;
        }
    }
}

void Parser::output(const std::string &msg) {
    out_strings_.push_back(msg);
}

// @brief: log a msg with current token line no
void Parser::handle_error(const std::string &msg) {
    error_handler_.log_error_with_line_no(token_.get_line_no(), msg);
}

// @brief: handle error at specific line
void Parser::add_error(int line_no, ErrorType error_type) {
    if (DBG) {
        error_handler_.log_error_with_line_no(line_no, error_type_to_alpha.find(error_type)->second);
    } else {
        errors.push_back(std::make_pair(line_no, error_type));
    }
}

void Parser::add_error(ErrorType error_type) {
    int line_no = token_.get_line_no();
    std::cerr << "error happended" << std::endl;
    if (DBG) {
        error_handler_.log_error_with_line_no(token_.get_line_no(), error_type_to_alpha.find(error_type)->second);
    } else {
        errors.push_back(std::make_pair(line_no, error_type));
    }
}

void Parser::output_error() {
    sort(errors.begin(), errors.end(),
         [](std::pair<int, ErrorType> a, std::pair<int, ErrorType> b)
                 -> bool {
             return a.first < b.first;
         }
    );
    for (std::pair<int, ErrorType> a: errors) {
        std::string err_msg;
        err_msg += std::to_string(a.first);
        err_msg += " ";
        std::string err_id = error_type_to_alpha.find(a.second)->second;
        err_msg += err_id;
        error_handler_.log_error(err_msg);
    }
    if (errors.size() > 0) {
        std::cout << "error happened" << std::endl;
    }
}

// CompUnit -> {Decl} {FuncDef} MainFuncDef
void Parser::Program() {
    next_sym();
    // three cond: const / int / void
    // three branches: Decl, Func, Main
    while (type_code_ == TypeCode::CONSTTK || type_code_ == TypeCode::INTTK) {
        if (type_code_ == TypeCode::CONSTTK) {
            Decl();
            next_sym();
        } else { // must be 'int' here
            next_sym(); // int a
            if (type_code_ == TypeCode::IDENFR) {
                next_sym();
                if (type_code_ == TypeCode::COMMA || type_code_ == TypeCode::SEMICN ||
                    type_code_ == TypeCode::ASSIGN || type_code_ == TypeCode::LBRACK) {
                    retract();
                    retract();
                    Decl();
                    next_sym();
                } else if (type_code_ == TypeCode::INTTK || type_code_ == TypeCode::VOIDTK ||
                           type_code_ == TypeCode::CONSTTK) {
                    // error handle branch, expect ';'
                    retract();
                    retract();
                    Decl();
                    next_sym();
                } else {
                    retract();
                    retract();
                    break;
                }
            } else {
                retract();
                break;
            }
        }
    }

    // promise there is only one token read
    // {FuncDef}
    while (type_code_ == TypeCode::VOIDTK || type_code_ == TypeCode::INTTK) {
        if (type_code_ == TypeCode::VOIDTK) {
            local_addr_ = local_addr_init;
            FuncDef();
            next_sym();
        } else { // must be 'int'
            next_sym();
            if (type_code_ == TypeCode::IDENFR) {
                retract();
                local_addr_ = local_addr_init;
                FuncDef();
                next_sym();
            } else {
                retract();
                break;
            }
        }
    }

    // MainDef
    if (type_code_ == TypeCode::INTTK) {
        local_addr_ = local_addr_init;
        MainFuncDef();
    } else {
        handle_error("expect 'int' head of MainFuncDef");
    }
    symbol_table_.PopLevel("", 0);

    out_strings_.emplace_back("<CompUnit>");
    if (print_mode_) {
        auto it = out_strings_.begin();
        while (it != out_strings_.end()) {
            out_ << *it << std::endl;
            it += 1;
        }
    }
    output_error();
}

// Decl -> ConstDecl | VarDecl
// @pre: already read a CONST or INT
// @retval: the parsed statement type
BlockItemType Parser::Decl() {
    BlockItemType item_type = BlockItemType::INVALID;
    if (type_code_ == TypeCode::CONSTTK) {
        ConstDecl();
        item_type = BlockItemType::CONST_DECL;
    } else if (type_code_ == TypeCode::INTTK) {
        VarDecl();
        item_type = BlockItemType::VAR_DECL;
    } else {
        handle_error("Decl expect a const or int");
    }
    return item_type;
}

// ConstDecl -> const int ConstDef {, ConstDef} ';'
// @pre: already read a 'const'
void Parser::ConstDecl() {
    next_sym();
    next_sym(); // before entering <ConstDef>, read a token
    ConstDef();
    next_sym();
    while (type_code_ == TypeCode::COMMA) {
        next_sym();
        ConstDef();
        next_sym();
    }
    if (type_code_ == TypeCode::SEMICN) {
        output("<ConstDecl>");
    } else {
        retract(); // give the token back, we assume u forget to write the ';'
        add_error(ErrorType::EXPECTED_SEMICN);
    }

}

// ConstDef -> Ident {'[' ConstExp ']'} '=' ConstInitVal
// @brief: if const is an integer, save it to the symbol table,
//         else it should be an array, u need to save it to the symbol table and add to mid-code .text
// @pre: already read an Identifier
void Parser::ConstDef() {
    reset_sym();
    int id_line_no = token_.get_line_no();
    bool is_array = false;
    name_ = token_.get_str_value();
    alias_ = name_ + "_" + std::to_string(id_line_no);
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        is_array = true;
        dims_ += 1;
        next_sym();
        auto const_exp_ret = ConstExp();
        dim0_size_ = std::stoi(const_exp_ret.second);
        next_sym();
        if (type_code_ == TypeCode::RBRACK) {

        } else {
            retract(); // add the ahead identifier line no
            add_error(ErrorType::EXPECTED_BRACK);
        }
    } else {
        retract();
    }
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        dims_ += 1;
        next_sym();
        auto const_exp_ret = ConstExp();
        dim1_size_ = std::stoi(const_exp_ret.second);
        next_sym();
        if (type_code_ == TypeCode::RBRACK) {

        } else {
            retract();
            add_error(ErrorType::EXPECTED_BRACK);
        }
    } else {
        retract();
    }
    next_sym(); // now at '='
    next_sym();
    auto const_init_val_ret = ConstInitVal();
    if (const_init_val_ret.first == DataType::INT) { // not array
        int parsed_int = std::stoi(const_init_val_ret.second);

        if (!symbol_table_.AddSymbol(cur_func_name_, DataType::INT, SymbolType::CONST,
                                     name_, alias_, parsed_int, cur_level_, 0, 0, 0, local_addr_)) {
            add_error(id_line_no, ErrorType::REDEF);
        }
        // const int a = 5;
        intermediate_.AddMidCode(alias_, IntermOp::ADD, const_init_val_ret.second, "0");
        local_addr_ += 4;
    } else if (const_init_val_ret.first == DataType::INT_ARR) {
        intermediate_.AddMidCode(alias_, IntermOp::INIT_ARR_PTR, "", "");
        std::vector<int> parsed_int_arr = str_to_vec_int(const_init_val_ret.second);
        if (!symbol_table_.AddConstArray(cur_func_name_, name_, alias_, cur_level_,
                                         dims_, dim0_size_, dim1_size_, parsed_int_arr, local_addr_)) {
            add_error(id_line_no, ErrorType::REDEF);
        }
        local_addr_ += 4; // for the pointer address
        int length = (dims_ == 1) ? dim0_size_ : dim0_size_ * dim1_size_;
        for (int i = 0; i < length; i++) {
            intermediate_.AddMidCode(alias_, IntermOp::ARR_SAVE, i, parsed_int_arr[i]);
            local_addr_ += 4;
        }
    } else {
        handle_error("DataType error in ConstDef");
    }
    output("<ConstDef>");
}

// ConstExp -> AddExp
// @brief: try to parse an integer out
// @retval: return an integer in string as the second of pair
std::pair<DataType, std::string> Parser::ConstExp() {
    int ret_int_value = 0;
    std::pair<DataType, std::string> add_exp_ret = AddExp();
    if (is_integer(add_exp_ret.second)) {
        ret_int_value = std::stoi(add_exp_ret.second);
    } else {
        handle_error("<ConstExp> expect a const exp from AddExp");
    }
    output("<ConstExp>");
    return std::make_pair(add_exp_ret.first, std::to_string(ret_int_value));
}

// AddExp-> MulExp | AddExp ('+'|'-') MulExp
// AddExp-> MulExp { ('+'|'-') MulExp}
// @attention: left recurrence
// @pre: already read a token
// @brief: try to parse as integer
//         if can't, return a temp_var_name
std::pair<DataType, std::string> Parser::AddExp() {
    bool cur_be_parsed_int = false; // can be parsed as integer currently?
    std::string ret_var_name;
    std::pair<DataType, std::string> mul_exp_ret; // ret received from MulExp
    DataType data_type = DataType::INVALID;

    mul_exp_ret = MulExp();
    data_type = mul_exp_ret.first;
    ret_var_name = mul_exp_ret.second; // the second may be an integer

    if (is_integer(ret_var_name)) {
        cur_be_parsed_int = true;
    } else {
        cur_be_parsed_int = false;
    }

    next_sym();
    while (type_code_ == TypeCode::PLUS || type_code_ == TypeCode::MINU) {
        int sign = (type_code_ == TypeCode::PLUS) ? 1 : -1;
        //erase then read again
        retract();
        output("<AddExp>");
        next_sym();

        next_sym();
        mul_exp_ret = MulExp();
        if (cur_be_parsed_int && is_integer(mul_exp_ret.second)) {
            ret_var_name = std::to_string(std::stoi(ret_var_name)
                                          + sign * std::stoi(mul_exp_ret.second));
        } else {
            cur_be_parsed_int = false;
            // combine it with the var before
            std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
            local_addr_ += 4;
            IntermOp op = (sign == 1) ? IntermOp::ADD : IntermOp::SUB;
            intermediate_.AddMidCode(tmp_var_name, op, ret_var_name, mul_exp_ret.second);
            ret_var_name = tmp_var_name;
        }
        next_sym();
    }
    retract(); // not '+' or '-', retract
    output("<AddExp>");
    return std::make_pair(data_type, ret_var_name);
}

// MulExp -> UnaryExp | MulExp (* / %) UnaryExp
// MulExp -> UnaryExp {('*' | '/' | '%') UnaryExp}
// @attention: left recurrence
// @pre: already read a token
// @brief: try to parse it: 2*a, 2*3/4
std::pair<DataType, std::string> Parser::MulExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name;
    bool cur_be_parsed_int = false;
    std::pair<DataType, std::string> unary_exp_ret;

    unary_exp_ret = UnaryExp();
    ret_type = unary_exp_ret.first;
    ret_var_name = unary_exp_ret.second;
    if (is_integer(ret_var_name)) {
        cur_be_parsed_int = true;
    } else {
        cur_be_parsed_int = false;
    }

    next_sym();
    while (type_code_ == TypeCode::MULT || type_code_ == TypeCode::DIV || type_code_ == TypeCode::MOD) {
        int sign = (type_code_ == TypeCode::MULT) ? 0 : ((type_code_ == TypeCode::DIV) ? 1 : 2);

        // erase then read
        retract();
        output("<MulExp>");
        next_sym();

        next_sym();
        unary_exp_ret = UnaryExp();
        if (cur_be_parsed_int && is_integer(unary_exp_ret.second)) {
            int parsed_unary_exp_value = std::stoi(unary_exp_ret.second);
            int cur_parsed_value = std::stoi(ret_var_name);
            if (sign == 0) {
                ret_var_name = std::to_string(parsed_unary_exp_value * cur_parsed_value);
            } else if (sign == 1) {
                ret_var_name = std::to_string(cur_parsed_value / parsed_unary_exp_value);
            } else {
                ret_var_name = std::to_string(cur_parsed_value % parsed_unary_exp_value);
            }
        } else {
            cur_be_parsed_int = false;
            std::string tmp_var_name =
                    intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
            local_addr_ += 4;
            IntermOp op = (sign == 0) ? IntermOp::MUL : ((sign == 1) ? IntermOp::DIV : IntermOp::MOD);
            intermediate_.AddMidCode(tmp_var_name, op, ret_var_name, unary_exp_ret.second);
            ret_var_name = tmp_var_name;
        }
        next_sym();
    }
    // not *| / | %
    retract();
    output("<MulExp>");
    return std::make_pair(ret_type, ret_var_name);
}

// UnaryExp -> PrimaryExp |
//             Ident '(' [FuncRParams] ')' |
//             UnaryOp UnaryExp
// @pre: already read a token
// note: 7 branches
std::pair<DataType, std::string> Parser::UnaryExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name; // init value is ""
    int func_name_line = token_.get_line_no();
    bool cur_be_parsed_int = false;
    std::pair<DataType, std::string> inner_exp_ret;


    // UnaryExp -> Primary -> '(' Exp ')'
    if (type_code_ == TypeCode::LPARENT) {
        inner_exp_ret = PrimaryExp();
        ret_type = inner_exp_ret.first;
        ret_var_name = inner_exp_ret.second;
        if (is_integer(inner_exp_ret.second)) {
            cur_be_parsed_int = true;
        } else {
            cur_be_parsed_int = false;
        }
    }
        // UnaryExp -> Primary -> Number -> IntConst
    else if (type_code_ == TypeCode::INTCON) {
        inner_exp_ret = PrimaryExp();
        ret_type = inner_exp_ret.first;
        ret_var_name = inner_exp_ret.second;
    }
        // + | - | !
    else if (type_code_ == TypeCode::PLUS || type_code_ == TypeCode::MINU || type_code_ == TypeCode::NOT) {
        int op_no = UnaryOp(); // op_no is "op number"
        next_sym();
        inner_exp_ret = UnaryExp();
        ret_type = inner_exp_ret.first;
        ret_var_name = inner_exp_ret.second;
        if (is_integer(ret_var_name)) {
            cur_be_parsed_int = true;
            if (op_no == 0) { // +
                //
            } else if (op_no == 1) {
                ret_var_name = std::to_string(0 - std::stoi(ret_var_name));
            } else {
                ret_var_name = std::to_string(!std::stoi(ret_var_name));
            }
        } else { // can't be parsed as integer
            cur_be_parsed_int = false;
            if (op_no == 0) { // +
                // no change to final return var name, it is in inner_exp_ret
            } else if (op_no == 1) {  // -
                std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, inner_exp_ret.first, cur_level_,
                                                                   local_addr_);
                local_addr_ += 4;
                intermediate_.AddMidCode(tmp_var_name, IntermOp::SUB, "0", inner_exp_ret.second);
                ret_var_name = tmp_var_name;
            } else {
                std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, inner_exp_ret.first, cur_level_,
                                                                   local_addr_);
                local_addr_ += 4;
                intermediate_.AddMidCode(tmp_var_name, IntermOp::NOT, inner_exp_ret.second, "");
                ret_var_name = tmp_var_name;
            }
        }
    }
        // Ident '(' [FuncRParams] ')'
        // Ident { '[' Exp ']' } may occur in Primary Expression
        // promise: parse const won't go into this branch
    else if (type_code_ == TypeCode::IDENFR) {
        std::string called_func_name = token_.get_str_value();
        next_sym();
        if (type_code_ == TypeCode::LPARENT) { // now sure that: Ident '(' [FuncRParams] ')'
            retract();
            inner_exp_ret = CallFunc();
            ret_type = inner_exp_ret.first;
            ret_var_name = inner_exp_ret.second;
        }
            // only one branch left
        else {
            retract();
            inner_exp_ret = PrimaryExp(); // will go to LVal
            ret_type = inner_exp_ret.first;
            ret_var_name = inner_exp_ret.second;
        }
    } else {
        handle_error("Expect a token in FIRST(UnaryExp)");
    }
    output("<UnaryExp>");
    return std::make_pair(ret_type, ret_var_name);
}


// PrimaryExp -> '(' Exp ')' |
//                 LVal |
//                 Number
// first: '(', IDENFR, INTCON
// promise: already read a token
std::pair<DataType, std::string> Parser::PrimaryExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name;
    std::pair<DataType, std::string> inner_exp_ret;

    if (type_code_ == TypeCode::LPARENT) {
        next_sym();
        inner_exp_ret = Exp();
        ret_type = inner_exp_ret.first;
        ret_var_name = inner_exp_ret.second;
        next_sym();
        if (type_code_ == TypeCode::RPARENT) {
            // pass
        } else {
            retract();
            add_error(ErrorType::EXPECTED_PARENT);
        }
    } else if (type_code_ == TypeCode::IDENFR) {
        inner_exp_ret = LVal();
        ret_type = inner_exp_ret.first;
        ret_var_name = inner_exp_ret.second;
    } else if (type_code_ == TypeCode::INTCON) {
        ret_type = DataType::INT;
        int inner_value = Number();
        ret_var_name = std::to_string(inner_value);
    } else {
        handle_error("Expect a token in FIRST(PrimaryExp)");
    }
    output("<PrimaryExp>");
    return std::make_pair(ret_type, ret_var_name);
}

// Exp -> AddExp
std::pair<DataType, std::string> Parser::Exp() {
    std::pair<DataType, std::string> ret = AddExp();
    output("<Exp>");
    return ret;
}

// LVal -> Ident { '[' Exp ']' }
// @brief: find a value from defined variable or
//         fetch a value from array
//         need to generate a temp variable if fetch from array,
//         else just use the original alias
// @! : use the alias name
// @pre: use in fetch or be assigned to sth
// @pre: already read an identifier token
// @exception: undeclared variable
std::pair<DataType, std::string> Parser::LVal() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name;
    std::string ident;
    int ident_line_no;
    bool fetch_array = false;
    int dims = 0;
    std::string dim0_idx, dim1_idx;
    std::pair<DataType, std::string> inner_exp_ret;

    ident = token_.get_str_value();
    ident_line_no = token_.get_line_no();
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        fetch_array = true;
        dims += 1;
        next_sym();
        inner_exp_ret = Exp();
        dim0_idx = inner_exp_ret.second;
        next_sym(); // read ']'
        if (type_code_ != TypeCode::RBRACK) {
            retract();
            add_error(ErrorType::EXPECTED_BRACK);
        }
    } else {
        retract();
    }

    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        fetch_array = true;
        next_sym();
        dims += 1;
        inner_exp_ret = Exp();
        dim1_idx = inner_exp_ret.second;
        next_sym(); // read ']'
        if (type_code_ != TypeCode::RBRACK) {
            retract();
            add_error(ErrorType::EXPECTED_BRACK);
        }
    } else {
        retract();
    }

    std::pair<bool, TableEntry *> search_res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, ident);
    TableEntry res_ptr = TableEntry(search_res.second);
    TableEntry *entry_ptr = &res_ptr;
    if (!search_res.first) {
        add_error(ErrorType::UNDECL);
        ret_type = DataType::INT;
    } else {
        if (dims == 0) { // ident
            if (entry_ptr->dims == 0) {
                ret_type = entry_ptr->data_type;
                if (entry_ptr->symbol_type == SymbolType::CONST) {
                    ret_var_name = std::to_string(entry_ptr->value);
                } else {
                    // todo: none-array return alias for generate code,
                    //       array return name, because it will be use in FuncCall,which will use alias
//                    ret_var_name = entry_ptr->name;
                    ret_var_name = entry_ptr->alias;
                }
            } else if (entry_ptr->dims == 1) {
                // the identifier is a 1d array
                ret_type = DataType::INT_ARR;
                ret_var_name = entry_ptr->name;
            } else if (entry_ptr->dims == 2) {
                // the identifier is a 2d array
                ret_type = DataType::INT_ARR;
                ret_var_name = entry_ptr->name;
            } else {
                handle_error("Lval can't parse a unknown dims");
            }
        } else if (dims == 1) { // ident [ exp ]
            if (entry_ptr->data_type == DataType::INT) { // ident is an integer
                ret_type = DataType::INVALID;
                ret_var_name = "UNDECL" + std::to_string(undef_name_no_++);
            } else if ((entry_ptr->data_type == DataType::INT_ARR) && (entry_ptr->dims == 1)) {
                ret_type = DataType::INT;
                if (entry_ptr->symbol_type == SymbolType::CONST && is_integer(dim0_idx)) {
                    ret_var_name = std::to_string(entry_ptr->array_values[std::stod(dim0_idx)]);
                } else {
                    ret_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
                    local_addr_ += 4;
                    intermediate_.AddMidCode(ret_var_name, IntermOp::ARR_LOAD, entry_ptr->alias, dim0_idx);
                }
            } else {
                // ident is a 2d array
                ret_type = DataType::INT_ARR;
                ret_var_name = intermediate_.GenTmpArr(cur_func_name_, DataType::INT_ARR, cur_level_,
                                                       1, entry_ptr->dim1_size, 0, local_addr_);
                local_addr_ += 4;
                // add the dim0_idx to this arr
                std::string offset = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
                local_addr_ += 4;
//                int row_size = entry_ptr->dim1_size*4;
                intermediate_.AddMidCode(offset, IntermOp::MUL, dim0_idx, 4 * entry_ptr->dim1_size);
                intermediate_.AddMidCode(ret_var_name, IntermOp::ADD, entry_ptr->alias, offset);
            }
        } else if (dims == 2) { // identifier [exp] [exp]
            if ((entry_ptr->data_type == DataType::INT_ARR) && (entry_ptr->dims == 2)) {
                if (entry_ptr->symbol_type == SymbolType::CONST && is_integer(dim0_idx) && is_integer(dim1_idx)) {
                    ret_type = DataType::INT;
                    int index = entry_ptr->dim1_size * std::stoi(dim0_idx) + std::stoi(dim1_idx);
                    ret_var_name = std::to_string(entry_ptr->array_values[index]);
                } else {
                    ret_type = DataType::INT;
                    ret_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
                    local_addr_ += 4;
                    // arr[m][n]
                    // index = m * dim1_size + n
                    // ret_var = arr[index]
                    std::string index = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
                    local_addr_ += 4;
                    intermediate_.AddMidCode(index, IntermOp::MUL, dim0_idx, entry_ptr->dim1_size);
                    intermediate_.AddMidCode(index, IntermOp::ADD, index, dim1_idx);
                    intermediate_.AddMidCode(ret_var_name, IntermOp::ARR_LOAD, entry_ptr->alias, index);
                }
            } else {
                ret_type = DataType::INVALID;
                ret_var_name = "UNDECL" + std::to_string(undef_name_no_++);
            }
        } else {
            handle_error("parse error in LVal");
        }
    }
    output("<LVal>");
    return std::make_pair(ret_type, ret_var_name);
}

// Number -> IntConst
int Parser::Number() {
    int ret = IntConst();
    output("<Number>");
    return ret;
}

// integer-const -> decimal-const | 0
// decimal-const -> nonzero-digit { digit }
// nonzero-digit -> [1-9]
int Parser::IntConst() {
    int ret = std::stoi(token_.get_str_value());
    return ret;
}

// CallFunc -> Ident '(' [FuncRParams] ')'
// @pre: already read an Identifier
std::pair<DataType, std::string> Parser::CallFunc() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name; // init value is ""
    std::string called_func_name = token_.get_str_value();
    int func_name_line = token_.get_line_no();
    std::pair<DataType, std::string> inner_exp_ret;
    int need_param_num = 0;
    int provide_param_num = 0;

    next_sym(); // now at '('
    // check if is recursive function

    if (cur_func_name_ == called_func_name) symbol_table_.SetRecurFunc(called_func_name);

    auto search_res = symbol_table_.SearchFunc(called_func_name);
    if (search_res.first) {
        need_param_num = search_res.second->value;
        ret_type = search_res.second->data_type;
        next_sym();
        if (type_code_ == TypeCode::RPARENT) { // no params
            intermediate_.AddMidCode(called_func_name, IntermOp::PREPARE_CALL, "", ""); // prepare call
            if (need_param_num != 0) add_error(func_name_line, ErrorType::ARG_NO_MISMATCH);
        } else { // call has params function
            // (
            // ( P
            // ( P )
            // already read a token
            if (first_exp.count(type_code_) != 0) {
                std::vector<std::pair<DataType, std::string>> param_list = FuncRParams();
                intermediate_.AddMidCode(called_func_name, IntermOp::PREPARE_CALL, "", ""); // prepare call
                provide_param_num = param_list.size();
                bool has_arg_type_error = false;
                for (int i = 0; i < need_param_num && i < provide_param_num; i++) {
                    TableEntry *need_param_ptr = symbol_table_.GetKthParam(called_func_name, i);
                    std::string param_name = param_list[i].second;
                    TableEntry *prvd_param_ptr = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_,
                                                                                          param_name).second;
                    // todo: u can delete this type check in code generate, it's such a mess
                    if (param_list[i].first != symbol_table_.GetKthParam(called_func_name, i)->data_type) {
                        has_arg_type_error = true;
                    } else {
                        if (need_param_ptr->data_type == DataType::INT_ARR) {
                            int need_dims = need_param_ptr->dims;
                            int provide_dims = prvd_param_ptr->dims;
                            if (need_dims != provide_dims) has_arg_type_error = true;
                            intermediate_.AddMidCode(prvd_param_ptr->alias,
                                                     IntermOp::PUSH_ARR, std::to_string(i), "");
                        } else {
                            intermediate_.AddMidCode(param_name,
                                                     IntermOp::PUSH_VAL, std::to_string(i), "");
                        }
                    }
                }
                if (need_param_num != provide_param_num) add_error(func_name_line, ErrorType::ARG_NO_MISMATCH);
                if (has_arg_type_error) add_error(func_name_line, ErrorType::ARG_TYPE_MISMATCH);
                next_sym();
                if (type_code_ != TypeCode::RPARENT) {
                    retract();
                    add_error(ErrorType::EXPECTED_PARENT);
                }
            } else {
                // read a token not in FIRST of <exp>
                retract();
                add_error(ErrorType::EXPECTED_PARENT);
            }
        }
        intermediate_.AddMidCode(called_func_name, IntermOp::CALL, "", "");
        if (ret_type != DataType::VOID) {
            ret_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
            local_addr_ += 4;
            intermediate_.AddMidCode(ret_var_name, IntermOp::ADD, "%RET", 0);
        }
    } else {
        add_error(ErrorType::UNDECL);
        // if the function is undeclared, ignore this line
        // we promise there is a ')', because there is at most one error each line
        while (type_code_ != TypeCode::RPARENT) {
            next_sym();
        }
    }
    return std::make_pair(ret_type, ret_var_name);
}

// FuncRParams -> Exp { ',' Exp }
// @brief: the function is called when calling a function,
//         parse the exp_ret_values back, DO NOT CHECK arg type and number
// promise: already read a token, at least one param
// @exception:
std::vector<std::pair<DataType, std::string>> Parser::FuncRParams() {
    std::vector<std::string> ret_vars;
    DataType need_param_type;
    int provide_param_no = 0;
    DataType provide_param_type = DataType::INVALID;
    std::vector<std::pair<DataType, std::string>> exp_rets;
    std::pair<DataType, std::string> inner_exp_ret;

    inner_exp_ret = Exp();
    exp_rets.push_back(inner_exp_ret);
    next_sym();
    while (type_code_ == TypeCode::COMMA) {
        next_sym();
        inner_exp_ret = Exp();
        exp_rets.push_back(inner_exp_ret);
        next_sym();
    }

    // parse all the expressions
    provide_param_no = exp_rets.size();
    retract();
    output("<FuncRParams>");
    return exp_rets;
}

// UnaryOp -> '+' | '-' | '!'
// @pre: already read a token
// @retval: 0 '+', 1 '-', 2 '!'
int Parser::UnaryOp() {
    int op = -1;
    if (type_code_ == TypeCode::PLUS ||
        type_code_ == TypeCode::MINU ||
        type_code_ == TypeCode::NOT) {
        if (type_code_ == TypeCode::PLUS) {
            op = 0;
        } else if (type_code_ == TypeCode::MINU) {
            op = 1;
        } else {
            op = 2;
        }
    } else {
        handle_error("expect + - ! in <UnaryOp>");
    }
    output("<UnaryOp>");
    return op;
}

// ConstInitVal -> '{' [ ConstInitVal { ',' ConstInitVal } ] '}' |
//                  ConstExp
// @pre: 1. already read a token
//          2. ConstExp can be parsed as an integer
// if is an array, return a str(vector<int>)
// if is an integer, return a str(int)
std::pair<DataType, std::string> Parser::ConstInitVal() {
    DataType ret_type;
    std::string ret_value;
    bool is_array = false;
    std::vector<int> elements;
    std::pair<DataType, std::string> inner_exp_ret;

    if (type_code_ == TypeCode::LBRACE) {
        is_array = true;
        next_sym();
        if (type_code_ == TypeCode::RBRACE) {
            // in this case, e.g. array[0] = {};
        } else {
            inner_exp_ret = ConstInitVal();
            if (inner_exp_ret.first == DataType::INT) {
                elements.push_back(std::stoi(inner_exp_ret.second));
            } else if (inner_exp_ret.first == DataType::INT_ARR) {
                std::vector<int> inner_vec = str_to_vec_int(inner_exp_ret.second);
                elements.insert(elements.end(), inner_vec.begin(), inner_vec.end());
            } else {
                handle_error("ConstInitVal return not INT not INT_ARR");
            }
            next_sym();
            while (type_code_ == TypeCode::COMMA) {
                next_sym();
                inner_exp_ret = ConstInitVal();
                if (inner_exp_ret.first == DataType::INT) {
                    elements.push_back(std::stoi(inner_exp_ret.second));
                } else if (inner_exp_ret.first == DataType::INT_ARR) {
                    std::vector<int> inner_vec = str_to_vec_int(inner_exp_ret.second);
                    elements.insert(elements.end(), inner_vec.begin(), inner_vec.end());
                } else {
                    handle_error("ConstInitVal return not INT not INT_ARR");
                }
                next_sym();
            }
            if (type_code_ == TypeCode::RBRACE) {
                // end
            } else {
                handle_error("expect '}' at end of <ConstInitVal>");
            }
        }
    } else {
        inner_exp_ret = ConstExp();
        ret_type = inner_exp_ret.first;
        ret_value = inner_exp_ret.second;
    }
    output("<ConstInitVal>");
    if (is_array) {
        return std::make_pair(DataType::INT_ARR, vec_int_to_str(elements));
    } else {
        return std::make_pair(DataType::INT, ret_value);
    }
}

// VarDecl -> BType VarDef { ',' VarDef } ';'
// promise: already read a token
void Parser::VarDecl() {
    if (type_code_ == TypeCode::INTTK) {
        next_sym();
        VarDef();
        next_sym();
        while (type_code_ == TypeCode::COMMA) {
            next_sym();
            VarDef();
            next_sym();
        }
        if (type_code_ == TypeCode::SEMICN) {
            output("<VarDecl>");
        } else {
            retract();
            add_error(ErrorType::EXPECTED_SEMICN);
        }
    } else {
        handle_error("expect int in VarDecl begin");
    }
}

// VarDef -> Ident { '[' ConstExp ']' } |
//           Ident { '[' ConstExp ']' } '=' InitVal
// promise: already read a token
void Parser::VarDef() {
    reset_sym();
    name_ = token_.get_str_value();
    alias_ = name_ + "_" + std::to_string(token_.get_line_no());
    bool is_array = false;
    std::pair<DataType, std::string> inner_exp_ret;
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        is_array = true;
        dims_ += 1;
        next_sym();
        inner_exp_ret = ConstExp();
        dim0_size_ = std::stoi(inner_exp_ret.second);
        next_sym();
        if (type_code_ != TypeCode::RBRACK) {
            retract();
            add_error(ErrorType::EXPECTED_BRACK);
        }
    } else {
        retract(); // should go to read '='
    }
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        dims_ += 1;
        next_sym();
        inner_exp_ret = ConstExp();
        dim1_size_ = std::stoi(inner_exp_ret.second);
        next_sym();
        if (type_code_ == TypeCode::RBRACK) {

        } else {
            retract();
            add_error(ErrorType::EXPECTED_BRACK);
        }
    } else {
        retract();
    }

    DataType data_type = is_array ? DataType::INT_ARR : DataType::INT;
    bool add_success = symbol_table_.AddSymbol(cur_func_name_, data_type, SymbolType::VAR,
                                               name_, alias_,
                                               0, cur_level_, dims_, dim0_size_, dim1_size_, local_addr_);
    local_addr_ += 4;
    if (is_array) {
        intermediate_.AddMidCode(alias_, IntermOp::INIT_ARR_PTR, "", "");
        if (dims_ == 1) {
            local_addr_ += (dim0_size_ * 4);
        } else {
            local_addr_ += (dim0_size_ * dim1_size_ * 4);
        }
    }
    if (!add_success) add_error(ErrorType::REDEF);
    next_sym();
    if (type_code_ == TypeCode::ASSIGN) { // ['=' InitVal]
        next_sym();
        inner_exp_ret = InitVal();
        if (inner_exp_ret.first == DataType::INT) {
            intermediate_.AddMidCode(alias_, IntermOp::ADD, inner_exp_ret.second, 0);
        } else {
            std::vector<std::string> vec_str = str_to_vec_str(inner_exp_ret.second);
            for (int i = 0; i < vec_str.size(); i++) {
                intermediate_.AddMidCode(alias_, IntermOp::ARR_SAVE, i, vec_str[i]);
            }
        }
    } else {
        // done
        retract();
    }
    output("<VarDef>");
}

// InitVal -> '{' [ InitVal { ',' InitVal } ] '}'
//             Exp |
// promise: already read a token
// @retval pair.string: a tmp_var_name or list of tmp_var_name in str
std::pair<DataType, std::string> Parser::InitVal() {
    DataType ret_type;
    std::string ret_var_name;
    std::vector<std::string> ret_var_vec;
    bool is_array = false;
    std::pair<DataType, std::string> inner_exp_ret;

    if (type_code_ == TypeCode::LBRACE) {
        ret_type = DataType::INT_ARR;
        is_array = true;
        next_sym();
        if (type_code_ == TypeCode::RBRACE) {
            // end
        } else {
            inner_exp_ret = InitVal();
            if (inner_exp_ret.first == DataType::INT_ARR) {
                std::vector<std::string> inner_vec = str_to_vec_str(inner_exp_ret.second);
                ret_var_vec.insert(ret_var_vec.end(), inner_vec.begin(), inner_vec.end());
            } else if (inner_exp_ret.first == DataType::INT) {
                ret_var_vec.push_back(inner_exp_ret.second);
            } else {
                handle_error("Not INT or INT_ARR type return from InitVal");
            }
            next_sym();
            while (type_code_ == TypeCode::COMMA) {
                next_sym();
                inner_exp_ret = InitVal();
                if (inner_exp_ret.first == DataType::INT_ARR) {
                    std::vector<std::string> inner_vec = str_to_vec_str(inner_exp_ret.second);
                    ret_var_vec.insert(ret_var_vec.end(), inner_vec.begin(), inner_vec.end());
                } else if (inner_exp_ret.first == DataType::INT) {
                    ret_var_vec.push_back(inner_exp_ret.second);
                } else {
                    handle_error("Not INT or INT_ARR type return from InitVal");
                }
                next_sym();
            }
            // get '}' then end
            if (type_code_ == TypeCode::RBRACE) {
                // end
            } else {
                handle_error("expect '}' at end of <InitVal>");
            }
        }
    } else {
        inner_exp_ret = Exp();
        ret_type = inner_exp_ret.first;
        ret_var_name = inner_exp_ret.second;
    }
    output("<InitVal>");
    if (is_array) {
        return std::make_pair(ret_type, vec_str_to_str(ret_var_vec));
    } else {
        return std::make_pair(ret_type, ret_var_name);
    }
}

// FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
// @brief: the function is called when defining a function
// @pre: already read a token
// @pre: has_ret_stmt is set to false
void Parser::FuncDef() {
    DataType func_type = DataType::INVALID;
    int param_no = 0;
    int func_name_line_no;
    std::string func_name;

    func_type = FuncType();
    next_sym();
    func_name = cur_func_name_ = token_.get_str_value();
    func_name_line_no = token_.get_line_no();
    intermediate_.AddMidCode(cur_func_name_, IntermOp::FUNC_BEGIN, "", "");

    next_sym(); // '('
    if (type_code_ == TypeCode::LPARENT) {
        next_sym(); // ')' or 'int'
        cur_level_ += 1;
        if (!symbol_table_.AddFunc(func_type, cur_func_name_, 0)) {
            add_error(func_name_line_no, ErrorType::REDEF);
            // though it is redefined, we need to give a different name in the symbol table
            std::string redef_name = "redef_" + cur_func_name_ + std::to_string(redef_func_no_);
            redef_func_no_ += 1;
            cur_func_name_ = redef_name;
            symbol_table_.AddFunc(func_type, cur_func_name_, 0);
        }

        if (type_code_ == TypeCode::RPARENT) {
            // go to Block
        } else if (type_code_ == TypeCode::INTTK) {
            param_no = FuncFParams();
            next_sym();
            if (type_code_ == TypeCode::RPARENT) {
                // go to Block
            } else {
                retract();
                add_error(ErrorType::EXPECTED_PARENT);
            }
        } else {
            retract();
            add_error(ErrorType::EXPECTED_PARENT);
        }
        symbol_table_.SearchFunc(cur_func_name_).second->value = param_no;
        next_sym();
        std::vector<BlockItemType> item_types = Block();
        symbol_table_.PopLevel(cur_func_name_, cur_level_);
        cur_func_name_ = "";
        cur_level_ -= 1;
        if (func_type == DataType::VOID) {
            // has return statement or not does not matter
        } else {
            if (!has_ret_stmt_) {
                add_error(ErrorType::MISSING_RET);
            } else {
                // though the function has return statement
                // we need to make sure it has one return statement at the end of Block
                if (item_types.back() != BlockItemType::RETURN_STMT) {
                    add_error(ErrorType::MISSING_RET);
                }
            }
        }
        has_ret_stmt_ = false; // init the variable again
    } else {
        handle_error("expect '(' in FuncDef");
    }
    intermediate_.AddMidCode(func_name, IntermOp::FUNC_END, "", "");
    output("<FuncDef>");
}

// FuncType -> 'int' | 'void'
DataType Parser::FuncType() {
    DataType ret_data_type = DataType::INVALID;
    if (type_code_ == TypeCode::VOIDTK) {
        ret_data_type = DataType::VOID;
    } else if (type_code_ == TypeCode::INTTK) {
        ret_data_type = DataType::INT;
    } else {
        handle_error("FuncType must be int of void");
    }
    output("<FuncType>");
    return ret_data_type;
}

// FuncFParams -> FuncFParam {',' FuncFParam}
// @pre: FuncFParam must begin with "int", already read a token
// @retval: the number of the parameters
int Parser::FuncFParams() {
    int param_no = 0;
    if (type_code_ == TypeCode::INTTK) {
        FuncFParam(param_no);
        param_no += 1;
        next_sym();
        while (type_code_ == TypeCode::COMMA) {
            next_sym();
            FuncFParam(param_no);
            param_no += 1;
            next_sym();
        }
    } else {
        handle_error("expect 'int' in <FuncFParams> begin");
    }
    retract();
    output("<FuncFParams>");
    return param_no;
}

// FuncFParam -> BType Ident [ '[' ']' { '[' ConstExp ']' }]
void Parser::FuncFParam(int param_ord) {
    reset_sym();
    DataType param_type = DataType::INT;

    next_sym();
    if (type_code_ == TypeCode::IDENFR) {
        name_ = token_.get_str_value();
        alias_ = name_ + "_" + std::to_string(token_.get_line_no());
        next_sym();
        if (type_code_ == TypeCode::LBRACK) {
            dims_ = 1;
            param_type = DataType::INT_ARR;
            next_sym(); // eat ']'
            if (type_code_ != TypeCode::RBRACK) {
                retract();
                add_error(ErrorType::EXPECTED_BRACK);
            }
            next_sym();
            if (type_code_ == TypeCode::LBRACK) {
                dims_ += 1;
                next_sym();
                auto inner_exp_ret = ConstExp();
                dim1_size_ = std::stoi(inner_exp_ret.second);
                next_sym();
                if (type_code_ == TypeCode::RBRACK) {

                } else {
                    retract();
                    add_error(ErrorType::EXPECTED_BRACK);
                }
            } else {
                retract(); // only one dimension
            }
        } else {
            retract(); // read a token not '[', only an identifier
        }
        if (dims_ != 0) param_type = DataType::INT_ARR;
        // u have to fill the dim1_size, because it will be used in assign or fetch
        // the dim0 size is 0 now
        if (!symbol_table_.AddSymbol(cur_func_name_, param_type, SymbolType::PARAM,
                                     name_, alias_,
                                     param_ord, cur_level_, dims_, dim0_size_, dim1_size_, local_addr_)) {
            add_error(ErrorType::REDEF);
        }
        local_addr_ += 4;
        output("<FuncFParam>");
        return;
    } else {
        handle_error("expect Ident in <FuncFParam>");
    }
}

// Block -> '{' { BlockItem } '}'
// @pre: the level has been self-added
std::vector<BlockItemType> Parser::Block() {
    std::vector<BlockItemType> stmt_types;
    BlockItemType inner_stmt_type = BlockItemType::INVALID;
    if (type_code_ == TypeCode::LBRACE) {
        next_sym();
        // read a token from back to begin,
        // so we don't need to know the FIRSTof(BlockItem)
        while (type_code_ != TypeCode::RBRACE) {
            inner_stmt_type = BlockItem(); // already read in
            stmt_types.emplace_back(inner_stmt_type);
            next_sym();
        }
        // already read '}', so end
    } else {
        handle_error("expected '{' at the begin of Block");
    }
    output("<Block>");
    return stmt_types;
}

// BlockItem -> Decl | Stmt
BlockItemType Parser::BlockItem() {
    BlockItemType item_type = BlockItemType::INVALID;
    if (type_code_ == TypeCode::CONSTTK || type_code_ == TypeCode::INTTK) {
        item_type = Decl();
    } else {
        // in usual, we should judge
        item_type = Stmt();
    }
    return item_type;
}

// Stmt -> LVal '=' Exp ';'
//    	|  [Exp] ';'
//         Block |
//         'if' '(' Cond ')' Stmt [ 'else' Stmt ] |
//         'while' '(' Cond ')' Stmt |
//         'break' ';' | 'continue' ';' |
//         'return' [Exp] ';' |
//         LVal '=' 'getint''('')'';' |
//         'printf''('FormatString{,Exp}')'';' // 1.with Exp 2.without Exp
BlockItemType Parser::Stmt() {
    BlockItemType item_type = BlockItemType::INVALID;
    if (type_code_ == TypeCode::LBRACE) {
        cur_level_ += 1;
        Block();
        symbol_table_.PopLevel(cur_func_name_, cur_level_);
        cur_level_ -= 1;
        item_type = BlockItemType::BLOCK_STMT;
    } else if (type_code_ == TypeCode::IFTK) {
        IfStmt();
        item_type = BlockItemType::IF_STMT;
    } else if (type_code_ == TypeCode::WHILETK) {
        loop_stack_.push_back(true);
        WhileStmt();
        item_type = BlockItemType::WHILE_STMT;
        loop_stack_.pop_back();
    } else if (type_code_ == TypeCode::BREAKTK || type_code_ == TypeCode::CONTINUETK) {
        auto it = loop_stack_.end() - 1;
        if (type_code_ == TypeCode::BREAKTK) {
            std::string while_end_label = *(while_labels.end() - 1);
            intermediate_.AddMidCode(while_end_label, IntermOp::JUMP, "", "");
        } else {
            std::string while_begin_label = *(while_labels.end() - 2);
            intermediate_.AddMidCode(while_begin_label, IntermOp::JUMP, "", "");
        }
        if (*it) {
            // yes, it is in loop
            item_type = (type_code_ == TypeCode::BREAKTK) ? BlockItemType::BREAK_STMT : BlockItemType::CONTINUE_STMT;
        } else {
            add_error(ErrorType::NOT_IN_LOOP);
        }
        next_sym();
        if (type_code_ == TypeCode::SEMICN) {
            // end
        } else {
            retract();
            add_error(ErrorType::EXPECTED_SEMICN);
        }
    } else if (type_code_ == TypeCode::RETURNTK) {
        ReturnStmt();
        item_type = BlockItemType::RETURN_STMT;
    } else if (type_code_ == TypeCode::PRINTFTK) {
        WriteStmt();
        item_type = BlockItemType::WRITE_STMT;
    } else if (type_code_ == TypeCode::SEMICN) {
        // empty stmt
        item_type = BlockItemType::EMPTY_STMT;
        // end
    } else {
        // assign stmt, exp stmt, read stmt
        if (type_code_ == TypeCode::IDENFR) {
            next_sym();
            if (type_code_ == TypeCode::LPARENT) {
                retract();
                Exp(); // function call
                next_sym();
                if (type_code_ == TypeCode::SEMICN) {
                    //
                } else {
                    retract();
                    add_error(ErrorType::EXPECTED_SEMICN);
                }
            } else {
                // lval;
                // lval = getint();
                // lval = Exp;
                retract(); // back to identifier
                int assigned_line_no = token_.get_line_no();
                std::string assigned_var_name = token_.get_str_value();
                std::pair<bool, TableEntry *> search_res =
                        symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, assigned_var_name);
                if (!search_res.first) handle_error("cant find a assigned name in symbol table");
                // judge it's an array or variable
                std::pair<std::string, std::string> assigned_lval_ret = AssignedLval();
                if (assigned_lval_ret.first.empty()) handle_error("can't find this assigned lval in symbol table");
                next_sym();
                if (type_code_ == TypeCode::ASSIGN) {
                    next_sym();
                    if (type_code_ == TypeCode::GETINTTK) {
                        ReadStmt(assigned_lval_ret);
                    } else {
                        retract(); // back to '='
                        AssignStmt(assigned_line_no, assigned_lval_ret);
                    }
                } else {
                    if (type_code_ == TypeCode::SEMICN) {

                    } else {
                        retract();
                        add_error(ErrorType::EXPECTED_SEMICN);
                    }
                }
            }
        } else if (first_exp.count(type_code_) != 0) {
            Exp();
            item_type = BlockItemType::EXP_STMT;
            next_sym();
            if (type_code_ == TypeCode::SEMICN) {
                // end
            } else {
                retract();
                add_error(ErrorType::EXPECTED_SEMICN);
            }
        } else {
            handle_error("can't find branch in Stmt");
        }
    }
    output("<Stmt>");
    return item_type;
}


// @brief: called when assign a value to a left value,
//         need to return the name and index of the LVal
// @retval: pair<name, index>, if index is empty, means it is NOT array
// @exec: the name may be not found
// @attention: use the alias of the variable
std::pair<std::string, std::string> Parser::AssignedLval() {
    std::string ident;
    std::string ret_var_name;
    std::string ret_idx;
    int ident_line_no;
    bool fetch_array = false;
    int dims = 0;
    std::string dim0_idx, dim1_idx;
    std::pair<DataType, std::string> inner_exp_ret;

    ident = token_.get_str_value();
    ident_line_no = token_.get_line_no();
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        fetch_array = true;
        dims += 1;
        next_sym();
        inner_exp_ret = Exp();
        dim0_idx = inner_exp_ret.second;
        next_sym(); // read ']'
        if (type_code_ != TypeCode::RBRACK) {
            retract();
            add_error(ErrorType::EXPECTED_BRACK);
        }
    } else {
        retract();
    }

    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        fetch_array = true;
        next_sym();
        dims += 1;
        inner_exp_ret = Exp();
        dim1_idx = inner_exp_ret.second;
        next_sym(); // read ']'
        if (type_code_ != TypeCode::RBRACK) {
            retract();
            add_error(ErrorType::EXPECTED_BRACK);
        }
    } else {
        retract();
    }

    std::pair<bool, TableEntry *> search_res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, ident);
    TableEntry res_ptr = TableEntry(search_res.second);
    TableEntry *entry_ptr = &res_ptr;
    if (!search_res.first) {
        add_error(ErrorType::UNDECL);
    } else {
        if (search_res.second->symbol_type == SymbolType::CONST) add_error(ident_line_no, ErrorType::CHANGE_CONST);

        if (dims == 0) { // read "ident"
            if (entry_ptr->dims == 0) {
                ret_var_name = entry_ptr->alias;
            } else {
                handle_error("AssignedLval can't parse a unknown dims");
            }
        } else if (dims == 1) { // ident [ exp ]
            if (entry_ptr->data_type == DataType::INT) { // ident is an integer
                handle_error("a variable is called with [], but it is an value");
            } else if ((entry_ptr->data_type == DataType::INT_ARR) && (entry_ptr->dims == 1)) {
                ret_var_name = search_res.second->alias;
                ret_idx = dim0_idx;
            } else { // ident is a 2d array
                handle_error("1 dimension array is called with [][]");
            }
        } else if (dims == 2) { // identifier [exp] [exp]
            if ((entry_ptr->data_type == DataType::INT_ARR) && (entry_ptr->dims == 2)) {
                ret_var_name = search_res.second->alias;
                // arr[m][n]
                // index = m * dim1_size
                // index = index + dim1_idx
                std::string index = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
                local_addr_ += 4;
                intermediate_.AddMidCode(index, IntermOp::MUL, dim0_idx, entry_ptr->dim1_size);
                intermediate_.AddMidCode(index, IntermOp::ADD, index, dim1_idx);
                ret_idx = index;
            } else {
                handle_error("parse error in AssignedLval()");
            }
        } else {
            handle_error("parse error in LVal");
        }
    }
    output("<LVal>");
    return std::make_pair(ret_var_name, ret_idx);
}

// AssignStmt -> AssignedLVal '=' Exp ';'
// @pre: already read '='
void Parser::AssignStmt(int assigned_line_no, const std::pair<std::string, std::string> &assigned_lval_ret) {
    next_sym();
    std::pair<DataType, std::string> exp_ret = Exp();
    next_sym();
    if (assigned_lval_ret.second.empty()) {
        intermediate_.AddMidCode(assigned_lval_ret.first, IntermOp::ADD, exp_ret.second, 0);
    } else {
        intermediate_.AddMidCode(assigned_lval_ret.first, IntermOp::ARR_SAVE, assigned_lval_ret.second, exp_ret.second);
    }

    if (type_code_ != TypeCode::SEMICN) {
        retract();
        add_error(ErrorType::EXPECTED_SEMICN);
    }
}

// IfStmt -> 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
// @pre: already read a if_token
void Parser::IfStmt() {
    next_sym();
    next_sym();
    auto cond_ret = Cond();
    if (cond_ret.first != DataType::INT) handle_error("expected ret_type INT from Cond");

    next_sym();
    if (type_code_ == TypeCode::RPARENT) {
        next_sym();
        std::string else_label = intermediate_.GenLabel();
        intermediate_.AddMidCode(else_label, IntermOp::BEQ, cond_ret.second, 0);
        Stmt(); // if-block
        next_sym();
        if (type_code_ == TypeCode::ELSETK) {
            // GenLabel if_end_label
            // AddMidCode: Jump if_end_label
            // AddMidCode: else_block_label:
            std::string if_end_label = intermediate_.GenLabel();
            intermediate_.AddMidCode(if_end_label, IntermOp::JUMP, "", "");
            intermediate_.AddMidCode(else_label, IntermOp::LABEL, "", "");
            next_sym();
            Stmt();
            // AddMidCode: if_end_label:
            intermediate_.AddMidCode(if_end_label, IntermOp::LABEL, "", "");
        } else {
            retract();
            // AddMidCode: else_label:
            intermediate_.AddMidCode(else_label, IntermOp::LABEL, "", "");
        }
    } else {
        retract();
        add_error(ErrorType::EXPECTED_PARENT);
    }

}

// Cond -> LOrExp
std::pair<DataType, std::string> Parser::Cond() {
    auto ret = LOrExp();
    output("<Cond>");
    return ret;
}

// LOrExp -> LAndExp | LOrExp '||' LAndExp
// LOrExp -> LAndExp { '||' LAndExp }
// @pre: already read a token
std::pair<DataType, std::string> Parser::LOrExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name;
    bool cur_be_parsed_int = false;
    std::pair<DataType, std::string> inner_exp_ret;
    std::string cond_end_label = intermediate_.GenCondEndLabel();

    ret_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
    local_addr_ += 4;

    inner_exp_ret = LAndExp();
    ret_type = inner_exp_ret.first;
    intermediate_.AddMidCode(ret_var_name, IntermOp::ADD, inner_exp_ret.second, 0);

    next_sym();
    while (type_code_ == TypeCode::OR) {
        // erase then read
        retract();
        output("<LOrExp>");
        next_sym();

        next_sym();
        intermediate_.AddMidCode(cond_end_label, IntermOp::BNE, ret_var_name, 0); // not equal 0, branch to cond end
        inner_exp_ret = LAndExp();
        intermediate_.AddMidCode(ret_var_name, IntermOp::ADD, inner_exp_ret.second, 0);
        next_sym();
    }
    intermediate_.AddMidCode(cond_end_label, IntermOp::LABEL, "", "");
    retract();
    output("<LOrExp>");
    return std::make_pair(ret_type, ret_var_name);
}

// LAndExp -> EqExp | LAndExp '&&' EqExp
// LAndExp -> EqExp { '&&' EqExp }
// @attention: left recurrence
// @pre: already read a token
std::pair<DataType, std::string> Parser::LAndExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name;
    std::pair<DataType, std::string> inner_exp_ret;

    std::string land_end_label = intermediate_.GenLAndEndLabel();
    ret_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
    local_addr_ += 4;

    inner_exp_ret = EqExp();
    ret_type = inner_exp_ret.first;
    intermediate_.AddMidCode(ret_var_name, IntermOp::ADD, inner_exp_ret.second, 0);

    next_sym();
    while (type_code_ == TypeCode::AND) {
        // erase then read
        retract();
        output("<LAndExp>");
        next_sym();

        next_sym();
        intermediate_.AddMidCode(land_end_label, IntermOp::BEQ, ret_var_name, 0);
        inner_exp_ret = EqExp();
        intermediate_.AddMidCode(ret_var_name, IntermOp::ADD, inner_exp_ret.second, 0);
        next_sym();
    }
    intermediate_.AddMidCode(land_end_label, IntermOp::LABEL, "", "");
    retract();
    output("<LAndExp>");
    return std::make_pair(ret_type, ret_var_name);
}

// EqExp -> RelExp { ('==' | '!=') RelExp}
// @attention: left recurrence
// @pre: already read a token
std::pair<DataType, std::string> Parser::EqExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name;
    bool cur_be_parsed_int = false;

    auto inner_exp_ret = RelExp();
    ret_type = inner_exp_ret.first;
    ret_var_name = inner_exp_ret.second;
    if (is_integer(ret_var_name)) {
        cur_be_parsed_int = true;
    } else {
        cur_be_parsed_int = false;
    }

    next_sym();
    while (type_code_ == TypeCode::EQL || type_code_ == TypeCode::NEQ) {
        // erase then read
        retract();
        output("<EqExp>");
        next_sym();

        int sign = (type_code_ == TypeCode::EQL) ? 0 : 1;
        next_sym();
        inner_exp_ret = RelExp();
        if (cur_be_parsed_int && is_integer(inner_exp_ret.second)) {
            int parsed_inner_value = std::stoi(inner_exp_ret.second);
            int cur_parsed_value = std::stoi(ret_var_name);
            if (sign == 0) {
                ret_var_name = std::to_string((parsed_inner_value == cur_parsed_value));
            } else {
                ret_var_name = std::to_string((parsed_inner_value != cur_parsed_value));
            }
        } else {
            cur_be_parsed_int = false;
            std::string tmp_var_name =
                    intermediate_.GenTmpVar(cur_func_name_, ret_type, cur_level_, local_addr_);
            local_addr_ += 4;
            IntermOp op = (sign == 0) ? IntermOp::EQ : IntermOp::NEQ;
            intermediate_.AddMidCode(tmp_var_name, op, ret_var_name, inner_exp_ret.second);
            ret_var_name = tmp_var_name;
        }
        next_sym();
    }
    retract();
    output("<EqExp>");
    return std::make_pair(ret_type, ret_var_name);
}

// RelExp -> AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
// RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
// @attention: left recurrence
std::pair<DataType, std::string> Parser::RelExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name;
    bool cur_be_parsed_int = false;

    auto inner_exp_ret = AddExp();
    ret_type = inner_exp_ret.first;
    ret_var_name = inner_exp_ret.second;
    if (is_integer(ret_var_name)) {
        cur_be_parsed_int = true;
    } else {
        cur_be_parsed_int = false;
    }

    next_sym();
    while (type_code_ == TypeCode::LSS || type_code_ == TypeCode::LEQ ||
           type_code_ == TypeCode::GRE || type_code_ == TypeCode::GEQ) {
        TypeCode comp_op = type_code_;
        // erase the token
        retract();
        output("<RelExp>");
        next_sym();

        next_sym();
        inner_exp_ret = AddExp();
        if (cur_be_parsed_int && is_integer(inner_exp_ret.second)) {
            int parsed_inner_value = std::stoi(inner_exp_ret.second);
            int cur_parsed_value = std::stoi(ret_var_name);
            if (comp_op == TypeCode::LSS) {
                ret_var_name = std::to_string((cur_parsed_value < parsed_inner_value));
            } else if (comp_op == TypeCode::LEQ) {
                ret_var_name = std::to_string((cur_parsed_value <= parsed_inner_value));
            } else if (comp_op == TypeCode::GRE) {
                ret_var_name = std::to_string((cur_parsed_value > parsed_inner_value));
            } else {
                ret_var_name = std::to_string((cur_parsed_value >= parsed_inner_value));
            }
        } else {
            cur_be_parsed_int = false;
            IntermOp op;
            if (comp_op == TypeCode::LSS) {
                op = IntermOp::LSS;
            } else if (comp_op == TypeCode::LEQ) {
                op = IntermOp::LEQ;
            } else if (comp_op == TypeCode::GRE) {
                op = IntermOp::GRE;
            } else {
                op = IntermOp::GEQ;
            }
            std::string tmp_var_name =
                    intermediate_.GenTmpVar(cur_func_name_, ret_type, cur_level_, local_addr_);
            local_addr_ += 4;
            intermediate_.AddMidCode(tmp_var_name, op, ret_var_name, inner_exp_ret.second);
            ret_var_name = tmp_var_name;
        }
        next_sym();
    }
    retract();
    output("<RelExp>");
    return std::make_pair(ret_type, ret_var_name);
}

// WhileStmt -> 'while' '(' Cond ')' Stmt
void Parser::WhileStmt() {
    std::string label_while_begin = intermediate_.GenWhileBeginLabel();
    std::string label_while_end = intermediate_.GenWhileEndLabel();
    while_labels.push_back(label_while_begin);
    while_labels.push_back(label_while_end);
    intermediate_.AddMidCode(label_while_begin, IntermOp::LABEL, "", "");

    next_sym(); // eat '('
    next_sym();
    std::pair<DataType, std::string> cond_ret = Cond();
    if (cond_ret.first != DataType::INT) handle_error("datatype of cond in while need to be int");
    intermediate_.AddMidCode(label_while_end, IntermOp::BEQ, cond_ret.second, 0);
    next_sym();
    if (type_code_ != TypeCode::RPARENT) {
        retract();
        add_error(ErrorType::EXPECTED_PARENT);
    }
    next_sym();
    Stmt();
    while_labels.pop_back();
    while_labels.pop_back();
    intermediate_.AddMidCode(label_while_begin, IntermOp::JUMP, "", "");
    intermediate_.AddMidCode(label_while_end, IntermOp::LABEL, "", "");
}

// ReturnStmt -> 'return' [<Exp>] ';'
// @pre: return statement must in a function def
void Parser::ReturnStmt() {
    has_ret_stmt_ = true;
    int return_line_no = token_.get_line_no();
    DataType parsed_ret_type = DataType::VOID;
    next_sym();
    if (first_exp.count(type_code_) != 0) {
        auto exp_ret = Exp(); // already read a token
        parsed_ret_type = exp_ret.first;
        next_sym();
        if (parsed_ret_type == DataType::INT) {
            intermediate_.AddMidCode(exp_ret.second, IntermOp::RET, "", "");
        } else {
            intermediate_.AddMidCode("", IntermOp::RET, "", "");
        }
        if (type_code_ != TypeCode::SEMICN) {
            retract();
            add_error(ErrorType::EXPECTED_SEMICN);
        }
    } else if (type_code_ == TypeCode::SEMICN) {
        intermediate_.AddMidCode("", IntermOp::RET, "", "");
    } else {
        retract();
        add_error(ErrorType::EXPECTED_SEMICN);
    }
    DataType should_ret_type = symbol_table_.SearchFunc(cur_func_name_).second->data_type;
    if (should_ret_type == DataType::VOID && parsed_ret_type != DataType::VOID) {
        add_error(return_line_no, ErrorType::RET_TYPE_MISMATCH);
    }
}

// ReadStmt -> LVal '=' 'getint' '(' ')' ';'
// @pre: already read 'getint'
// param[in] assigned_var_name: the name be assigned
void Parser::ReadStmt(const std::pair<std::string, std::string> &assigned_lval_ret) {
    // todo: lval_ret is a string
    if (assigned_lval_ret.second.empty()) { // is an int variable
        intermediate_.AddMidCode(assigned_lval_ret.first, IntermOp::GETINT, "", "");
    } else {
        std::string tmp = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_, local_addr_);
        local_addr_ += 4;
        intermediate_.AddMidCode(tmp, IntermOp::GETINT, "", "");
        intermediate_.AddMidCode(assigned_lval_ret.first, IntermOp::ARR_SAVE, assigned_lval_ret.second, tmp);
    }
    next_sym(); // eat  (
    next_sym(); // eat )
    if (type_code_ != TypeCode::RPARENT) {
        retract();
        add_error(ErrorType::EXPECTED_PARENT);
    }
    next_sym();
    if (type_code_ != TypeCode::SEMICN) { // eat ;
        retract();
        add_error(ErrorType::EXPECTED_SEMICN);
    }
}

// WriteStmt-> 'printf' '(' FormatString {',' Exp } ')' ';'
// @pre: already read 'printf'
void Parser::WriteStmt() {
    int printf_line_no = token_.get_line_no();
    std::pair<int, std::vector<std::string>> fmt_str_ret;
    std::vector<std::string> vec_fmt_str;
    std::pair<DataType, std::string> exp_ret;
    std::vector<std::string> vec_exp_str;
    int format_no = 0;
    int exp_no = 0;

    next_sym();
    next_sym();  // this sym is STRCON, FormatString
    fmt_str_ret = FormatString();
    format_no = fmt_str_ret.first;
    vec_fmt_str = fmt_str_ret.second;
    for (auto &i: vec_fmt_str) {
        symbol_table_.add_to_strcons(i);
    }

    next_sym();
    while (type_code_ == TypeCode::COMMA) {
        next_sym();
        exp_ret = Exp();
        vec_exp_str.push_back(exp_ret.second);
        exp_no += 1;
        next_sym(); // will be ',' ?
    }

    if (format_no != exp_no) add_error(printf_line_no, ErrorType::PRINT_NO_MISMATCH);

    // add to mid-code
    int j = 0;
    for (auto &i: vec_fmt_str) {
        if (i == "%d") {
            intermediate_.AddMidCode(vec_exp_str[j], IntermOp::PRINT, "int", "");
            j += 1;
        } else if (i == "\\n") {
            intermediate_.AddMidCode("\\n", IntermOp::PRINT, "str", "");
        } else {
            intermediate_.AddMidCode(i, IntermOp::PRINT, "str", "");
        }
    }


    if (type_code_ != TypeCode::RPARENT) {
        retract();
        add_error(ErrorType::EXPECTED_PARENT);
    }
    next_sym();
    if (type_code_ != TypeCode::SEMICN) {
        retract();
        add_error(ErrorType::EXPECTED_SEMICN);
    }

}

// FormatString> → '"' { Char } '"'
// Char ->  FormatChar | NormalChar
// FormatChar -> '%' 'd'
// NormalChar -> ⼗进制编码为32,33,40-126的ASCII字符，'\'（编码92）出现当且仅当为'\n'
// @note: ascii('%') is 37, ascii('\') = 92
// @attention: there can't be two error in one line
//             if we can't parse the string, return <-1, str_vec>
// @retval: format %d number and the slice of the string
std::pair<int, std::vector<std::string>> Parser::FormatString() {
    bool has_error = false;
    int format_no = 0;
    std::vector<std::string> vec_str;
    std::string str_con = token_.get_str_value();
    unsigned long long len = str_con.length();
    if (len < 2) {
        add_error(ErrorType::ILLEGAL_CHAR);
        return std::make_pair(0, vec_str);
    }
    if (str_con[0] != '"' || str_con.back() != '"') {
        add_error(ErrorType::ILLEGAL_CHAR);
        return std::make_pair(0, vec_str);
    }

    int final_format_no = get_substr_no(str_con, "%d");

    for (int i = 0; i < str_con.length(); i++) {
        if (str_con[i] == '"') {
            if (i == 0 || i == str_con.length() - 1) {
                // pass
            } else {
                has_error = true;
                break;
            }
        } else if (str_con[i] == '%') {
            if (str_con[i + 1] != 'd') {
                has_error = true;
                break;
            }
        } else if (str_con[i] == '\\') {
            if (str_con[i + 1] != 'n') {
                has_error = true;
                break;
            }
        } else if (str_con[i] == 32 || str_con[i] == 33 || (40 <= str_con[i] && str_con[i] <= 126)) {
            // pass
        } else {
            has_error = true;
            break;
        }
    }
    if (has_error) {
        add_error(ErrorType::ILLEGAL_CHAR);
        return std::make_pair(final_format_no, vec_str);
    }

    std::string str_tmp;
    int i = 0;
    if (str_con[i] == '"') {
        i++;
        while (str_con[i] == '%' || // the first set of Char
               str_con[i] == 32 || str_con[i] == 33 || (40 <= str_con[i] && str_con[i] <= 126) || str_con[i] == '\\') {
            if (str_con[i] == '%') {
                if (str_tmp.empty()) {
                    // pass
                } else {
                    vec_str.push_back(str_tmp);
                    str_tmp.clear();
                }
                i += 1;
                if (str_con[i] == 'd') {
                    vec_str.emplace_back("%d");
                    format_no += 1;
                    i += 1;
                } // eaten %d
                else {
                    if (has_error) {
                        // pass
                    } else {
                        has_error = true;
                        add_error(ErrorType::ILLEGAL_CHAR);
                    }
                    // already point to the char behind %
                }
            } else {
                str_tmp += str_con[i];
                if (str_con[i] == '\\') {
                    i += 1;
                    if (str_con[i] != 'n') {
                        i -= 1;
                        if (has_error) {
                            // pass
                        } else {
                            has_error = true;
                            add_error(ErrorType::ILLEGAL_CHAR);
                        }
                    } else { // now at "n"
                        str_tmp.erase(str_tmp.end() - 1);
                        if (!str_tmp.empty()) {
                            vec_str.push_back(str_tmp);
                            str_tmp.clear();
                        }
                        vec_str.emplace_back("\\n");
                    }
                } else {

                }
                i += 1;
            }
        }
        if (str_con[i] == '"') {
            if (!str_tmp.empty()) {
                vec_str.push_back(str_tmp);
            }
            // end
        } else {
            if (has_error) {
                // pass
            } else {
                has_error = true;
                add_error(ErrorType::ILLEGAL_CHAR);
            }
        }
    } else {
        has_error = true;
        add_error(ErrorType::ILLEGAL_CHAR);
    }

    for (auto &fmt_str: vec_str) {
        if (fmt_str != "%d") {
            intermediate_.strcons.push_back(fmt_str);
        }
    }
    return std::make_pair(final_format_no, vec_str);
}

// MainFuncDef -> 'int' 'main' '(' ')' Block
void Parser::MainFuncDef() {
    next_sym(); // eat main
    cur_func_name_ = "main";
    symbol_table_.AddFunc(DataType::INT, "main", 0);
    intermediate_.AddMidCode("main", IntermOp::FUNC_BEGIN, "", "");
    cur_level_ += 1;
    next_sym(); // eat (
    next_sym(); // eat )
    if (type_code_ != TypeCode::RPARENT) {
        retract();
        add_error(ErrorType::EXPECTED_PARENT);
    }

    next_sym();
    std::vector<BlockItemType> item_types = Block();
    symbol_table_.PopLevel(cur_func_name_, cur_level_);
    cur_func_name_ = "";
    cur_level_ -= 1;
    if (has_ret_stmt_) {
        if (item_types.back() != BlockItemType::RETURN_STMT) {
            add_error(ErrorType::MISSING_RET);
        } else {
            // pass
        }
    } else {
        add_error(ErrorType::MISSING_RET);
    }
    intermediate_.AddMidCode("main", IntermOp::FUNC_END, "", "");
    has_ret_stmt_ = false;
    output("<MainFuncDef>");
}

// @brief: rest current symbol info
void Parser::reset_sym() {
    name_ = "";
    alias_ = "";
    dims_ = dim0_size_ = dim1_size_ = 0;
}
