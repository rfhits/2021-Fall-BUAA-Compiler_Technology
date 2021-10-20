//
// Created by WYSJ6174 on 2021/10/3.
//

#include "Parser.h"

Parser::Parser(SymbolTable& symbol_table, Lexer &lexer, ErrorHandler &error_handler, Intermediate& intermediate,
               bool print_mode,std::ofstream& out) :
        symbol_table_(symbol_table), lexer_(lexer), error_handler_(error_handler),
        intermediate_(intermediate), print_mode_(print_mode), out_(out) {}


// if pos is behind read_tokens
// then read from it
// else use get_token() to get a token from src
void Parser::next_sym() {

    // may retract
    if (pos_ < read_tokens_.size()) {
        token_ = read_tokens_[pos_];
    } else {
        token_ = lexer_.get_token();
        read_tokens_.push_back(token_);
    }
    type_code_ = token_.get_type_code();
    name_ = token_.get_str_value();
    pos_ += 1;
    out_strings_.push_back(token_.to_string());
}

// change pos and output_strings
void Parser::retract() {
    pos_ -= 1;
    token_ = read_tokens_[pos_ - 1];
    type_code_ = token_.get_type_code();

    // erase the output until meets <V_n>
    for (unsigned int i = out_strings_.size() - 1; i >= 0; i--) {
        if (out_strings_[i][0] != '<') {
            out_strings_.erase(out_strings_.begin() + i);
            break;
        }
    }
}


void Parser::output(const std::string& msg) {
    out_strings_.push_back(msg);
}


void Parser::handle_error(const std::string& msg) {
    error_handler_.log_error(token_.get_line_no(), msg);
}


void Parser::handle_error(ErrorType error_type) {
    if (error_type == ErrorType::A) {

    } else if (error_type == ErrorType::B) {

    }
}


