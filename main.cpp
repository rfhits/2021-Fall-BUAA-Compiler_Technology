#include <iostream>
#include <fstream>
#include "Lexer.h"
using namespace std;

int main() {
    string testfile_path = "testfile.txt";
    string output_path = "output.txt";
    ifstream in_stream(testfile_path);
    stringstream ss;
    ss << in_stream.rdbuf();
    in_stream.close();

    Lexer l = Lexer(ss.str());
    l.uncomment();
    ofstream out(output_path);
    while (true) {
        Token token = l.get_token();
        if (token.get_type_code() != TYPE_EOF) {
            out << token.get_type_code() << " " << token.get_str_value() <<endl;
        } else {
            break;
        }
    }
    out.close();
    return 0;
}
