//
// Created by WYSJ6174 on 2021/9/24.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_ERRORHANDLER_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_ERRORHANDLER_H

#include <iostream>
#include <fstream>


class ErrorHandler {
private:
    std::ofstream& out_;

public:

    explicit ErrorHandler(std::ofstream &out): out_(out){}

    // log error to out stream with a line number
    void log_error_with_line_no(int line_no, const std::string& msg) {
        std::string log_msg;
        log_msg += std::to_string(line_no);
        log_msg += " ";
        log_msg += msg;
        out_ << log_msg << std::endl;
    }

    // @brief: output a message to error stream
    void log_error(const std::string& msg) {
        out_ << msg << std::endl;
    }
};

#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_ERRORHANDLER_H