// CompUnit -> {Decl} {FuncDef} MainFuncDef
void Parser::Program() {
    next_sym();
    // three cond: const / int / void
    // three branches: Decl, Func, Main
    while (type_code_ == TypeCode::CONSTTK ||
           type_code_ == TypeCode::INTTK) {
        if (type_code_ == TypeCode::CONSTTK) {
            Decl();
            next_sym();
        } else { // must be 'int' here
            next_sym(); // int a
            if (type_code_ == TypeCode::IDENFR) {
                next_sym();
                if (type_code_ == TypeCode::COMMA ||
                    type_code_ == TypeCode::SEMICN ||
                    type_code_ == TypeCode::ASSIGN ||
                    type_code_ == TypeCode::LBRACK )
                {
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
    while (type_code_ == TypeCode::VOIDTK ||
            type_code_ == TypeCode::INTTK)
    {
        if (type_code_ == TypeCode::VOIDTK) {
            FuncDef();
            next_sym();
        } else { // must be 'int'
            next_sym();
            if (type_code_ == TypeCode::IDENFR) {
                retract();
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
        MainFuncDef();
    } else {
        handle_error("expect 'int' head of MainFuncDef");
    }


    out_strings_.emplace_back("<CompUnit>");
    if (print_mode_) {
        auto it = out_strings_.begin();
        while (it != out_strings_.end()) {
            out_ << *it << std::endl;
            it += 1;
        }
    }
}

// Decl -> ConstDecl | VarDecl
// promise: already read a CONST or INT
void Parser::Decl() {
    if (type_code_ == TypeCode::CONSTTK) {
        ConstDecl();
    } else if (type_code_ == TypeCode::INTTK) {
        VarDecl();
    } else {
        handle_error("Decl expect a const or int");
    }
}

// ConstDecl -> const int ConstDef {, ConstDef}
// promise: already read a const
void Parser::ConstDecl() {
    next_sym();
    if (type_code_ == TypeCode::INTTK) {
        next_sym(); // before enter <ConstDef>, read a token
        ConstDef();
        next_sym();
        while (type_code_ == TypeCode::COMMA) {
            next_sym();
            ConstDef();
            next_sym();
        }
        // type_code is not ";", expected ";"
        if (type_code_ == TypeCode::SEMICN) {
            output("<ConstDecl>");
        } else {
            handle_error(ErrorType::EXPECTED_SEMICN);
        }
    } else {
        handle_error("Const expect a int");
    }
}

// ConstDef -> Ident {'[' ConstExp ']'} '=' ConstInitVal
// promise: already read an Identifier
void Parser::ConstDef() {
    reset_sym();
    bool is_array = false;
    name_ = token_.get_str_value();
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        is_array = true;
        dims_ += 1;
        next_sym();
        auto const_exp_ret = ConstExp();
        dim0_size_ = std::stoi(const_exp_ret.second);
        next_sym();
        if (type_code_ == TypeCode::RBRACK) {
            next_sym();
        } else {
            handle_error(ErrorType::EXPECTED_BRACK);
        }
    }
    if (type_code_ == TypeCode::LBRACK) {
        dims_ += 1;
        next_sym();
        auto const_exp_ret = ConstExp();
        dim1_size_ = std::stoi(const_exp_ret.second);
        next_sym();
        if (type_code_ == TypeCode::RBRACK) {
            next_sym();
        } else {
            handle_error(ErrorType::EXPECTED_BRACK);
        }
    }

    if (type_code_ == TypeCode::ASSIGN) {
        next_sym();
        auto const_init_val_ret = ConstInitVal();
        if (const_init_val_ret.first == DataType::INT) { // not array
            int parsed_int = std::stoi(const_init_val_ret.second);
            symbol_table_.AddSymbol("", DataType::INT, SymbolType::CONST,
                                    name_, parsed_int, 0, 0, 0, 0);
        } else if (const_init_val_ret.first == DataType::INT_ARR) {
            std::vector<int> parsed_int_arr = str_to_vec_int(const_init_val_ret.second);
            symbol_table_.AddConstArray(name_, dim0_size_, dim1_size_, parsed_int_arr);
        } else {
            handle_error("DataType error in ConstDef");
        }
    } else {
        handle_error("expect '=' in <ConstDef>");
    }

    output("<ConstDef>");
}

// ConstExp -> AddExp
// @brief: try to parse an integer out
// @retval: return an integer in string as the second of pair
std::pair<DataType, std::string> Parser::ConstExp() {
    int ret_int_value;
    auto add_exp_ret =  AddExp();
    try {
        // judge is an integer
        ret_int_value = std::stoi(add_exp_ret.second);
    } catch(std::invalid_argument& e) {
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

    if(is_integer(ret_var_name)) {
        cur_be_parsed_int = true;
    } else {
        cur_be_parsed_int = false;
    }

    next_sym();
    while (type_code_ == TypeCode::PLUS ||
            type_code_ == TypeCode::MINU){
        int sign = (type_code_ == TypeCode::PLUS)? 1:-1;
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
            std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_);
            IntermOp op = (sign == 1)? IntermOp::ADD : IntermOp::SUB;
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
    std::string ret_var_name;
    bool cur_be_parsed_int = false;
    std::pair<DataType, std::string> unary_exp_ret;

    unary_exp_ret = UnaryExp();
    ret_var_name = unary_exp_ret.second;
    if (is_integer(ret_var_name)) {
        cur_be_parsed_int = true;
    } else {
        cur_be_parsed_int = false;
    }


    next_sym();
    while (type_code_ == TypeCode::MULT ||
            type_code_ == TypeCode::DIV ||
            type_code_ == TypeCode::MOD) {
        int sign = (type_code_ == TypeCode::MULT)? 0 : ((type_code_==TypeCode::DIV)? 1:2);

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
                ret_var_name = std::to_string(parsed_unary_exp_value / cur_parsed_value);
            } else {
                ret_var_name = std::to_string(parsed_unary_exp_value % cur_parsed_value);
            }
        } else {
            cur_be_parsed_int = false;
            std::string tmp_var_name =
                    intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_);
            IntermOp op = (sign==0)? IntermOp::MUL : ((sign==1)? IntermOp::DIV:IntermOp::MOD);
            intermediate_.AddMidCode(tmp_var_name, op, ret_var_name, unary_exp_ret.second);
            ret_var_name = tmp_var_name;
        }
        next_sym();
    }
    // not *| / | %
    retract();
    output("<MulExp>");
    return std::make_pair(DataType::INT, ret_var_name);
}

// UnaryExp -> PrimaryExp |
//             Ident '(' [FuncRParams] ')' |
//             UnaryOp UnaryExp
// promise: already read a token
// note: 7 branches
std::pair<DataType, std::string> Parser::UnaryExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name; // init value is ""
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
    else if (type_code_ == TypeCode::PLUS ||
                type_code_ == TypeCode::MINU ||
                type_code_ == TypeCode::NOT) {
        std::string temp_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_);
        int op_no = UnaryOp(); // op number
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
        } else {
            if (op_no == 0) { // +
                // no change to final return var name, it is in inner_exp_ret
            } else if (op_no == 1) {
                std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, inner_exp_ret.first, cur_level_);
                intermediate_.AddMidCode(temp_var_name, IntermOp::SUB, "0", inner_exp_ret.second);
                ret_var_name = temp_var_name;
            } else {
                std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, inner_exp_ret.first, cur_level_);
                intermediate_.AddMidCode(temp_var_name, IntermOp::NOT, inner_exp_ret.second, "");
                ret_var_name = temp_var_name;
            }
        }
    }
    // Ident '(' [FuncRParams] ')'
    // Ident { '[' Exp ']' } may occur in Primary Expression
    // promise: parse const won't go into this branch
    else if (type_code_ == TypeCode::IDENFR) {
        next_sym();
        if (type_code_ == TypeCode::LPARENT) { // now sure that: Ident '(' [FuncRParams] ')'
            int need_param_num = 0;
            int provide_param_num = 0;
            std::string called_func_name = token_.get_str_value();
            auto search_res = symbol_table_.SearchFunc(called_func_name);
            if (search_res.first) {
                need_param_num = search_res.second->value;
                ret_type = search_res.second->data_type;
            } else {
                handle_error(ErrorType::UNDECL);
            }

            next_sym();
            if (type_code_ == TypeCode::RPARENT) {
                if (need_param_num != 0) handle_error(ErrorType::ARG_NO_MISMATCH);
                if (ret_type != DataType::VOID) {
                    ret_var_name = intermediate_.GenTmpVar(cur_func_name_, ret_type, cur_level_);
                    intermediate_.AddMidCode(ret_var_name, IntermOp::CALL, called_func_name, "");
                } else {
                    intermediate_.AddMidCode(ret_var_name, IntermOp::CALL, called_func_name, "");
                }
            } else {
                std::vector<std::string> param_list = FuncRParams(called_func_name, need_param_num);
                // TODO
                // just call the function, params have should be pushed in param list

                if (ret_type != DataType::VOID) {
                    ret_var_name = intermediate_.GenTmpVar(cur_func_name_, ret_type, cur_level_);
                    intermediate_.AddMidCode(ret_var_name, IntermOp::CALL, called_func_name, "");
                } else {
                    intermediate_.AddMidCode(ret_var_name, IntermOp::CALL, called_func_name, "");
                }
                next_sym();
                if (type_code_ == TypeCode::RPARENT) {
                    // pass
                } else {
                    handle_error(ErrorType::EXPECTED_PARENT);
                }
            }
        }
        // only one branch left
        else {
            retract();
            inner_exp_ret = PrimaryExp(); // will go to LVal
            ret_type = inner_exp_ret.first;
            ret_var_name = inner_exp_ret.second;
        }
    }
    else {
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
            handle_error(ErrorType::EXPECTED_PARENT);
        }
    }
    else if (type_code_ == TypeCode::IDENFR) {
        inner_exp_ret = LVal();
        ret_type = inner_exp_ret.first;
        ret_var_name = inner_exp_ret.second;
    }
    else if (type_code_ == TypeCode::INTCON) {
        ret_type = DataType::INT;
        int inner_value = Number();
        ret_var_name = std::to_string(inner_value);
    }
    else {
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
// @pre: use in fetch or be assigned to sth
// @pre: already read a token
std::pair<DataType, std::string> Parser::LVal() {
    DataType ret_type = DataType::INT;
    std::string ret_var_name;
    std::string ident;
    bool is_array = false;
    int dims = 0;
    std::string dim_0, dim_1;
    std::pair<DataType, std::string> inner_exp_ret;

    if (type_code_ == TypeCode::IDENFR) {
        ident = token_.get_str_value();
        auto search_res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, ident);
        if (!search_res.first) handle_error(ErrorType::UNDECL);
        next_sym();
        if (type_code_ == TypeCode::LBRACK) {
            is_array = true;
            dims += 1;
            next_sym();
            inner_exp_ret = Exp();
            dim_0 = inner_exp_ret.second;
            next_sym(); // read ']'
            if (type_code_ == TypeCode::RBRACK) {
                // pass
            } else {
                handle_error(ErrorType::EXPECTED_BRACK);
            }
            next_sym();
        }

        if (type_code_ == TypeCode::LBRACK) {
            next_sym();
            dims += 1;
            inner_exp_ret = Exp();
            dim_1 = inner_exp_ret.second;
            next_sym(); // read ']'
            if (type_code_ == TypeCode::RBRACK) {
                // pass
            } else {
                handle_error(ErrorType::EXPECTED_BRACK);
            }
            next_sym();
        }
    } else {
        handle_error("expect Ident in <LVal>");
    }
    // TODO
    // fetch the value of the LVal
    // save it to the ret_var_name
    // if is a integer, just pass it ,
    // else add to the mid code
    retract();
    output("<LVal>");
    return std::make_pair(ret_type, ret_var_name);
}

