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


// CompUnit-> {Decl} {FuncDef} MainFuncDef
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

// Decl-> ConstDecl | VarDecl
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
            handle_error("expect ; at end of ConstDecl");
        }
    } else {
        handle_error("Const expect a int");
    }
}

// ConstDef -> Ident {'[' ConstExp ']'} '=' ConstInitVal
// promise: already read an Identifier
void Parser::ConstDef() {
    bool is_array = false;
    std::string const_name = token_.get_str_value();
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        is_array = true;
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
        if (const_init_val_ret.first == DataType::INT) {
            int parsed_int = std::stoi(const_init_val_ret.second);
            symbol_table_.AddSymbol("", DataType::INT, SymbolType::CONST,
                                    const_name, parsed_int, 0);
        } else if (const_init_val_ret.first == DataType::INT_ARR) {
            std::vector<int> parsed_int_arr = str_to_vec_int(const_init_val_ret.second);
            symbol_table_.AddConstArray(const_name, dim0_size_, dim1_size_, parsed_int_arr);
        } else {
            handle_error("DataType error in ConstDef");
        }
    } else {
        handle_error("expect '=' in <ConstDef>");
    }

    output("<ConstDef>");
}

// ConstExp-> AddExp
// check: is an integer string return
// promise: return an integer in string
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
// note: left recurrence
// promise: already read a token
// try to parse as integer
// if can't, return a temp_var_name
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

// MulExp-> UnaryExp | MulExp (* / %) UnaryExp
// MulExp-> UnaryExp {('*' | '/' | '%') UnaryExp}
// alert: left recurrence
// promise: already read a token
// try to parse it: 2*a, 2*3/4
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
            std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_);
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
            auto search_res = symbol_table_.SearchSymbol("", called_func_name, 0);
            need_param_num = search_res.second->value;

            if (!search_res.first) handle_error(ErrorType::UNDECL);

            ret_type = search_res.second->data_type;

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
// promise: already read a token
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
        if (!symbol_table_.SearchSymbol(cur_func_name_, ident, cur_level_).first) handle_error(ErrorType::UNDECL);
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
                handle_error("expect ']' in <LVal> ");
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
            handle_error("expect ';' at end of <VarDef>");
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
// promise: already read a token
void Parser::FuncDef() {
    DataType func_type = DataType::INVALID;
    func_type = FuncType();
    next_sym();
    std::string func_name = token_.get_str_value();
    if (type_code_ == TypeCode::IDENFR) {
        next_sym(); // '('
        next_sym(); // ')' or 'int'
        if (type_code_ == TypeCode::RPARENT) {
            // go to Block
        } else {
            FuncFParams();
            next_sym();
            if (type_code_ == TypeCode::RPARENT) {
                // go to Block
            } else {
                handle_error("expect ')' in <FuncDef>");
            }
        }
        next_sym();
        Block();
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


// <FuncFParams>::= <FuncFParam> {',' FuncFParam}
// promise:
// note: FuncFParam must begin with "int"
void Parser::FuncFParams() {
    if (type_code_ == TypeCode::INTTK) {
        FuncFParam();
        next_sym();
        while (type_code_ == TypeCode::COMMA) {
            next_sym();
            FuncFParam();
            next_sym();
        }
    } else {
        handle_error("expect 'int' in <FuncFParams> begin");
    }
    retract();
    output("<FuncFParams>");
}

// <FuncFParam>::= BType Ident [ '[' ']' { '[' ConstExp ']' }]
void Parser::FuncFParam() {
    if (type_code_ == TypeCode::INTTK) {
        next_sym();
        if (type_code_ == TypeCode::IDENFR) {
            next_sym();
            if (type_code_ == TypeCode::LBRACK) {
                next_sym(); // now at []
                next_sym();
                while (type_code_ == TypeCode::LBRACK) {
                    next_sym();
                    ConstExp();
                    next_sym();
                    if (type_code_ == TypeCode::RBRACK) {
                        next_sym(); // {} while
                    } else {
                        handle_error("expect ']' in <FuncFParam>");
                    }
                }
                retract();
//                return ;
            } else {
                retract();
//                return ;
            }
            output("<FuncFParam>");
            return;
        } else {
            handle_error("expect Ident in <FuncFParam>");
        }
    } else {
        handle_error("expect int int FuncFParam");
    }
}

// <Block>::= '{' { BlockItem } '}'
void Parser::Block() {
    if (type_code_ == TypeCode::LBRACE) {
        next_sym();
        // from back to begin
        while (type_code_ != TypeCode::RBRACE) {
            BlockItem(); // already read in
            next_sym();
        }
        // read '}'
        // end
    } else {
        handle_error("<Block> begin with '{'");
    }
    output("<Block>");
}

//
void Parser::BlockItem() {
    if (type_code_ == TypeCode::CONSTTK ||
        type_code_ == TypeCode::INTTK) {
        Decl();
    } else {
        // in usual, we should judge
        Stmt();
    }
}

//
void Parser::Stmt() {
    if (type_code_ == TypeCode::LBRACE) {
        Block();
    } else if (type_code_ == TypeCode::IFTK) {
        IfStmt();
    } else if (type_code_ == TypeCode::WHILETK) {
        WhileStmt();
    } else if (type_code_ == TypeCode::BREAKTK ||
               type_code_ == TypeCode::CONTINUETK) {
        next_sym();
        if (type_code_ == TypeCode::SEMICN) {
            // end
        } else {
            handle_error("expect ';' behind break | continue");
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
        while (type_code_ != TypeCode::SEMICN) {
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
                handle_error("expect ';' end of <stmt>");
            }
        }
    }
    output("<Stmt>");
}


// AssignStmt::= LVal '=' Exp ';'
void Parser::AssignStmt() {
    LVal();
    next_sym();
    if (type_code_ == TypeCode::ASSIGN) {
        next_sym();
        Exp();
        next_sym();
        if (type_code_ == TypeCode::COMMA) {
            // end
        } else {
            handle_error("expect ; end of Assignment");
        }
    } else {
        handle_error("expect '=' end of <AssignStmt>");
    }
}

// IfStmt::= 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
// promise: already read a if_token
void Parser::IfStmt() {
    next_sym();
    if (type_code_ == TypeCode::LPARENT) {
        next_sym();
        Cond();
        next_sym();
        if (type_code_ == TypeCode::RPARENT) {
            next_sym();
            Stmt();
            next_sym();
            if (type_code_ == TypeCode::ELSETK) {
                next_sym();
                Stmt();
            } else {
                retract();
            }
        } else {
            handle_error("");
        }
    }
}

// Cond-> LOrExp
void Parser::Cond() {
    LOrExp();
    output("<Cond>");
}

// LOrExp -> LAndExp | LOrExp '||' LAndExp
// LOrExp -> LAndExp { '||' LAndExp }
// note: left recurrence
// promise: already read a token
void Parser::LOrExp() {
    LAndExp();
    next_sym();
    while (type_code_ == TypeCode::OR) {
        // erase then read
        retract();
        output("<LOrExp>");
        next_sym();

        next_sym();
        LAndExp();
        next_sym();
    }
    retract();
    output("<LOrExp>");
}

// LAndExp -> EqExp | LAndExp '&&' EqExp
// LAndExp -> EqExp { '&&' EqExp }
// note: left recurrence
// promise: already read a token
void Parser::LAndExp() {
    EqExp();
    next_sym();
    while (type_code_ == TypeCode::AND) {
        // erase then read
        retract();
        output("<LAndExp>");
        next_sym();

        next_sym();
        EqExp();
        next_sym();
    }
    retract();
    output("<LAndExp>");
}

// <EqExp>::= <RelExp> | <EqExp> ( '==' | '!=' ) <RelExp>
// <EqExp>::= <RelExp> { ('==' | '!=') <RelExp>}
// note: left recurrence
// promise: already read a token
void Parser::EqExp() {
    RelExp();
    next_sym();
    while (type_code_ == TypeCode::EQL ||
           type_code_ == TypeCode::NEQ) {
        // erase then read
        retract();
        output("<EqExp>");
        next_sym();

        next_sym();
        RelExp();
        next_sym();
    }
    retract();
    output("<EqExp>");
}


// <RelExp>::= AddExp | RelExp ('<' | '>' | '<=' | '>=') AddExp
// RelExp::= <AddExp> { ('<' | '>' | '<=' | '>=') <AddExp> }
// note: left recurrence
void Parser::RelExp() {
    AddExp();
    next_sym();
    while (type_code_ == TypeCode::LSS ||
           type_code_ == TypeCode::LEQ ||
           type_code_ == TypeCode::GRE ||
           type_code_ == TypeCode::GEQ) {
        // erase the token then ...
        retract();
        output("<RelExp>");
        next_sym();

        next_sym();
        AddExp();
        next_sym();
    }
    retract();
    output("<RelExp>");
}

// <WhileStmt>::= 'while' '(' Cond ')' Stmt
void Parser::WhileStmt() {
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
                handle_error("expect ')' in <WhileStmt>");
            }
        } else {
            handle_error("expect '(' in <WhileStmt>");
        }

    } else {
        handle_error("while");
    }
}

