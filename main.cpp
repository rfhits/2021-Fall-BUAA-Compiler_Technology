#include <iostream>
#include "Lexer.h"
using namespace std;
int main() {
    string testfile = "testfile.txt";
    string output = "output.txt";

    Lexer l = Lexer(testfile);
    ofstream out(output);
    l.pre_treat();
    while (true) {
        Token token = l.get_token();
        if (token.type != INVALID_TYPE) {
            out << token.type << " " << token.str_value <<endl;
        } else {
            break;
        }
    }
    out.close();
    return 0;
}
