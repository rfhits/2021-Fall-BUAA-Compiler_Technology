//
// Created by WYSJ6174 on 2021/10/3.
//

#include <commctrl.h>
#include "Parser.h"

Parser::Parser(Lexer &lexer, ErrorHandler &error_handler, bool print_mode, std::ofstream &out) :
        lexer_(lexer), error_handler_(error_handler), print_mode_(print_mode), out_(out) {}


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

// CompUnit::=
void Parser::Program() {
    next_sym();
    // three cond: const / int / void
    // three branches: Decl, Func, Main
    while (true) {
        if (type_code_ == TypeCode::CONSTTK) {
            Decl();
            next_sym();
        } else if (type_code_ == TypeCode::VOIDTK) {
            break;
        }
        // int a = / int a[ / int a( / int main(
        else if (type_code_ == TypeCode::INTTK) {
            next_sym(); // int a
            next_sym(); // int a *
            if (type_code_ == TypeCode::ASSIGN ||
                type_code_ == TypeCode::LBRACK) {
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
            handle_error("const or int or void at {Decl}");
        }
    }

    // {FuncDef}
    while (true) {
        if (type_code_ == TypeCode::VOIDTK) {
            FuncDef();
            next_sym();
        }
        // int a( / int main(
        else if (type_code_ == TypeCode::INTTK) {
            next_sym();
            if (type_code_ == TypeCode::IDENFR) {
                retract();
                FuncDef();
                next_sym();
            }
            else {
                retract();
                break;
            }
        } else {
//            handle_error("funcdef error");
            break;
        }
    }

    // MainDef
    next_sym();
    if (type_code_ == TypeCode::MAINTK){
        retract();
        MainFuncDef();
    } else {
        handle_error("expect main");
    }

    out_strings_.emplace_back("CompUnit");
    // TODO
    // OUTPUT string in vector
}

// Decl-> ConstDecl | VarDecl
// promise: already has a CONST or INT
void Parser::Decl() {
    if (type_code_ == TypeCode::CONSTTK) {
        ConstDecl();
    } else if (type_code_ == TypeCode::INTTK) {
        VarDecl();
    } else {
        handle_error("Decl expect a const or int");
    }
}

// <ConstDecl>::= const int <ConstDef> {, <ConstDef>}
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

// <ConstDef>::= Ident {'[' ConstExp ']'} '=' ConstInitVal
// promise: already read an Indent, AK type_code==Ident
void Parser::ConstDef() {
    next_sym();
    while (type_code_ == TypeCode::LBRACK) {
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

// <ConstExp>::= AddExp
void Parser::ConstExp() {
    AddExp();
    output("<ConstExp>");
}

// <AddExp>::= <MulExp> | <AddExp> ('+'|'-') <MulExp>
// <AddExp>::= <MulExp> {('+'|'-') <MulExp>}
// !!: left recurrence
// promise: already read a token
void Parser::AddExp() {
    MulExp();
    next_sym();
    while (type_code_ == TypeCode::PLUS ||
            type_code_ == TypeCode::MINU){
        next_sym();
        MulExp();
        next_sym();
    }
    retract(); // not '+' or '-', retract
    output("<AddExp>");
}

// <MulExp>::= <UnaryExp> | <MulExp> (* / %) <UnaryExp>
// <MulExp>::= <UnaryExp> {('*' | '/' | '%') <UnaryExp>}
// alert: left recurrence
// promise: already a token
void Parser::MulExp() {
    UnaryExp();
    next_sym();
    while (type_code_ == TypeCode::MULT ||
            type_code_ == TypeCode::DIV ||
            type_code_ == TypeCode::MOD) {
        next_sym();
        UnaryExp();
        next_sym();
    }
    // not *| / | %
    retract();
    output("<MulExp>");
}

// <UnaryExp>::= PrimaryExp |
//              Ident '(' [FuncRParams] ')' |
//              UnaryOp UnaryExp
// promise: already read a token
// note: 7 branches
void Parser::UnaryExp() {
    // ( Exp )
    if (type_code_ == TypeCode::LPARENT) {
        PrimaryExp();
    }
    else if (type_code_ == TypeCode::INTCON) {
        PrimaryExp();
    }
    else if (type_code_ == TypeCode::PLUS ||
                type_code_ == TypeCode::MINU ||
                type_code_ == TypeCode::NOT) {
        UnaryOp();
        next_sym();
        UnaryExp();
    }
    // Ident '(' [FuncRParams] ')'
    // Ident { '[' Exp ']' }
    else if (type_code_ == TypeCode::IDENFR) {
        next_sym();
        if (type_code_ == TypeCode::LPARENT) {
            // now sure that: Ident '(' [FuncRParams] ')'
            next_sym();
            if (type_code_ == TypeCode::RPARENT) {
                // END
            } else {
                FuncRParams();
                next_sym();
                if (type_code_ == TypeCode::RPARENT) {
                    // pass
                } else {
                    handle_error("expect ')' in <UnaryExp>");
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
// promise: already a token
void Parser::PrimaryExp() {
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
void Parser::FuncRParams() {
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
void Parser::UnaryOp() {
    if (type_code_ == TypeCode::PLUS ||
        type_code_ == TypeCode::MINU ||
        type_code_ == TypeCode::NOT) {
        // pass
    } else {
        handle_error("expect + - ! in <UnaryOp>");
    }
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
    } else {
        handle_error("expect int in VarDecl begin");
    }
}

void Parser::VarDef() {

}

void Parser::InitVal() {

}


void Parser::FuncDef() {

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

void Parser::Block() {

}

void Parser::BlockItem() {

}

void Parser::Stmt() {

}
