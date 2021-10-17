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
    CHAR,
    VOID,
    NUM, // for ConstExpr to parse
    INVALID};

enum class SymbolType {CONST, VAR, FUNC, PARAM};

struct TableEntry {
    SymbolType sym_type;
    DataType data_type;
    std::string name;
    int value;
    int dim0, dim1;
    int addr;
    int level;
    bool visible;
};

class SymbolTable {
private:
    std::unordered_map<std::string, TableEntry> global_table_;
    std::unordered_map<std::string, std::vector<TableEntry>> func_table_;
public:
    SymbolTable();

    std::pair<bool, TableEntry*> SearchVisibleSymbol(const std::string& func_name, const std::string& name, int level);

    bool AddSymbol(const std::string& func_name, DataType data_type, SymbolType sym_type,
                   const std::string& name, int value, int level=0, int dim0=0, int dim1=0);

    void SetVisible(const std::string& func_name, int level, bool visible);

    TableEntry* GetKthParam(const std::string& func_name, int k);

};


#endif //INC_2021_FALL_BUAA_COMPILER_TECHNOLOGY_SYMBOLTABLE_H