// Number -> IntConst
int Parser::Number() {
    int ret = IntConst();
    output("<Number>");
    return ret;
}

int Parser::IntConst() {
    // TODO
    // in fact, this func should return a integer or str
    int ret = std::stoi(token_.get_str_value());
    return ret;
}

// FuncRParams -> Exp { ',' Exp }
// promise: already read a token, at least one param
std::vector<std::string> Parser::FuncRParams(const std::string& func_name, int need_param_no) {
    std::vector<std::string> ret_vars;
    DataType need_param_type;
    int cur_param_no = 0;
    int provide_param_no = 0;
    std::pair<DataType, std::string> inner_exp_ret;

    provide_param_no += 1;
    inner_exp_ret = Exp();
    need_param_type = symbol_table_.GetKthParam(func_name, cur_param_no)->data_type;
    ret_vars.push_back(inner_exp_ret.second);
    if (inner_exp_ret.first != need_param_type)
        handle_error(ErrorType::ARG_TYPE_MISMATCH);
    next_sym();
    while (type_code_ == TypeCode::COMMA) {
        next_sym();
        provide_param_no += 1;
        cur_param_no += 1;
        inner_exp_ret = Exp();
        ret_vars.push_back(inner_exp_ret.second);
        if (need_param_no >= provide_param_no) {
            need_param_type = symbol_table_.GetKthParam(func_name, cur_param_no)->data_type;
            if (inner_exp_ret.first != need_param_type)
                handle_error(ErrorType::ARG_TYPE_MISMATCH);
            // TODO
            // push to stack
        } else {
            // pass
        }
        next_sym();
    }
    retract();
    output("<FuncRParams>");
    return ret_vars;
}

