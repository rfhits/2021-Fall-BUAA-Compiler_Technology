#include <iostream>
#include <fstream>
#include "ErrorHandler.h"
#include "Lexer.h"
#include "Parser.h"

int main() {
    std::string testfile_path = "testfile.txt";
    std::string output_path = "output.txt";
    std::string error_out_path = "error.txt";
    std::vector<std::string> output_str;

    std::ifstream in_stream(testfile_path);
    std::stringstream ss;
    ss << in_stream.rdbuf();
    in_stream.close();
    std::ofstream out(output_path);
    std::ofstream error_out(error_out_path);
    ErrorHandler error_handler(error_out);
//    ErrorHandler error_handler(std::cerr);
    Lexer lexer(ss.str(), error_handler);
    lexer.uncomment();

    SymbolTable symbol_table;
    Intermediate interm(symbol_table, out);
    Parser parser( symbol_table,lexer,error_handler, interm, true, out);
    parser.Program();

    out.close();
    error_out.close();
    return 0;
}
