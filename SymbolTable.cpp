//
// Created by WYSJ6174 on 2021/10/3.
//

#include "SymbolTable.h"

SymbolTable::SymbolTable() = default;

// search a visible symbol
//
// in def stmt, should check if exists
// in assign stmt, should check if
std::pair<bool, TableEntry*> SymbolTable::SearchVisibleSymbol(
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
                if (i.level == level && i.visible) {
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
                            const std::string &name, int value, int level, int dim0, int dim1) {
    TableEntry table_entry;
    table_entry.data_type = data_type;
    table_entry.sym_type = sym_type;
    table_entry.name = name;
    table_entry.value = value;
    table_entry.level = level;
    table_entry.dim0 = dim0;
    table_entry.dim1 = dim1;
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
        std::pair<bool, TableEntry*> res = SearchVisibleSymbol(func_name, name, level);
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
void SymbolTable::SetVisible(const std::string& func_name, int level, bool visible) {
    std::vector<TableEntry>& entry_table = func_table_[func_name];
    for (auto & item : entry_table) {
        if (item.level == level) {
            item.visible = false;
        }
    }
}

TableEntry *SymbolTable::GetKthParam(const std::string& func_name, int k) {
    for (auto &item : func_table_[func_name]) {
        if (item.sym_type == SymbolType::PARAM && item.value == k) {
            return &item;
        }
    }
    return nullptr;
}
