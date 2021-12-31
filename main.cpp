#include <iostream>
#include <fstream>
#include "ErrorHandler.h"
#include "Lexer.h"
#include "Parser.h"
#include "MipsGenerator.h"

int main() {
    std::string testfile_path = "testfile.txt";
    std::string interm_output_path = "interm.txt";
    std::string interm_opt_output_path = "interm_opt.txt";
    std::string basic_blocks_output_path = "basic_blocks.txt";
    std::string mips_output_path = "mips.txt";
    std::string error_out_path = "error.txt";

    std::vector<std::string> output_str;

    // read from testfile
    std::ifstream in_stream(testfile_path);
    std::stringstream ss;
    ss << in_stream.rdbuf();
    in_stream.close();

    // init output stream
    std::ofstream interm_out(interm_output_path);
    std::ofstream interm_opt_out(interm_opt_output_path);
    std::ofstream mips_out(mips_output_path);
    std::ofstream error_out(error_out_path);
    ErrorHandler error_handler(error_out);
    Lexer lexer(ss.str(), error_handler);
    lexer.uncomment();

    SymbolTable symbol_table;
    Intermediate interm(symbol_table, interm_out);
    Parser parser(symbol_table, lexer, error_handler, interm, false, interm_out);
    MipsGenerator mips_generator(symbol_table, interm.codes_, interm.func_blocks_, interm.basic_blocks_, mips_out);
    parser.Program();
    symbol_table.show_table();
    interm.OutputCodes(interm_out);

    interm.Optimize();
    interm.OutputCodes(interm_opt_out);
    symbol_table.show_table();

    mips_generator.Translate();

    interm_out.close();
    interm_opt_out.close();
    error_out.close();
    mips_out.close();
    return 0;
}
