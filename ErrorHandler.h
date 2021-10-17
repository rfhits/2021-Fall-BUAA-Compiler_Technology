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
    void log_error(int line_no, const std::string& msg) {
        out_ << "error at " << line_no << " line: ";
        out_ << msg << std::endl;
    }

    // log error to out stream with a line number
    void log_error(const std::string& msg) {
        out_ << msg << std::endl;
    }
};

#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_ERRORHANDLER_H