// <UnaryOp>::= '+' | '-' | '!'
// promise: already read a token
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
// promise: 1. already read a token
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
            // if that case, e.g. array[0] = {};
        }
        else {
            inner_exp_ret = ConstInitVal();
            if (inner_exp_ret.first == DataType::INT) {
                elements.push_back(std::stoi(inner_exp_ret.second));
            } else if (inner_exp_ret.first == DataType::INT_ARR){
                std::vector<int> inner_vec = str_to_vec_int(inner_exp_ret.second);
                elements.insert(elements.begin(), inner_vec.begin(), inner_vec.end());
            } else {
                handle_error("ConstInitVal return not INT not INT_ARR");
            }
            next_sym();
            while (type_code_ == TypeCode::COMMA) {
                next_sym();
                inner_exp_ret = ConstInitVal();
                if (inner_exp_ret.first == DataType::INT) {
                    elements.push_back(std::stoi(inner_exp_ret.second));
                } else if (inner_exp_ret.first == DataType::INT_ARR){
                    std::vector<int> inner_vec = str_to_vec_int(inner_exp_ret.second);
                    elements.insert(elements.begin(), inner_vec.begin(), inner_vec.end());
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
            handle_error(ErrorType::EXPECTED_SEMICN);
        }
    } else {
        handle_error("expect int in VarDecl begin");
    }
}

