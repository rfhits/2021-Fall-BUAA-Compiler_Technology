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

// ConstDecl-> const int ConstDef {, ConstDef}
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

// ConstDef-> Ident {'[' ConstExp ']'} '=' ConstInitVal
// promise: already read an Identifier
void Parser::ConstDef() {
    next_sym();
    if (type_code_ == TypeCode::LBRACK) {
        next_sym();
        auto const_ret = ConstExp();

        next_sym();
        if (type_code_ == TypeCode::RBRACK) {
            next_sym();
        } else {
            handle_error("expect a ] match for [");
        }
    }

    if (type_code_ == TypeCode::LBRACK) {
        next_sym();
        ConstExp();
        next_sym();
        if (type_code_ == TypeCode::RBRACK) {
            next_sym();
        } else {
            handle_error("expect a ] match for [");
        }
    }

    // expect '='
    if (type_code_ == TypeCode::ASSIGN) {
        next_sym();
        ConstInitVal();
    } else {
        handle_error("expect = in <ConstDef>");
    }
    output("<ConstDef>");
}

// ConstExp-> AddExp
int Parser::ConstExp() {
    auto addexp_ret =  AddExp(true);
    output("<ConstExp>");
    return std::stoi(addexp_ret.second);
}

// AddExp-> MulExp | AddExp ('+'|'-') MulExp
// AddExp-> MulExp { ('+'|'-') MulExp}
// note: left recurrence
// promise: already read a token
// @param: parse_const: should parse a const but not an expr
std::pair<DataType, std::string> Parser::AddExp(bool parse_const) {
    int parsed_value = 0; // for parse const
    std::pair<DataType, std::string> mulexp_ret; // ret received from MulExp
    std::string ret_var_name;
    DataType data_type = DataType::INVALID;

    mulexp_ret = MulExp(parse_const);
    if (parse_const) {
        parsed_value = std::stoi(mulexp_ret.second);
    } else {
        ret_var_name = mulexp_ret.second;
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
        mulexp_ret = MulExp();
        if (parse_const) {
            parsed_value += sign * std::stoi(mulexp_ret.second);
        } else {
            // combine it with the var before
            std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_);
            IntermOp op = (sign == 1)? IntermOp::ADD : IntermOp::SUB;
            intermediate_.AddMidCode(tmp_var_name, op, ret_var_name, mulexp_ret.second);
            ret_var_name = tmp_var_name;
        }
        next_sym();
    }
    retract(); // not '+' or '-', retract
    output("<AddExp>");
    if (parse_const) {
        return std::make_pair(DataType::NUM, std::to_string(parsed_value));
    } else {
        return std::make_pair(data_type, ret_var_name);
    }
}

