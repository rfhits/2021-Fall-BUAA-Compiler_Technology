#include <iostream>
#include <fstream>
#include "ErrorHandler.h"
#include "Lexer.h"
#include "Parser.h"
#include "MipsGenerator.h"

int main() {
    std::string testfile_path = "testfile.txt";
    std::string interm_output_path = "output.txt";
    std::string mips_output_path = "mips.txt";
    std::string error_out_path = "error.txt";

    std::vector<std::string> output_str;

    std::ifstream in_stream(testfile_path);
    std::stringstream ss;
    ss << in_stream.rdbuf();
    in_stream.close();
    std::ofstream interm_out(interm_output_path);
    std::ofstream mips_out(mips_output_path);
    std::ofstream error_out(error_out_path);
    ErrorHandler error_handler(error_out);
    Lexer lexer(ss.str(), error_handler);
    lexer.uncomment();

    SymbolTable symbol_table;
    Intermediate interm(symbol_table, interm_out);
    Parser parser(symbol_table, lexer, error_handler, interm, false, interm_out);
    MipsGenerator mips_generator(symbol_table, interm.interm_codes_, mips_out);
    parser.Program();

    symbol_table.show_table();
//    interm.codes_to_string();
    mips_generator.translate();

    interm_out.close();
    error_out.close();
    mips_out.close();
    return 0;
}