// ReturnStmt-> 'return' [<Exp>] ';'
void Parser::ReturnStmt() {
    if (type_code_ == TypeCode::RETURNTK) {
        next_sym();
        if (type_code_ == TypeCode::SEMICN) {
            // end
        } else {
            Exp(); // already read a token
            next_sym();
            if (type_code_ == TypeCode::SEMICN) {
                // end
            } else {
                handle_error("expect ';' end of <ReturnStmt>");
            }
        }
    } else {
        handle_error("expect 'return' in <ReturnStmt>");
    }
}


// ReadStmt-> LVal '=' 'getint' '(' ')' ';'
void Parser::ReadStmt() {
    LVal();
    // TODO: handle error
    next_sym(); // =
    next_sym(); // = getint
    next_sym(); // = getint (
    next_sym(); // = getint ( )
    next_sym(); // = getint ( ) ;
}

// WriteStmt-> 'printf' '(' FormatString {',' Exp } ')' ';'
// promise: printf is the token read, don't check
void Parser::WriteStmt() {
    next_sym();
    if (type_code_ == TypeCode::LPARENT) {
        next_sym();  // FormatString
        next_sym(); //
        while (type_code_ == TypeCode::COMMA) {
            next_sym();
            Exp();
            next_sym(); // will be ',' ?
        }
        if (type_code_ == TypeCode::RPARENT) {
            next_sym();
            if (type_code_ == TypeCode::SEMICN) {
                // end
            } else {
                handle_error("expect ';' end of WriteStmt");
            }
        } else {
            handle_error("expect ')' in WriteStmt");
        }
    } else {
        handle_error("expect '(' in WriteStmt");
    }
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