// VarDef -> Ident { '[' ConstExp ']' } |
//           Ident { '[' ConstExp ']' } '=' InitVal
// promise: already read a token
void Parser::VarDef() {
    name_ = token_.get_str_value();
    bool is_array = false;
    std::pair<DataType, std::string> inner_exp_ret;
    if (type_code_ == TypeCode::IDENFR) {
        next_sym();
        if (type_code_ == TypeCode::LBRACK) {
            is_array = true;
            dims_ += 1;
            next_sym();
            inner_exp_ret = ConstExp();
            dim0_size_ = std::stoi(inner_exp_ret.second);
            next_sym();
            if (type_code_ == TypeCode::RBRACK) {
                next_sym();
            } else {
                handle_error(ErrorType::EXPECTED_BRACK);
            }
        }

        if (type_code_ == TypeCode::LBRACK) {
            dims_ += 1;
            next_sym();
            inner_exp_ret = ConstExp();
            dim1_size_ = std::stoi(inner_exp_ret.second);
            next_sym();
            if (type_code_ == TypeCode::RBRACK) {
                next_sym();
            } else {
                handle_error(ErrorType::EXPECTED_BRACK);
            }
        }

        DataType data_type = is_array? DataType::INT_ARR : DataType::INT;
        bool add_success = symbol_table_.AddSymbol(cur_func_name_, data_type, SymbolType::VAR, name_,
                                                   0, cur_level_, dims_, dim0_size_, dim1_size_);
        if (!add_success) handle_error(ErrorType::REDEF);

        // ['=' InitVal]
        if (type_code_ == TypeCode::ASSIGN) {
            next_sym();
            inner_exp_ret = InitVal();
            // TODO
            // may only one value, may a arr values
            // use ARR_SAVE to save the values into the var arr
        } else {
            // done
            retract();
        }
        output("<VarDef>");
    } else {
        handle_error("expect identifier at begin of <VarDef>");
    }
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
        if ( type_code_ == TypeCode::RBRACE) {
            // end
        } else {
            inner_exp_ret = InitVal();
            if (inner_exp_ret.first == DataType::INT_ARR) {
                std::vector<std::string> inner_vec = str_to_vec_str(inner_exp_ret.second);
                ret_var_vec.insert(ret_var_vec.begin(), inner_vec.begin(), inner_vec.end());
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
                    ret_var_vec.insert(ret_var_vec.begin(), inner_vec.begin(), inner_vec.end());
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
// @pre: already read a token
void Parser::FuncDef() {
    DataType func_type = DataType::INVALID;
    int param_no = 0;

    func_type = FuncType();
    next_sym();
    if (type_code_ == TypeCode::IDENFR) { // func name
        std::string func_name = token_.get_str_value();
        next_sym(); // '('
        if (type_code_ == TypeCode::LPARENT) {
            next_sym(); // ')' or 'int'
            cur_level_ += 1;
            cur_func_name_ = func_name;
            if (!symbol_table_.AddFunc(func_type, func_name, 0)) {
                handle_error(ErrorType::REDEF);
            }

            if (type_code_ == TypeCode::RPARENT) {
                // go to Block
            } else if (type_code_ == TypeCode::INTTK) {
                param_no = FuncFParams();
                next_sym();
                if (type_code_ == TypeCode::RPARENT) {
                    // go to Block
                } else {
                    handle_error(ErrorType::EXPECTED_PARENT);
                }
            } else {
                handle_error(ErrorType::EXPECTED_PARENT);
            }
            symbol_table_.SearchFunc(cur_func_name_).second->value = param_no;
            next_sym();
            Block();
            cur_level_ -= 1;
            if (!has_ret_stmt_) {
                handle_error(ErrorType::MISSING_RET);
            } else {
                // pass
            }
            has_ret_stmt_ = false; // init the variable again
        } else {
            handle_error("expect a '(' in FuncDef");
        }
    } else {
        handle_error("expect Identifier in FuncDef");
    }
    output("<FuncDef>");
}

// <FuncType>::= 'int' | 'void'
DataType Parser::FuncType() {
    DataType ret_data_type = DataType::INVALID;
    if (type_code_ == TypeCode::VOIDTK) {
        ret_data_type = DataType::VOID;
    } else if (type_code_ == TypeCode::INTTK) {
        ret_data_type = DataType::INT;
    } else{
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
        FuncFParam();
        param_no += 1;
        next_sym();
        while (type_code_ == TypeCode::COMMA) {
            next_sym();
            FuncFParam();
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
void Parser::FuncFParam() {
    reset_sym();
    DataType param_type = DataType::INT;
    if (type_code_ == TypeCode::INTTK) {
        next_sym();
        if (type_code_ == TypeCode::IDENFR) {
            name_ = token_.get_str_value();
            next_sym();
            if (type_code_ == TypeCode::LBRACK) {
                dims_ = 1;
                param_type = DataType::INT_ARR;
                next_sym(); // eat ']'
                if (type_code_ == TypeCode::RBRACK) {
                    next_sym();
                    if (type_code_ == TypeCode::LBRACK) {
                        next_sym();
                        auto inner_exp_ret = ConstExp();
                        dim1_size_ = std::stoi(inner_exp_ret.second);
                        next_sym();
                        if (type_code_ == TypeCode::RBRACK) {
                            next_sym();
                        } else {
                            handle_error(ErrorType::EXPECTED_BRACK);
                        }
                    }
                    retract();
                } else {
                    handle_error(ErrorType::EXPECTED_BRACK);
                }
            } else {
                retract(); // read a token not '['
            }
            if (!symbol_table_.AddSymbol(cur_func_name_, param_type, SymbolType::PARAM, name_,
                                    0, cur_level_, dims_, dim0_size_, dim1_size_)) {
                handle_error(ErrorType::REDEF);
            };
            output("<FuncFParam>");
            return;
        } else {
            handle_error("expect Ident in <FuncFParam>");
        }
    } else {
        handle_error("expect int int FuncFParam");
    }
}

// Block -> '{' { BlockItem } '}'
// @pre: the level has been self-added
void Parser::Block() {
    if (type_code_ == TypeCode::LBRACE) {
        next_sym();
        // read a token from back to begin,
        // so we don't need to know the FIRSTof(BlockItem)
        while (type_code_ != TypeCode::RBRACE) {
            BlockItem(); // already read in
            next_sym();
        }
        // read '}'
        // end
    } else {
        handle_error("expected '{' at the begin of Block");
    }
    output("<Block>");
}

// BlockItem -> Decl | Stmt
void Parser::BlockItem() {
    if (type_code_ == TypeCode::CONSTTK ||
        type_code_ == TypeCode::INTTK) {
        Decl();
    } else {
        // in usual, we should judge
        Stmt();
    }
}

// Stmt -> LVal '=' Exp ';' |
//    	   [Exp] ';' |
//         Block |
//         'if' '(' Cond ')' Stmt [ 'else' Stmt ] |
//         'while' '(' Cond ')' Stmt |
//         'break' ';' | 'continue' ';' |
//         'return' [Exp] ';' |
//         LVal = 'getint''('')'';' |
//         'printf''('FormatString{,Exp}')'';' // 1.有Exp 2.⽆Exp
void Parser::Stmt() {
    if (type_code_ == TypeCode::LBRACE) {
        cur_level_ += 1;
        Block();
        cur_level_ -= 1;
    } else if (type_code_ == TypeCode::IFTK) {
        IfStmt();
    } else if (type_code_ == TypeCode::WHILETK) {
        loop_stack_.push_back(true);
        WhileStmt();
        loop_stack_.pop_back();
    } else if (type_code_ == TypeCode::BREAKTK ||
               type_code_ == TypeCode::CONTINUETK) {
        auto it = loop_stack_.end();
        if (*it) {
            // yes, it is in loop
        } else {
            handle_error(ErrorType::NOT_IN_LOOP);
        }
        next_sym();
        if (type_code_ == TypeCode::SEMICN) {
            // end
        } else {
            handle_error(ErrorType::EXPECTED_SEMICN);
        }
    } else if (type_code_ == TypeCode::RETURNTK) {
        ReturnStmt();
    } else if (type_code_ == TypeCode::PRINTFTK) {
        WriteStmt();
    } else if (type_code_ == TypeCode::SEMICN) {
        // empty stmt
        // end
    } else {
        // assign stmt, exp stmt, read stmt
        // read till ';'
        bool read_assign = false, read_getint = false;
        int read_times = 0;
        while (type_code_ != TypeCode::SEMICN ||
                type_code_ != TypeCode::LBRACE ||
                type_code_ != TypeCode::RBRACE) { // the ';' may be missing
            next_sym();
            if (type_code_ == TypeCode::ASSIGN) read_assign = true;
            if (type_code_ == TypeCode::GETINTTK) read_getint = true;
            read_times += 1;
        }
        while (read_times != 0) {
            retract();
            read_times -= 1;
        }
        if (read_assign && read_getint) {
            ReadStmt();
        } else if (read_assign && (!read_getint)) {
            AssignStmt();
        } else if ((!read_assign)&&(!read_getint)) {
            Exp();
            next_sym();
            if (type_code_ == TypeCode::SEMICN) {
                // end
            } else {
                handle_error(ErrorType::EXPECTED_SEMICN);
            }
        }
    }
    output("<Stmt>");
}


// AssignStmt -> LVal '=' Exp ';'
// @attention:
//      1. Error(LVal is undeclared) is handled in LVal()
//      2. if LVal is const, we will parse its value as an integer, then u can't change integer
void Parser::AssignStmt() {
    std::string assigned_var_name = token_.get_str_value();
    auto search_res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, assigned_var_name);
    if (search_res.first && search_res.second->symbol_type == SymbolType::CONST) {
        handle_error(ErrorType::CHANGE_CONST);
    }
    auto lVal_ret = LVal();
    next_sym();
    if (type_code_ == TypeCode::ASSIGN) {
        next_sym();
        auto exp_ret = Exp();
        next_sym();
        // TODO
        // generate mid code to change the value of lvalue
        if (type_code_ == TypeCode::SEMICN) {
            // end
        } else {
            handle_error(ErrorType::EXPECTED_SEMICN);
        }
    } else {
        handle_error("expect '=' end of <AssignStmt>");
    }
}

// IfStmt -> 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
// @pre: already read a if_token
void Parser::IfStmt() {
    next_sym();
    if (type_code_ == TypeCode::LPARENT) {
        next_sym();
        auto cond_ret = Cond();
        if (cond_ret.first == DataType::INT) {
            next_sym();
            if (type_code_ == TypeCode::RPARENT) {
                next_sym();
                // TODO
                // add mid code: BNE cond 1 else_block_label
                Stmt(); // if-block
                next_sym();
                if (type_code_ == TypeCode::ELSETK) {
                    // TODO
                    // GenLabel if_end_label
                    // AddMidCode: Jump if_end_label
                    // AddMidCode: else_block_label:
                    next_sym();
                    Stmt();
                    // AddMidCode: if_end_label:
                } else {
                    retract();
                    // TODO
                    // AddMidCode: else_block_label:
                }
            } else {
                handle_error(ErrorType::EXPECTED_PARENT);
            }
        } else {
            handle_error("expected ret_type INT from Cond");
        }
    } else {
        handle_error("expected a '(' in IfStmt");
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
// @attention: left recurrence
// @pre: already read a token
std::pair<DataType, std::string> Parser::LOrExp() {
    DataType ret_type = DataType::INVALID;
    std::string ret_var_name;
    bool cur_be_parsed_int = false;
    std::pair<DataType, std::string> inner_exp_ret;

    inner_exp_ret = LAndExp();
    ret_type = inner_exp_ret.first;
    ret_var_name = inner_exp_ret.second;
    if (is_integer(ret_var_name)) {
        cur_be_parsed_int = true;
    } else {
        cur_be_parsed_int = false;
    }

    next_sym();
    while (type_code_ == TypeCode::OR) {
        // erase then read
        retract();
        output("<LOrExp>");
        next_sym();

        next_sym();
        inner_exp_ret = LAndExp();
        if (cur_be_parsed_int && is_integer(inner_exp_ret.second)) {
            int parsed_inner_exp_value = std::stoi(inner_exp_ret.second);
            int cur_parsed_value = std::stoi(ret_var_name);
            ret_var_name = std::to_string((cur_parsed_value || parsed_inner_exp_value));
        } else {
            cur_be_parsed_int = false;
            std::string tmp_var_name =
                    intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_);
            IntermOp op = IntermOp::OR;
            intermediate_.AddMidCode(tmp_var_name, op, ret_var_name, inner_exp_ret.second);
            ret_var_name = tmp_var_name;
        }
        next_sym();
    }
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

    bool cur_be_parsed_int = false;
    std::pair<DataType, std::string> inner_exp_ret;
    inner_exp_ret = EqExp();
    ret_type = inner_exp_ret.first;
    ret_var_name = inner_exp_ret.second;

    if (is_integer(ret_var_name)) {
        cur_be_parsed_int = true;
    }  else {
        cur_be_parsed_int = false;
    }

    next_sym();
    while (type_code_ == TypeCode::AND) {
        // erase then read
        retract();
        output("<LAndExp>");
        next_sym();

        next_sym();
        inner_exp_ret = EqExp();
        if (cur_be_parsed_int && is_integer(inner_exp_ret.second)) {
            int parsed_inner_value = std::stoi(inner_exp_ret.second);
            int cur_parsed_value = std::stoi(ret_var_name);
            ret_var_name = std::to_string((cur_parsed_value && parsed_inner_value));
        } else {
            cur_be_parsed_int = false;
            std::string tmp_var_name =
                    intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_);
            intermediate_.AddMidCode(tmp_var_name, IntermOp::AND, ret_var_name, inner_exp_ret.second);
            ret_var_name = tmp_var_name;
        }
        next_sym();
    }
    retract();
    output("<LAndExp>");
    return std::make_pair(ret_type, ret_var_name);
}

// EqExp -> RelExp | EqExp ( '==' | '!=' ) RelExp
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
    while (type_code_ == TypeCode::EQL ||
           type_code_ == TypeCode::NEQ) {
        // erase then read
        retract();
        output("<EqExp>");
        next_sym();

        int sign = (type_code_ == TypeCode::EQL)? 0 : 1;
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
                    intermediate_.GenTmpVar(cur_func_name_, ret_type, cur_level_);
            IntermOp op = (sign == 0)? IntermOp::EQ : IntermOp::NEQ;
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
    while (type_code_ == TypeCode::LSS ||
           type_code_ == TypeCode::LEQ ||
           type_code_ == TypeCode::GRE ||
           type_code_ == TypeCode::GEQ) {
        // erase the token
        retract();
        output("<RelExp>");
        next_sym();

        next_sym();
        inner_exp_ret = AddExp();
        if (cur_be_parsed_int && is_integer(inner_exp_ret.second)) {
            int parsed_inner_value = std::stoi(inner_exp_ret.second);
            int cur_parsed_value = std::stoi(ret_var_name);
            if (type_code_ == TypeCode::LSS) {
                ret_var_name = std::to_string((cur_parsed_value < parsed_inner_value));
            } else if (type_code_ == TypeCode::LEQ) {
                ret_var_name = std::to_string((cur_parsed_value <= parsed_inner_value));
            } else if (type_code_ == TypeCode::GRE) {
                ret_var_name = std::to_string((cur_parsed_value > parsed_inner_value));
            } else {
                ret_var_name = std::to_string((cur_parsed_value >= parsed_inner_value));
            }
        } else {
            cur_be_parsed_int = false;
            IntermOp op;
            if (type_code_ == TypeCode::LSS) {
                op = IntermOp::LSS;
            } else if (type_code_ == TypeCode::LEQ) {
                op = IntermOp::LEQ;
            } else if (type_code_ == TypeCode::GRE) {
                op = IntermOp::GRE;
            } else {
                op = IntermOp::GEQ;
            }
            std::string tmp_var_name =
                    intermediate_.GenTmpVar(cur_func_name_, ret_type, cur_level_);
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
    // TODO
    // generate mid code
    if (type_code_ == TypeCode::WHILETK) {
        next_sym(); // '('
        if (type_code_ == TypeCode::LPARENT) {
            next_sym();
            Cond();
            next_sym();
            if (type_code_ == TypeCode::RPARENT) {
                next_sym();
                Stmt();
            }
            else {
                handle_error(ErrorType::EXPECTED_PARENT);
            }
        } else {
            handle_error("expect '(' in <WhileStmt>");
        }
    } else {
        handle_error("while");
    }
}

// ReturnStmt -> 'return' [<Exp>] ';'
// @pre: return statement must in a function def
void Parser::ReturnStmt() {
    has_ret_stmt_ = true;
    DataType parsed_ret_type = DataType::VOID;
    if (type_code_ == TypeCode::RETURNTK) {
        next_sym();
        if (first_exp.count(type_code_) != 0) {
            auto inner_exp_ret = Exp(); // already read a token
            parsed_ret_type = inner_exp_ret.first;
            next_sym();
            if (type_code_ == TypeCode::SEMICN) {
                // end
            } else {
                handle_error(ErrorType::EXPECTED_SEMICN);
            }
        } else if (type_code_ == TypeCode::SEMICN){
            // end
        } else {
            handle_error(ErrorType::EXPECTED_SEMICN);
        }
        DataType should_ret_type = symbol_table_.SearchFunc(cur_func_name_).second->data_type;
        if (should_ret_type == DataType::VOID && parsed_ret_type != DataType::VOID) {
            handle_error(ErrorType::RET_TYPE_MISMATCH);
        }
    } else {
        handle_error("expect 'return' in ReturnStmt");
    }
}

// ReadStmt -> LVal '=' 'getint' '(' ')' ';'
void Parser::ReadStmt() {
    std::string assigned_var_name = token_.get_str_value();
    auto search_res = symbol_table_.SearchNearestSymbolNotFunc(cur_func_name_, assigned_var_name);
    if (search_res.first && search_res.second->symbol_type == SymbolType::CONST) {
        handle_error(ErrorType::CHANGE_CONST);
    }
    auto lVal_ret = LVal();
    // TODO: handle error
    next_sym(); // eat =
    next_sym(); // eat getint
    next_sym(); // eat  (
    next_sym(); // eat )
    if (type_code_ != TypeCode::RPARENT) {
        handle_error(ErrorType::EXPECTED_PARENT);
    }
    next_sym();
    if (type_code_ != TypeCode::SEMICN) { // eat ;
        handle_error(ErrorType::EXPECTED_SEMICN);
    }
}

// WriteStmt-> 'printf' '(' FormatString {',' Exp } ')' ';'
// promise: printf is the token read, don't check
void Parser::WriteStmt() {
    int format_no = 0;
    int exp_no = 0;
    next_sym();

    if (type_code_ == TypeCode::LPARENT) {
        next_sym();  // this sym is STRCON, FormatString
        auto inner_ret = FormatString();
        format_no = inner_ret.first;
        next_sym();
        while (type_code_ == TypeCode::COMMA) {
            next_sym();
            Exp();
            exp_no += 1;
            next_sym(); // will be ',' ?
        }
        if (exp_no != format_no) {
            handle_error(ErrorType::PRINT_NO_MISMATCH);
        }
        if (type_code_ == TypeCode::RPARENT) {
            next_sym();
            if (type_code_ == TypeCode::SEMICN) {
                // end
            } else {
                handle_error("expect ';' end of WriteStmt");
            }
        } else {
            handle_error(ErrorType::EXPECTED_SEMICN);
        }
    } else {
        handle_error("expect '(' in WriteStmt");
    }
}

// @note: ascii of '%' is 37
// TODO
// test
std::pair<int, std::vector<std::string>> Parser::FormatString() {
    int format_no = 0;
    std::vector<std::string> vec_str;
    std::string str_con = token_.get_str_value();
    int i = 0;
    if (str_con[i] == '"') {
        i++;
        while (str_con[i] == '%' ||
            str_con[i] == 32 || str_con[i]==33 || (40 <= str_con[i] && str_con[i] <= 126) || str_con[i]=='\\') {
            if (str_con[i] == '%') {
                i+=1;
                if (str_con[i] == 'd') {
                    vec_str.emplace_back("%d");
                    format_no += 1;
                    i+=1;
                } else {
                    handle_error(ErrorType::ILLEGAL_CHAR);
                }
            } else if (str_con[i] == '\\'){
                i += 1;
                if (str_con[i] == 'n') {
                    vec_str.emplace_back("\\n");
                    i += 1;
                } else {
                    handle_error(ErrorType::ILLEGAL_CHAR);
                }
            } else {
                std::string str_tmp;
                while (str_con[i] == 37 || str_con[i] == 32 || str_con[i]==33 ||
                        (40 <= str_con[i] && str_con[i] <= 126) || str_con[i]=='\\') {
                    str_tmp += str_con[i];
                    i += 1;
                }
                vec_str.push_back(str_tmp);
            }
        }
        if (str_con[i] == '"') {
            // end
        } else {
            handle_error(ErrorType::ILLEGAL_CHAR);
        }

    } else {
        handle_error(ErrorType::ILLEGAL_CHAR);
    }
    return std::make_pair(format_no, vec_str);
}


// MainFuncDef-> 'int' 'main' '(' ')' Block
void Parser::MainFuncDef() {
    next_sym(); // int main
    next_sym(); // int main (
    next_sym(); // int main ( )
    next_sym();
    Block();
    output("<MainFuncDef>");
}

// @brief: rest current symbol info
void Parser::reset_sym() {
    name_ = "";
    dims_ = dim0_size_ = dim1_size_ = 0;
}