// MulExp-> UnaryExp | MulExp (* / %) UnaryExp
// MulExp-> UnaryExp {('*' | '/' | '%') UnaryExp}
// alert: left recurrence
// promise: already read a token
// @ret:
//   if parse_const, return a NUM
std::pair<DataType, std::string> Parser::MulExp(bool parse_const) {
    int parsed_value = 0;
    std::pair<DataType, std::string> unary_exp_ret;
    std::string ret_var_name;

    unary_exp_ret = UnaryExp(parse_const);
    if (parse_const) {
        parsed_value = std::stoi(unary_exp_ret.second);
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
        if (parse_const) {
            int parsed_unary_exp_value = std::stoi(unary_exp_ret.second);
            if (sign == 0) {
                parsed_value *= parsed_unary_exp_value;
            } else if (sign == 1) {
                parsed_value /= parsed_unary_exp_value;
            } else {
                parsed_value %= parsed_unary_exp_value;
            }
        } else {
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
    if (parse_const) {
        return std::make_pair(DataType::NUM, std::to_string(parsed_value));
    } else {
        return std::make_pair(DataType::INT, ret_var_name);
    }
}

// UnaryExp -> PrimaryExp |
//             Ident '(' [FuncRParams] ')' |
//             UnaryOp UnaryExp
// promise: already read a token
// note: 7 branches
std::pair<DataType, std::string> Parser::UnaryExp(bool parse_const) {
    int parsed_value = 0;
    std::string ret_var_name;
    DataType ret_type;
    std::pair<DataType, std::string> inner_exp_ret;


    // ( Exp )
    if (type_code_ == TypeCode::LPARENT) {
        inner_exp_ret = PrimaryExp(parse_const);
        if (parse_const) {
            parsed_value = std::stoi(inner_exp_ret.second);
        } else {
            ret_var_name = inner_exp_ret.second;
        }
    }
    else if (type_code_ == TypeCode::INTCON) {
        inner_exp_ret = PrimaryExp(parse_const);
        if (parse_const) {
            parsed_value = std::stoi(inner_exp_ret.second);
        } else {
            ret_var_name = inner_exp_ret.second;
        }
    }
    else if (type_code_ == TypeCode::PLUS ||
                type_code_ == TypeCode::MINU ||
                type_code_ == TypeCode::NOT) {
        std::string temp_var_name = intermediate_.GenTmpVar(cur_func_name_, DataType::INT, cur_level_);
        int op_no = UnaryOp();
        next_sym();
        inner_exp_ret = UnaryExp();
        parsed_value = std::stoi(inner_exp_ret.second);
        if (op_no == 0) { // +
            // no change to final return var name, it is in inner_exp_ret
        } else if (op_no == 1) {
            if (parse_const) {
                parsed_value = 0 - parsed_value;
            } else {
                std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, inner_exp_ret.first, cur_level_);
                intermediate_.AddMidCode(temp_var_name, IntermOp::SUB, "0", inner_exp_ret.second);
                ret_var_name = temp_var_name;
            }
        }
    }
    // Ident '(' [FuncRParams] ')'
    // Ident { '[' Exp ']' } may occur in Primary Expression
    // promise: parse const won't go into this branch
    else if (type_code_ == TypeCode::IDENFR) {
        std::string called_func_name = token_.get_str_value();
        int need_param_num = 0;
        int provide_param_num = 0;
        next_sym();
        if (type_code_ == TypeCode::LPARENT) {
            // now sure that: Ident '(' [FuncRParams] ')'
            auto search_res = symbol_table_.SearchVisibleSymbol("", called_func_name, 0);
            need_param_num = search_res.second->value;

            if (search_res.first == false) handle_error(ErrorType::C);

            ret_type = search_res.second->data_type;
            if (ret_type == DataType::VOID) handle_error(ErrorType::E);

            next_sym();
            if (type_code_ == TypeCode::RPARENT) {
                if (need_param_num != 0) handle_error(ErrorType::D);
                if (ret_type != DataType::VOID) {
                    std::string tmp_var_name = intermediate_.GenTmpVar(cur_func_name_, ret_type, cur_level_);
                    intermediate_.AddMidCode(tmp_var_name, IntermOp::CALL, called_func_name, "");
                } else {
                    // pass;
                }
            } else {
                FuncRParams();
                next_sym();
                if (type_code_ == TypeCode::RPARENT) {
                    // pass
                } else {
                    handle_error(ErrorType::J);
                }
            }
        }
        // only one branch left
        else {
            retract();
            PrimaryExp(); // will go to LVal
        }
    }
    else {
        handle_error("UnaryExp not match");
    }
    output("<UnaryExp>");
}


// <PrimaryExp>::= '(' Exp ')' |
//                 LVal |
//                 Number
// first: '(', IDENFR, INTCON
// promise: already a token
std::pair<DataType, std::string> Parser::PrimaryExp(bool parse_const) {
    if (type_code_ == TypeCode::LPARENT) {
        next_sym();
        Exp();
        next_sym();
        if (type_code_ == TypeCode::RPARENT) {
            // pass
        } else {
            handle_error("expect ) in <Primary>::= '(' Exp ')'");
        }
    }
    else if (type_code_ == TypeCode::IDENFR) {
        LVal();
    }
    else if (type_code_ == TypeCode::INTCON) {
        Number();
    }
    else {
        handle_error("<PrimaryExp> cant match any branches");
    }
    output("<PrimaryExp>");
}

// <Exp>::= <AddExp>
void Parser::Exp() {
    AddExp();
    output("<Exp>");
}

// <LVal>::= Ident { '[' Exp ']' }
// promise: already read a token
void Parser::LVal() {
    if (type_code_ == TypeCode::IDENFR) {
        next_sym();
        while (type_code_ == TypeCode::LBRACK) {
            next_sym();
            Exp();
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
    retract();
    output("<LVal>");
}

void Parser::Number() {
    IntConst();
    output("<Number>");
}

void Parser::IntConst() {
    // TODO
    // in fact, this func should return a integer or str
}

// <FuncRParams>::= <Exp> { ',' <Exp> }
// promise: already read a token
std::vector<std::string> Parser::FuncRParams() {
    Exp();
    next_sym();
    while (type_code_ == TypeCode::COMMA) {
        next_sym();
        Exp();
        next_sym();
    }
    retract();
    output("<FuncRParams>");
}

// <UnaryOp>::= '+' | '-' | '!'
// promise: already read a token
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

// <ConstInitVal>::= '{' [ ConstInitVal { ',' ConstInitVal } ] '}' |
//               <ConstExp>
// promise: already read a token
void Parser::ConstInitVal() {
    if (type_code_ == TypeCode::LBRACE) {
        next_sym();
        if (type_code_ == TypeCode::RBRACE) {
            // end
        } else {
            ConstInitVal();
            next_sym();
            while (type_code_ == TypeCode::COMMA) {
                next_sym();
                ConstInitVal();
                next_sym();
            }
            if (type_code_ == TypeCode::RBRACE) {
                // end
            } else {
                handle_error("expect '}' at end of <ConstInitVal>");
            }
        }
    } else {
        ConstExp();
    }
    output("<ConstInitVal>");
}


// <VarDecl>::= BType VarDef { ',' VarDef } ';'
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

// <VarDef>::= Ident { '[' ConstExp ']' } |
//             Ident { '[' ConstExp ']' } '=' InitVal
// promise: already read a token
void Parser::VarDef() {
    if (type_code_ == TypeCode::IDENFR) {
        next_sym();
        while (type_code_ == TypeCode::LBRACK) {
            next_sym();
            ConstExp();
            next_sym();
            if (type_code_ == TypeCode::RBRACK) {
                next_sym();
            } else {
                handle_error("expect ] at end of <VarDef>");
            }
        }
        // ['=' InitVal]
        if (type_code_ == TypeCode::ASSIGN) {
            next_sym();
            InitVal();
        } else {
            // done
            retract();
        }
        output("<VarDef>");
    } else {
        handle_error("expect identifier at begin of <VarDef>");
    }
}

// <InitVal>::= '{' [ InitVal { ',' InitVal } ] '}'
//              Exp |
// promise: already read a token
void Parser::InitVal() {
    if (type_code_ == TypeCode::LBRACE) {
        next_sym();
        if ( type_code_ == TypeCode::RBRACE) {
            // end
        } else {
            InitVal();
            next_sym();
            while (type_code_ == TypeCode::COMMA) {
                next_sym();
                InitVal();
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
        Exp();
    }
    output("<InitVal>");
}

// <FuncDef>::= FuncType Ident '(' [FuncFParams] ')' Block
// promise: already read a token
void Parser::FuncDef() {
    FuncType();
    next_sym();
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
        handle_error("expect Identifier in <FuncDef>");
    }
    output("<FuncDef>");
}

// <FuncType>::= 'int' | 'void'
void Parser::FuncType() {
    if (type_code_ == TypeCode::VOIDTK) {
        // pass
    } else if (type_code_ == TypeCode::INTTK) {
        // pass
    } else{
        handle_error("FuncType must be int of void");
    }
    output("<FuncType>");
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
