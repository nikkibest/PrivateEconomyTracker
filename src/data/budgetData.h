#ifndef BUDGETDATA_H
#define BUDGETDATA_H
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

struct Income {
    std::string source;
    double nrAnnualPayments;
    double amountNet;     // after taxes
    double amountNet_month;     // after taxes
    double amountNet_year;      // after taxes
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Income, source, nrAnnualPayments, amountNet, amountNet_month, amountNet_year);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Income, source, nrAnnualPayments, amountNet, amountNet_month, amountNet_year);
};

inline void save_incomes(const std::vector<Income>& incomes, const std::string& filename) {
    nlohmann::json j = incomes;
    std::ofstream file(filename);
    file << j.dump(4);
}

inline void load_incomes(std::vector<Income>& incomes, const std::string& filename) {
    std::ifstream file(filename);
    if (file) {
        nlohmann::json j;
        file >> j;
        incomes = j.get<std::vector<Income>>();
    }
}

struct Expense {
    std::string source;
    std::string category;
    std::string person;
    std::string typeExpense;
    std::string typeAccount;
    double nrAnnualPayments;
    double amountNet;
    double amountNet_month;
    double amountNet_year;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Expense, source, nrAnnualPayments, amountNet, amountNet_month, amountNet_year, category, person, typeExpense, typeAccount);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Expense, source, nrAnnualPayments, amountNet, amountNet_month, amountNet_year, category, person, typeExpense, typeAccount);
};

// Save item to budget.json with nested keys using dot notation
// Example: key = "expenses.expenses" will save to j["expenses"]["expenses"]
// This function will create missing objects for intermediate keys if needed.
template<typename T>
inline void save_json_array(const std::vector<T>& in, const std::string& filename, const std::string& key) {
    nlohmann::json j;
    std::ifstream file(filename);
    if (file) {
        file >> j;
        file.close();
    }
    // Split key by '.' for nested access
    std::vector<std::string> keys;
    size_t start = 0, end = 0;
    while ((end = key.find('.', start)) != std::string::npos) {
        keys.push_back(key.substr(start, end - start));
        start = end + 1;
    }
    keys.push_back(key.substr(start));
    // Traverse or create nested objects for each key except the last
    nlohmann::json* ptr = &j;
    for (size_t i = 0; i < keys.size() - 1; ++i) {
        if (!ptr->contains(keys[i]) || !(*ptr)[keys[i]].is_object()) {
            (*ptr)[keys[i]] = nlohmann::json::object(); // create if missing
        }
        ptr = &((*ptr)[keys[i]]);
    }
    // Set the final key to the array
    (*ptr)[keys.back()] = in;
    std::ofstream out(filename);
    out << j.dump(4);
}

// Load item from budget.json with nested keys using dot notation
// Example: key = "expenses.expenses" will load from j["expenses"]["expenses"]
// This function will safely traverse the nested keys and only load if the final key is an array.
template<typename T>
inline void load_json_array(std::vector<T>& out, const std::string& filename, const std::string& key) {
    std::ifstream file(filename);
    out.clear();
    if (file) {
        nlohmann::json j;
        file >> j;
        // Split key by '.' for nested access
        std::vector<std::string> keys;
        size_t start = 0, end = 0;
        while ((end = key.find('.', start)) != std::string::npos) {
            keys.push_back(key.substr(start, end - start));
            start = end + 1;
        }
        keys.push_back(key.substr(start));
        // Traverse nested objects for each key
        nlohmann::json* ptr = &j;
        bool missing = false;
        for (const auto& k : keys) {
            if (ptr->contains(k)) {
                ptr = &((*ptr)[k]);
            } else {
                missing = true;
                ptr = nullptr;
                break;
            }
        }
        // Only load if the final key is an array
        if (ptr && ptr->is_array()) {
            for (const auto& item : *ptr) {
                out.push_back(item.get<T>());
            }
        } else if (missing) {
            std::cerr << "[ERROR] load_json_array: Key '" << key << "' does not exist in '" << filename << "'.\n";
            std::cin.get();
            std::exit(EXIT_FAILURE);
        }
    }
}

/*
-------------------
How this logic works:

1. Dot notation keys (e.g. "expenses.expenses") are split into a vector of strings: ["expenses", "expenses"].
2. For saving, the function traverses the JSON object, creating intermediate objects if they don't exist, and sets the final key to the array.
3. For loading, the function traverses the JSON object, checking that each key exists, and only loads the array if the final key is present and is an array.
4. This prevents out-of-bounds errors and ensures robust handling of missing or malformed data.
5. You can use these functions for any vector type (Expense, Income, std::string, etc.) and any nested key path.

Example usage:
save_json_array(expenses, "budget.json", "expenses.expenses");
load_json_array(expenses, "budget.json", "expenses.expenses");
save_json_array(categories, "budget.json", "expenses.categories");
load_json_array(categories, "budget.json", "expenses.categories");
save_json_array(incomes, "budget.json", "incomes.incomes");
load_json_array(incomes, "budget.json", "incomes.incomes");
save_json_array(persons, "budget.json", "persons");
load_json_array(persons, "budget.json", "persons");
-------------------
*/

#endif // BUDGETDATA_H
