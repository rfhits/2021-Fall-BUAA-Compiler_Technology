//
// Created by WYSJ6174 on 2021/10/3.
//

#include "SymbolTable.h"

SymbolTable::SymbolTable() = default;

// @note: search a function in global table
// @attention: func name may same as the const or variable name
std::pair<bool, TableEntry *> SymbolTable::SearchFunc(const std::string &func_name) {
    for (auto &entry: global_table_) {
        if (entry.name == func_name && entry.symbol_type == SymbolType::FUNC) {
            return std::make_pair(true, &entry);
        } else {
            // pass
        }
    }
    return std::make_pair(false, nullptr);
}

// @brief: this function is called when defining a variable or constant,
//       check whether exists a same name const or variable in the same scope,
//       so it will search a symbol in a specific scope
// @attention: won't search the Function Name
// @param[in] func_name: search in  which function, if empty, in global
std::pair<bool, TableEntry *> SymbolTable::SearchSymbolInLevel(
        const std::string &func_name, int level, const std::string &sym_name) {
    if (func_name.empty()) { // it is defined in global
        for (auto &i: global_table_) {
            if (i.name == sym_name && (i.symbol_type != SymbolType::FUNC)) {
                return std::make_pair(true, &i);
            } else {
                // pass
            }
        }
    } else { // it is defined in a function
        auto it = func_tables_.find(func_name);
        if (it != func_tables_.end()) {
            std::vector<TableEntry> &table = it->second;
            for (auto &i: table) {
                if (i.name == sym_name && i.level == level) {
                    return std::make_pair(true, &i);
                } else {
                    // pass
                }
            }
        } else { // can't find this function
            return std::make_pair(false, nullptr);
        }
    }
    return std::make_pair(false, nullptr);
}

// @brief: try to search in the function table,
//         if you can't find, go back to search in global table
//
// @pre: the function is called when assigning or use a variable, search a symbol in the whole scope
// @attention: do not search in function
// @post: get the entry and judge if it is a const or if it is declared
std::pair<bool, TableEntry *> SymbolTable::SearchNearestSymbolNotFunc(
        const std::string &func_name, const std::string &name) {
    if (func_name.empty()) {
        // search in the global table
//        for (unsigned long long i = 0; i < global_table_.size(); i++) {
//            if (global_table_[i].name == name && (global_table_[i].symbol_type != SymbolType::FUNC)) {
//                return std::make_pair(true, &(global_table_[i]));
//            } else {
//                // pass
//            }
//        }
        for (auto &i : global_table_) {
            if (i.name == name && (i.symbol_type != SymbolType::FUNC)) {
                return std::make_pair(true, &i);
            }
        }

    } else {
        auto it = func_tables_.find(func_name);
        if (it != func_tables_.end()) {
            for(auto entry_it = it->second.rbegin(); entry_it != it->second.rend(); entry_it++) {
                if (entry_it->name == name) {
                    return std::make_pair(true, &(*entry_it));
                }
            }
//            for (long long i = it->second.size() - 1; i >= 0; i--) {
//                if (it->second[i].name == name) {
//                    return std::make_pair(true, &(it->second[i]));
//                } else {
//                    // pass
//                }
//            }

            return SearchSymbolInLevel("", 0, name);
        } else { // the function does not exist
            return std::make_pair(false, nullptr);
        }
    }
    return std::make_pair(false, nullptr);
}

// @brief: add a func symbol to global
// @retval: success?
bool SymbolTable::AddFunc(DataType data_type, const std::string &func_name, int value) {
    auto search_res = SearchFunc(func_name);
    if (search_res.first) { // already exists
        return false;
    } else {
        TableEntry entry;
        entry.name = func_name;
        entry.data_type = data_type;
        entry.symbol_type = SymbolType::FUNC;
        entry.value = value;
        entry.level = 0;
        global_table_.push_back(entry);
        std::vector<TableEntry> func_table;
        func_tables_[func_name] = func_table;
        return true;
    }
}

// @brief: change the symbol's name of the level to its alias
// @pre: this function is called after parsing a block inside function body
// @pre: the func_table exists
void SymbolTable::PopLevel(const std::string &func_name, int level) {
    if (func_name.empty()) {
        auto it = global_table_.begin();
        while (it != global_table_.end()) {
            if (it->symbol_type == SymbolType::CONST || it->symbol_type == SymbolType::VAR) {
                it->name = it->alias;
                it++;
            } else {
                it++;
            }
        }
    } else {
        std::vector<TableEntry> &entry_table = func_tables_[func_name];
        auto it = entry_table.begin();
        while (it != entry_table.end()) {
            if (it->level == level) {
//            it = entry_table.erase(it);
                it->name = it->alias;
                it++;
            } else {
                it++;
            }
        }
    }
}

// @attention: count from 0
TableEntry *SymbolTable::GetKthParam(const std::string &func_name, int k) {
    for (auto &item: func_tables_[func_name]) {
        if (item.symbol_type == SymbolType::PARAM && item.value == k) {
            return &item;
        }
    }
    return nullptr;
}

