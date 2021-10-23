//
// Created by WYSJ6174 on 2021/9/25.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_UTILS_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_UTILS_H
#include <iostream>
#include <utility>
#include <vector>
#include <map>
#include <algorithm>

bool is_unsigned_integer(std::string& s);
bool is_integer(std::string &s);
std::string& str_trim(std::string& str);
std::vector<int> str_to_vec_int(std::string str);
std::string vec_int_to_str(std::vector<int> vec);
std::vector<std::string> str_to_vec_str(const std::string& str);
std::string vec_str_to_str(const std::vector<std::string>& vec);
int get_substr_no(const std::string& str, const std::string& sub);
#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_UTILS_H
