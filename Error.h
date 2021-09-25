//
// Created by WYSJ6174 on 2021/9/24.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_ERROR_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_ERROR_H

#include <iostream>
using namespace std;
class Error {
public:
    string msg_;
    Error(string msg): msg_(std::move(msg)){};
    void alert();
};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_ERROR_H