// add a const array into symbol_table
// if success, return true
bool SymbolTable::AddConstArray(const std::string &cur_func_name, const std::string &name, const std::string &alias,
                                int level, int dims, int dim0, int dim1, std::vector<int> array_values,
                                unsigned int addr) {
    if (SearchSymbolInLevel(cur_func_name, level, name).first) {
        return false;
    } else {
        TableEntry table_entry;
        table_entry.symbol_type = SymbolType::CONST;
        table_entry.data_type = DataType::INT_ARR;
        table_entry.name = name;
        table_entry.alias = alias;
        table_entry.dims = dims;
        table_entry.dim0_size = dim0;
        table_entry.dim1_size = dim1;
        table_entry.size = (dims == 1) ? 4 * dim0 : 4 * dim0 * dim1;

        table_entry.level = level;
        table_entry.array_values = std::move(array_values);
        table_entry.addr = addr;
        if (cur_func_name.empty()) {
            global_table_.push_back(table_entry);
        } else {
            auto &it = func_tables_[cur_func_name];
            it.push_back(table_entry);
        }
        return true;
    }
}

void SymbolTable::show_table() {
    std::cout << "symbol table" << std::endl;
    std::cout << "global_table: " << std::endl;
    std::cout << "sym_type | data_type | name | value | dims | level | size | addr" << std::endl;

    for (int i = 0; i < global_table_.size(); i++) {
        std::cout << entry_to_string(&global_table_[i]) << std::endl;
    }

    for (auto &i: func_tables_) {
        std::cout << i.first << std::endl;
        for (int j = 0; j < i.second.size(); j++) {
            std::cout << entry_to_string(&i.second[j]) << std::endl;
        }
    }
}

std::string SymbolTable::entry_to_string(TableEntry *entry) {
    std::string str;
    str += symbol_type_to_str.find(entry->symbol_type)->second; // symbol_type
    str += " | ";
    str += data_type_to_str.find(entry->data_type)->second; // data_type
    str += " | ";
    str += entry->name;
    str += " | ";
    str += std::to_string(entry->value);
    str += " | ";
    str += std::to_string(entry->dims);
    str += " | ";
    str += std::to_string(entry->level);
    str += " | ";
    str += std::to_string(entry->size);
    str += " | ";
    str += std::to_string(entry->addr);
    return str;
}


// @brief: called while defining a var or const,
//         if func_name is empty, add to global,
//         else add to the func
// @param[in] func_name: the scope to add the symbol
bool
SymbolTable::AddSymbol(const std::string &func_name, DataType data_type, SymbolType sym_type, const std::string &name,
                       const std::string &alias, int value, int level, int dims, int dim0_size, int dim1_size,
                       unsigned int addr) {
    if (sym_type == SymbolType::FUNC) {
        return AddFunc(data_type, func_name, value);
    } else { // add a variable or const
        if (SearchSymbolInLevel(func_name, level, name).first) { // already exists in the same scope
            return false;
        } else {
            TableEntry table_entry;
            table_entry.data_type = data_type;
            table_entry.symbol_type = sym_type;
            table_entry.name = name;
            table_entry.alias = alias;
            table_entry.value = value;
            table_entry.level = level;
            table_entry.dims = dims;
            table_entry.dim0_size = dim0_size;
            table_entry.dim1_size = dim1_size;
            table_entry.addr = addr;
            if (data_type == DataType::INT) {
                table_entry.size = 4;
            } else if (data_type == DataType::INT_ARR) {
                table_entry.size = (dims == 1) ? (4 * dim0_size) : (4 * dim0_size * dim1_size);
            } else {}

            if (func_name.empty()) {
                global_table_.push_back(table_entry);
            } else {
                auto &it = func_tables_[func_name];
                it.push_back(table_entry);
            }
            return true;
        }
    }
}


int SymbolTable::get_global_data_size() {
    int global_size = 0;
    for (auto &i : global_table_) {
        if (i.symbol_type == SymbolType::VAR || i.symbol_type == SymbolType::CONST) {
            global_size += i.size; // the param array is just an address
        }
    }
    return global_size;
}



// @brief: the memory the function need
// @exec: can't find function
int SymbolTable::get_func_stack_size(const std::string &func_name) {
    std::pair<bool, TableEntry *> search_res = SearchFunc(func_name);
    if (!search_res.first) {
        std::cout << "can't find func" << std::endl;
        return 0;
    } else {
        int func_size = 0;
        std::vector<TableEntry> func_table = func_tables_[func_name];
        for (auto &i: func_table) {
            if (i.symbol_type == SymbolType::PARAM) {
                func_size += 4; // the param array is just an address
            } else {
                func_size += i.size;
            }
        }
        return func_size;
    }
}

void SymbolTable::add_to_strcons(const std::string& str) {
    if (str == "%d") return;
    for (auto & strcon : strcons_) {
        if (strcon == str) {
            return;
        }
    }
    strcons_.push_back(str);
}

int SymbolTable::find_str_idx(const std::string& str) {
    auto it = std::find(strcons_.begin(), strcons_.end(), str);
    if (it != strcons_.end()) {
        return it-strcons_.begin();
    } else {
        add_error("can't find " + str + " in strcons");
        return -1;
    }
}

void SymbolTable::add_error(const std::string& msg) {
    std::cout << msg << std::endl;
}
