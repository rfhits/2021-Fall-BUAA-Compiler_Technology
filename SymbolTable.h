//
// Created by WYSJ6174 on 2021/10/3.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>
#include "ErrorHandler.h"

enum class DataType {
    INT,
    INT_ARR,
    VOID,
    INVALID
};

enum class SymbolType {CONST, VAR, FUNC, PARAM};

struct TableEntry {
    SymbolType symbol_type;
    DataType data_type;
    std::string name;
    int value; // const: its int value, func: its param number
    int dims, dim0_size, dim1_size;
    int addr;
    int level;
    std::vector<int> array_values; // array const
};

class SymbolTable {
private:
    std::vector<TableEntry> global_table_;
    std::unordered_map<std::string, std::vector<TableEntry>> func_table_;
public:
    SymbolTable();

    std::pair<bool, TableEntry*> SearchFunc(const std::string& func_name);
    std::pair<bool, TableEntry*> SearchSymbolInLevel(const std::string& func_name, int level, const std::string& sym_name);
    std::pair<bool, TableEntry*> SearchNearestSymbolNotFunc(const std::string& func_name, const std::string& name);

    bool AddFunc(DataType data_type,const std::string& func_name, int value);

    bool AddSymbol(const std::string& func_name, DataType data_type, SymbolType sym_type,
                   const std::string& name, int value, int level, int dims, int dim0_size, int dim1_size);

    bool AddConstArray(const std::string& name, int dim0, int dim1, std::vector<int> array_values);



    void PopLevel(const std::string& func_name, int level);

    TableEntry* GetKthParam(const std::string& func_name, int k);

};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H
