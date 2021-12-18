//
// Created by WYSJ6174 on 2021/10/3.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include <map>
#include "ErrorHandler.h"
#include "utils.h"

enum class DataType {
    INVALID,
    INT,
    INT_ARR,
    VOID
};

enum class SymbolType {CONST, VAR, FUNC, PARAM};

const std::map<DataType, std::string> data_type_to_str = {
        {DataType::INT, "int"},
        {DataType::INT_ARR, "int_arr"},
        {DataType::VOID, "void"},
        {DataType::INVALID, "invalid"}
};


const std::map<SymbolType, std::string> symbol_type_to_str = {
        {SymbolType::CONST, "const"},
        {SymbolType::VAR, "var"},
        {SymbolType::FUNC, "func"},
        {SymbolType::PARAM, "param"}
};

struct TableEntry {
    SymbolType symbol_type;
    DataType data_type;
    std::string name;
    std::string alias;
    int value; // const: its int value, func: its param number
    int dims, dim0_size, dim1_size;
    int level;
    unsigned int addr;
    int size;
    bool is_recur_func = false;
    std::vector<int> array_values; // array const
    TableEntry(){};
    TableEntry(TableEntry* entry_ptr) {
        symbol_type = entry_ptr->symbol_type;
        data_type = entry_ptr->data_type;
        name = entry_ptr->name;
        alias = entry_ptr->alias;
        value = entry_ptr->value;
        dims = entry_ptr->dims;
        dim0_size = entry_ptr->dim0_size;
        dim1_size = entry_ptr->dim1_size;
        level = entry_ptr->level;
        addr = entry_ptr->addr;
        size = entry_ptr->size;
        array_values = entry_ptr->array_values;
    };
};



class SymbolTable {
public:
    std::vector<TableEntry> global_table_;
    std::unordered_map<std::string, std::vector<TableEntry>> func_tables_;
    std::vector<std::string> strcons_;

    SymbolTable();

    bool is_global_symbol(const std::string& sym_name);

    std::pair<bool, TableEntry*> SearchFunc(const std::string& func_name);
    std::pair<bool, TableEntry*> SearchSymbolInLevel(const std::string& func_name, int level, const std::string& sym_name);
    std::pair<bool, TableEntry*> SearchNearestSymbolNotFunc(const std::string& func_name, const std::string& name);

    std::string entry_to_string(TableEntry* entry);
    void show_table();

    bool AddFunc(DataType data_type,const std::string& func_name, int value);

    void SetRecurFunc(const std::string &func_name);

    bool AddSymbol(const std::string& func_name, DataType data_type, SymbolType sym_type,
                   const std::string& name, const std::string& alias,
                   int value, int level, int dims, int dim0_size, int dim1_size, unsigned int addr);

    bool AddConstArray(const std::string& func_name, const std::string& name, const std::string& alias,
                       int level, int dims, int dim0, int dim1, std::vector<int> array_values, unsigned int addr);

    void PopLevel(const std::string& func_name, int level);

    TableEntry* GetKthParam(const std::string& func_name, int k);

    int get_global_data_size();

    std::vector<std::string> GetFuncParams(std::string func_name);

    int GetFuncStackSize(const std::string& func_name);

    void add_to_strcons(const std::string& str);

    int find_str_idx(const std::string& str);

    void add_error(const std::string& msg);

};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H
