#include <iostream>
#include <fstream>
#include "ErrorHandler.h"
#include "Lexer.h"
#include "Parser.h"

int main() {
    string testfile_path = "testfile.txt";
    string output_path = "output.txt";
    string error_out_path = "error.txt";
    vector<string> output_str;

    ifstream in_stream(testfile_path);
    stringstream ss;
    ss << in_stream.rdbuf();
    in_stream.close();
    ofstream out(output_path);
    ofstream error_out(error_out_path);

    ErrorHandler error_handler(error_out);

    Lexer lexer(ss.str(), error_handler, true);
    lexer.uncomment();

    Parser parser(lexer, error_handler, true, output_str);
    parser.Program();

    out.close();
    error_out.close();
    return 0;
}
