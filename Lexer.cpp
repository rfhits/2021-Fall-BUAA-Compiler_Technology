//
// Created by WYSJ6174 on 2021/9/24.
//

#include "Lexer.h"
#include "Error.h"
map<string, string> Lexer::token_to_reserved_type {
        {"main", "MAINTK"},
        {"const", "CONSTTK"},
        {"int", "INTTK"},
        {"break", "BREAKTK"},
        {"continue", "CONTINUETK"},
        {"if", "IFTK"},
        {"else", "ELSETK"},
        {"while", "WHILETK"},
        {"getint", "GETINTTK"},
        {"printf", "PRINTFTK"},
        {"return", "RETURNTK"},
        {"void", "VOIDTK"},
};

map<string, string> Lexer::token_to_operation_type {
        {"!", "NOT"},
        {"&&", "AND"},
        {"||", "OR"},
        {"+", "PLUS"},
        {"-", "MINU"},
        {"*", "MULT"},
        {"/", "DIV"},
        {"%", "MOD"},
        {"<", "LSS"},
        {"<=", "LEQ"},
        {">", "GRE"},
        {">=", "GEQ"},
        {"==", "EQL"},
        {"!=","NEQ"},
        {"=", "ASSIGN"},
        {";", "SEMICN"},
        {",", "COMMA"},
        {"(", "LPARENT"},
        {")", "RPARENT"},
        {"[", "LBRACK"},
        {"]", "RBRACK"},
        {"{", "LBRACE"},
        {"}", "RBRACE"},
};

// replace comment to ' '
void Lexer::pre_treat() {

    // Error treat, may illegal comment
    for (int i = 0; i<source.length(); ) {
        if (source[i] == '"') {
            do {
                i += 1;
            } while(source[i] != '"');
            i+=1;
        }
        else if (source[i] == '/' && source[i+1] == '/') {
            source[i] = source[i+1] = ' ';
            i += 2; // the char after "//"
            while (source[i] != '\n') {
                source[i] = ' ';
                i+=1;
            }
            // now source[i] is \n
            i+=1;
        }
        else if (source[i] == '/' && source[i+1] == '*') {
            source[i] = source[i+1] = ' ';
            i += 2; // at the char after "/*"
            while (source[i] != '*' || source[i+1] != '/') {
                source[i] = (source[i] == '\n')? '\n':' ';
                i+=1;
            }
            // now source[i] is at "*/"
            source[i] = source[i+1] = ' ';
            i += 2;
        }
        else {
            i += 1;
        }
    }
}

// read a char from source and store it to ch
// maintain the line_no, col_num, pos.
int Lexer::get_char() {
    if (pos >= source.length()) {
        throw exception();
    }
    ch = source[pos++];
    if (ch == '\n') {
        line_no += 1;
        col_no = 1;
    } else {
        col_no += 1;
    }
    return 0;
}

void Lexer::retract() {
    if (ch == '\n') {
        line_no -= 1;
    }
    col_no -= 1;
    pos -= 1;
    ch = source[pos-1];
}

// put a word into Lexer::token
// if token.type == comment, pass it, meaningless
// if token.type == INVALID_TYPE
// if token.type ==
Token Lexer::get_token() {
    Token r_token(INVALID_TYPE, INVALID_STRING_VALUE, line_no, col_no);
    token.clear();
    // read till there is a none-blank char in ch
    get_char();

    while (isspace(ch) && pos < source.length()) {
        get_char();
    }
    if (ch == '\n') {
        return r_token;
    }

    string type; // gonna to Token


    // identifier begins with
    if (isalpha(ch) || ch == '_') {
        do {
            token += ch;
            get_char();
        } while(isalnum(ch) || ch =='_');
        retract();
        auto iter = token_to_reserved_type.find(token);
        if (iter == token_to_reserved_type.end()) { // not a reserved type
            type = "IDENFR";
        } else { // found, is a reserved type
            type = iter->second;
        }
    } else if (isdigit(ch)) {
        do {
            token += ch;
            get_char();
        } while (isdigit(ch));
        retract();
        type = "INTCON";
    } else if (ch == '\"') {
        do {
            token += ch;
            get_char();
        } while (ch != '\"');
        token += ch;
        type = "STRCON";
    }
    // we will begin pre-read here
    // !, !=; &&; ||, <, <=; >, >=; =, ==;
    else if (ch == '!') {
        get_char();
        if (ch != '=') {
            token = "!";
            type = "NOT";
            retract();
        } else {
            token = "!=";
            type = "NEQ";
        }
    }
    else if (ch == '&') {
        get_char();
        if (ch == '&') {
            token = "&&";
            type = "AND";
        } else {
            Error e("expect & after &");
            e.alert();
        }
    }
    else if (ch == '|') {
        get_char();
        if (ch == '|') {
            type = "OR";
            token = "||";
        } else {
            Error e("expect | after |");
            e.alert();
        }
    }
    else if (ch == '<') {
        get_char();
        if (ch != '=') {
            retract();
            type = "LSS";
            token = "<";
        } else {
            type = "LEQ";
            token = "<=";
        }
    }
    else if (ch == '>') {
        get_char();
        if (ch != '=') {
            retract();
            type = "GRE";
            token = ">";
        }
        else {
            type = "GEQ";
            token = ">=";
        }
    }
    else if (ch == '=') {
        get_char();
        if (ch != '=') {
            retract();
            type = "ASSIGN";
            token = "=";
        }
        else {
            type = "EQL";
            token = "==";
        }
    }
    else if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%'
        || ch == ';' || ch == ',' || ch == '(' || ch == ')' || ch == '['
        || ch == ']' || ch == '{' || ch == '}') {
        auto iter = token_to_operation_type.find(string(1,ch));
        if (iter != token_to_operation_type.end()) {
            type = iter->second;
            token = string(1, ch);
        }
        else {
            Error e("token to reserved operations match error");
            e.alert();
        }
    }
    else {
        Error e("ch can't match any char ");
        e.alert();
    }
    r_token.type = std::move(type);
    r_token.str_value = token;
    return r_token;
}

