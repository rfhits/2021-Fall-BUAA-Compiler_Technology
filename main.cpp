#include <iostream>
#include <fstream>
#include "ErrorHandler.h"
#include "Lexer.h"

using namespace std;

int main() {
    string testfile_path = "testfile.txt";
    string output_path = "output.txt";
    string error_out_path = "error.txt";
    ifstream in_stream(testfile_path);
    stringstream ss;
    ss << in_stream.rdbuf();
    in_stream.close();
    ofstream out(output_path);
    ofstream error_out(error_out_path);

    ErrorHandler error_handler(error_out);

    Lexer l = Lexer(ss.str(), error_handler, true, out);
    l.uncomment();
    while (true) {
        Token token = l.get_token();
        if (token.get_type_code() != TYPE_EOF) {
            continue;
        } else {
            break;
        }
    }
    out.close();
    error_out.close();
    return 0;
}
