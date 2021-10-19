//
// Created by WYSJ6174 on 2021/10/3.
//

#include "SymbolTable.h"

SymbolTable::SymbolTable() = default;

// search a visible symbol
//
// in def stmt, should check if exists
// in assign stmt, should check if
std::pair<bool, TableEntry*> SymbolTable::SearchSymbol(
        const std::string &func_name, const std::string &name, int level) {
    if (func_name.empty()) {
        // search in the global table
        auto it = global_table_.find(name);
        if (it != global_table_.end()) {
            return std::make_pair(true, &(it->second));
        } else {
            return std::make_pair(false, nullptr);
        }
    }
    else {
        auto it = func_table_.find(func_name);
        if (it != func_table_.end()) {
            // if function exists
            std::vector<TableEntry>& table = it->second;
            for (auto & i : table) {
                // search in this level of the vector
                if (i.level == level) {
                    return std::make_pair(true, &i);
                } else {
                    continue;
                }
            }
            return std::make_pair(false, nullptr);
        } else {
            return std::make_pair(false, nullptr);
        }
    }
}


// add a symbol entry
// if func_name is empty, add to global
// else add to the func
// promise: the func_name exists
bool SymbolTable::AddSymbol(const std::string &func_name, DataType data_type, SymbolType sym_type,
                            const std::string &name, int value, int level, int dims, int dim0_size, int dim1_size) {
    TableEntry table_entry;
    table_entry.data_type = data_type;
    table_entry.symbol_type = sym_type;
    table_entry.name = name;
    table_entry.value = value;
    table_entry.level = level;
    table_entry.dim0_size = dim0_size;
    table_entry.dim1_size = dim1_size;
    if (func_name.empty()) {
        // add to global
        if (global_table_.count(name) != 0) {
            return false;
        } else {
            global_table_[name] = table_entry;
            if (sym_type==SymbolType::FUNC) {
                func_table_[name] = std::vector<TableEntry>();
            }
            return true;
        }
    }
    else {
        // search in the table of func table
        // if in the same level and visible, return false
        std::pair<bool, TableEntry*> res = SearchSymbol(func_name, name, level);
        if (res.first) {
            // find a same name symbol in the same level
            return false;
        } else {
            // add to the func table
            func_table_[func_name].push_back(table_entry);
            return true;
        }
    }
}

// after passing a block in function,
// need to set the var in the block to invisible
// promise: the func_table exists
void SymbolTable::PopLevel(const std::string& func_name, int level, bool visible) {
    std::vector<TableEntry>& entry_table = func_table_[func_name];
    auto it = entry_table.begin();
    while (it != entry_table.end()) {
        if (it->level == level) {
            it = entry_table.erase(it);
        } else {
            it++;
        }
    }
}

// count from 0
TableEntry* SymbolTable::GetKthParam(const std::string& func_name, int k) {
    for (auto &item : func_table_[func_name]) {
        if (item.symbol_type == SymbolType::PARAM && item.value == k) {
            return &item;
        }
    }
    return nullptr;
}

// add a const array into global table
// if success, return true
bool SymbolTable::AddConstArray(const std::string &name, int dim0, int dim1,
                                std::vector<int> array_values) {
    if (global_table_.count(name) != 0) {
        return false;
    } else {
        TableEntry table_entry;
        table_entry.symbol_type = SymbolType::CONST;
        table_entry.data_type = DataType::INT_ARR;
        table_entry.name = name;
        table_entry.dim0_size = dim0;
        table_entry.dim1_size = dim1;
        table_entry.level = 0;
        table_entry.array_values = std::move(array_values);
        global_table_[name] = table_entry;
        return true;
    }
}
