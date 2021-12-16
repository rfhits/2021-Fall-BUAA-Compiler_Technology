//
// Created by WYSJ6174 on 2021/9/25.
//

#include "utils.h"

// whether a string can be parsed as an integer
bool is_integer(std::string &s) {
    bool can_be_parsed = false;
    try {
        std::stoi(s);
        can_be_parsed = true;
    } catch (std::invalid_argument &e) {
        can_be_parsed = false;
    }
    return can_be_parsed;
}

bool is_unsigned_integer(std::string &s) {
    bool flag = true;
    for (char i: s)//length是string类的一个成员函数返回字符串的长度
    {
        if (!isdigit(i)) {
            flag = false;
            break;
        }
    }
    return flag;//返回flag，如果字符串为数字则flag为true，否则为false
}

std::string &str_trim(std::string &str) {
    if (str.empty()) {
        return str;
    }
    str.erase(0, str.find_first_not_of(' '));
    str.erase(str.find_last_not_of(' ') + 1);
    return str;
}

// {1, +2, -3}
std::vector<int> str_to_vec_int(std::string str) {
    std::vector<int> ret_vec;
    int begin = 1;
    int end = 1;
    std::string tmp_value;
    while (end < str.length()) {
        if (str[begin] == '}') {
            break;
        }
        if (str[end] == ',') {
            tmp_value = str.substr(begin, end - begin);
            ret_vec.push_back(std::stoi(tmp_value));
            begin = end + 1;
            end += 1;
        } else if (str[end] == '}') {
            tmp_value = str.substr(begin, end - begin);
            ret_vec.push_back(std::stoi(tmp_value));
            break;
        } else {
            end += 1;
        }
    }
    return ret_vec;
}

std::string vec_int_to_str(std::vector<int> vec) {
    std::string str = "{";
    int i = 0;
    if (vec.empty()) {
        // pass
    } else {
        for (; i < vec.size() - 1; i++) {
            str += std::to_string(vec[i]);
            str += ",";
        }
        str += std::to_string(vec[i]);
    }
    str += '}';
    return str;
}

// param str: "{var1,var2,1,3}"
std::vector<std::string> str_to_vec_str(const std::string &str) {
    std::vector<std::string> ret_vec;
    int begin = 1;
    int end = 1;
    std::string tmp_value;
    while (end < str.length()) {
        if (str[begin] == '}') {
            break;
        }
        if (str[end] == ',') {
            tmp_value = str.substr(begin, end - begin);
            str_trim(tmp_value);
            ret_vec.push_back(tmp_value);
            begin = end + 1;
            end += 1;
        } else if (str[end] == '}') {
            tmp_value = str.substr(begin, end - begin);
            str_trim(tmp_value);
            ret_vec.push_back(tmp_value);
            break;
        } else {
            end += 1;
        }
    }
    return ret_vec;
}

std::string vec_str_to_str(const std::vector<std::string> &vec) {
    std::string str = "{";
    int i = 0;
    if (vec.empty()) {
        // pass
    } else {
        for (; i < vec.size() - 1; i++) {
            str += vec[i];
            str += ",";
        }
        str += vec[i];
    }
    str += "}";
    return str;
}

// 字符串中子串出现次数
int get_substr_no(const std::string &str, const std::string &sub) {
    int index = 0;    //下标
    int count = 0;    //次数

    //b.find(a);这句代码的意思就是从b字符串中查找a字符串
    //返回值的类型为int类型，返回的是字符串的下标
    //如果没找到，返回一个特别的标志c++中用npos表示，string::npos很大的一个数，转成int值是-1
    //

    while ((index = str.find(sub, index)) < str.length()) {
        count++;
        index++;
    }
    return count;
}

int sum(const std::vector<int> &vec_int) {
    int res = 0;
    for (int i: vec_int) {
        res += i;
    }
    return res;
}

bool str_set_equal(std::set<std::string> &set1, std::set<std::string> &set2) {
    auto it = set1.begin();
    while (it != set1.end()) {
        if (set2.find(*it) == set2.end()) {
            return false;
        } else {
            it++;
            continue;
        }
    }

    it = set2.begin();
    while (it != set2.end()) {
        if (set1.find(*it) == set1.end()) {
            return false;
        } else {
            it++;
            continue;
        }
    }
    return true;
}


// @brief: res_set = original_set - input_set
// @attention: if we use "&" to pass the parameter, the res_set may same as the original set or input set
void str_set_diff(std::set<std::string> &res_set, std::set<std::string> original_set, std::set<std::string> input_set) {
    res_set.clear();
    res_set.insert(original_set.begin(), original_set.end());
    for (const auto &it: input_set) {
        res_set.erase(it);
    }
}

// 2, 4, 8, 16, 32
bool is_2_pow(int a) {
    if (a < 0) return false;
    if (__builtin_popcount(a) == 1) {
        return true;
    } else {
        return false;
    }
}

// @pre: the input is 2 power
int get_2_pow(int a) {
    int b = 2;
    for (int i = 1; i < 32; i++) {
        if (a == b) {
            return i;
        } else {
            b = b << 1;
        }
    }
    return -1;
}

bool can_be_div_opt(int d) {
    // 乘除优化开关
//    return false;


    if (is_2_pow(d)) return true;

    if (d == 3 || d == 5 || d == 6 || d == 9 || d == 10 || d == 11 || d == 12 || d == 25 || d == 125 ||
        d == 625) {
        return true;
    } else {
        return false;
    }
}


// @pre: the d "can be div opt"
std::pair<unsigned int, unsigned int> get_multer_and_shifter(int d) {
    unsigned int multer, shifter;
//    if (is_2_pow(d)) {
//        multer = 1;
//        shifter = get_2_pow(d);
//    } else

    if (d == 3) {
        multer = 0x55555556;
        shifter = 0;
    } else if (d == 5) {
        multer = 0x66666667;
        shifter = 1;
    } else if (d == 6) {
        multer = 0x2AAAAAAB;
        shifter = 0;
    }

//    else if (d == 7) {
//        multer = 0x92492493;
//        shifter = 2;
//    }

    else if (d == 9) {
        multer = 0x38E38E39;
        shifter = 1;
    } else if (d == 10) {
        multer = 0x66666667;
        shifter = 2;
    } else if (d == 11) {
        multer = 0x2E8BA2E9;
        shifter = 1;
    } else if (d == 12) {
        multer = 0x2AAAAAAB;
        shifter = 1;
    } else if (d == 25) {
        multer = 0x51EB851F;
        shifter = 3;
    } else if (d == 125) {
        multer = 0x10624DD3;
        shifter = 3;
    } else if (d == 625) {
        multer = 0x68DB8BAD;
        shifter = 8;
    }
    return std::make_pair(multer, shifter);

}