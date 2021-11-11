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
    } catch (std::invalid_argument& e) {
        can_be_parsed = false;
    }
    return can_be_parsed;
}

bool is_unsigned_integer(std::string &s) {
    bool flag = true;
    for (char i : s)//length是string类的一个成员函数返回字符串的长度
    {
        if (!isdigit(i))
        {
            flag = false;
            break;
        }
    }
    return flag;//返回flag，如果字符串为数字则flag为true，否则为false
}

std::string& str_trim(std::string &str)
{
    if (str.empty()){
        return str;
    }
    str.erase(0, str.find_first_not_of(' '));
    str.erase(str.find_last_not_of(' ') + 1);
    return str;
}

// {1, +2, -3   }
std::vector<int> str_to_vec_int(std::string str) {
    std::vector<int> ret_vec;
    int begin = 1;
    int end = 1;
    std::string tmp_value;
    while ( end < str.length()) {
        if (str[begin] == '}') {
            break;
        }
        if (str[end] == ',') {
            tmp_value = str.substr(begin, end-begin);
            ret_vec.push_back(std::stoi(tmp_value));
            begin = end + 1;
            end +=1 ;
        } else if (str[end] == '}') {
            tmp_value = str.substr(begin, end-begin);
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
        for (; i < vec.size()-1; i++) {
            str += std::to_string(vec[i]);
            str += ",";
        }
        str += std::to_string(vec[i]);
    }
    str += '}';
    return str;
}

// param str: "{var1,var2,1,3}"
std::vector<std::string> str_to_vec_str(const std::string& str) {
    std::vector<std::string> ret_vec;
    int begin = 1;
    int end = 1;
    std::string tmp_value;
    while (end < str.length()) {
        if (str[begin] == '}') {
            break;
        }
        if (str[end] == ',') {
            tmp_value = str.substr(begin, end-begin);
            str_trim(tmp_value);
            ret_vec.push_back(tmp_value);
            begin = end + 1;
            end +=1 ;
        } else if (str[end] == '}') {
            tmp_value = str.substr(begin, end-begin);
            str_trim(tmp_value);
            ret_vec.push_back(tmp_value);
            break;
        } else {
            end += 1;
        }
    }
    return ret_vec;
}

std::string vec_str_to_str(const std::vector<std::string>& vec) {
    std::string str = "{";
    int i = 0;
    if (vec.empty()) {
        // pass
    } else {
        for (; i < vec.size()-1; i++) {
            str += vec[i];
            str += ",";
        }
        str += vec[i];
    }
    str += "}";
    return str;
}

//字符串中子串出现次数
int get_substr_no(const std::string& str, const std::string& sub) {
    int index = 0;	//下标
    int count = 0;	//次数

    //b.find(a);这句代码的意思就是从b字符串中查找a字符串
    //返回值的类型为int类型，返回的是字符串的下标
    //如果没找到，返回一个特别的标志c++中用npos表示，string::npos很大的一个数，转成int值是-1
    //

    while( (index=str.find(sub,index)) < str.length() ){
        count++;
        index++;
    }
    return count;
}

int sum(const std::vector<int>& vec_int) {
    int res = 0;
    for (int i : vec_int) {
        res += i;
    }
    return res;
}

void assert(const std::string& msg){
    std::cout << msg << std::endl;
}