//
// Created by WYSJ6174 on 2021/10/3.
//

#ifndef INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H
#define INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

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
    int value;
    int dims, dim0_size, dim1_size;
    int addr;
    int level;
    std::vector<int> array_values;
};

class SymbolTable {
private:
    std::unordered_map<std::string, TableEntry> global_table_;
    std::unordered_map<std::string, std::vector<TableEntry>> func_table_;
public:
    SymbolTable();

    std::pair<bool, TableEntry*> SearchSymbol(const std::string& func_name, const std::string& name, int level);

    bool AddSymbol(const std::string& func_name, DataType data_type, SymbolType sym_type,
                   const std::string& name, int value, int level=0, int dims=0, int dim0_size=0, int dim1_size=0);

    bool AddConstArray(const std::string& name, int dim0, int dim1, std::vector<int> array_values);

    void PopLevel(const std::string& func_name, int level, bool visible);

    TableEntry* GetKthParam(const std::string& func_name, int k);

};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H
