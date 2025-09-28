#ifndef BUDGETDATA_H
#define BUDGETDATA_H
#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>

inline const std::string filename = "../budget.json";

struct BankBalance {
    int id = 0;
    std::string source;
    double amountNet;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(BankBalance, id, source, amountNet);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(BankBalance, id, source, amountNet);
};
inline std::vector<std::string> bankbalanceTableHeader = {
    "Source", "Amount (DKK)"
};
inline std::vector<std::string> bankbalanceTableOrder = {
    "source", "amountNet"
};

struct Income {
    int id = 0;
    std::string source;
    double nrAnnualPayments;
    double amountNet;     // after taxes
    double amountNet_month;     // after taxes
    double amountNet_year;      // after taxes
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Income, id, source, nrAnnualPayments, amountNet, amountNet_month, amountNet_year);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Income, id, source, nrAnnualPayments, amountNet, amountNet_month, amountNet_year);
};
inline std::vector<std::string> incomeTableHeader = {
    "Source", "Nr. Annual Payments", "Amount (DKK)", "Monthly (DKK)", "Yearly (DKK)"
};
inline std::vector<std::string> incomeTableOrder = {
    "source", "nrAnnualPayments", "amountNet", "amountNet_month", "amountNet_year"
};

struct Expense {
    int id = 0;
    std::string source;
    std::string category;
    std::string person;
    std::string typeExpense;
    std::string typeAccount;
    double nrAnnualPayments;
    double amountNet;
    double amountNet_month;
    double amountNet_year;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Expense, id, source, nrAnnualPayments, amountNet, amountNet_month, amountNet_year, category, person, typeExpense, typeAccount);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Expense, id, source, nrAnnualPayments, amountNet, amountNet_month, amountNet_year, category, person, typeExpense, typeAccount);
};
inline std::vector<std::string> expenseTableHeader = {
    "Source", "Category", "Person", "Type", "Account", "Nr. Annual Payments", "Amount (DKK)", "Monthly (DKK)", "Yearly (DKK)"
};
inline std::vector<std::string> expenseTableOrder = {
    "source", "category", "person", "typeExpense", "typeAccount", "nrAnnualPayments", "amountNet", "amountNet_month", "amountNet_year"
};

// Save item to budget.json with nested keys using dot notation
// Example: key = "expenses.expenses" will save to j["expenses"]["expenses"]
// This function will create missing objects for intermediate keys if needed.
template<typename T>
inline void save__to__json(const T& in, const std::string& filename, const std::string& key) {
    nlohmann::json j;
    std::ifstream file(filename);
    if (!file){
        std::cerr << "[ERROR] load_from_json: File '" << filename << "' does not exist.\n";
        std::cin.get();
        std::exit(EXIT_FAILURE);
    }
    file >> j;
    file.close();
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
// Helper for vector types
template<typename T>
void load_from_json_impl(std::vector<T>& out, nlohmann::json* ptr) {
    out.clear();
    if (ptr && ptr->is_array()) {
        for (const auto& item : *ptr) {
            out.push_back(item.get<T>());
        }
    }
}

// Helper for scalar types
template<typename T>
void load_from_json_impl(T& out, nlohmann::json* ptr) {
    if (ptr && (ptr->is_number() || ptr->is_string() || ptr->is_boolean())) {
        out = ptr->get<T>();
    }
}

template<typename T>
inline void load_from_json(T& out, const std::string& filename, const std::string& key) {
    std::ifstream file(filename);
    if (!file){
        std::cerr << "[ERROR] load_from_json: File '" << filename << "' does not exist.\n";
        std::cin.get();
        std::exit(EXIT_FAILURE);
    }
    nlohmann::json j;
    file >> j;
    file.close();
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
    for (const auto& k : keys) {
        if (ptr->contains(k)) {
            ptr = &((*ptr)[k]);
        } else {
            std::cerr << "[ERROR] load_from_json: Key '" << key << "' does not exist in '" << filename << "'.\n";
            std::cin.get();
            std::exit(EXIT_FAILURE);
        }
    }
    load_from_json_impl(out, ptr);
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
save__to__json(expenses, "budget.json", "expenses.expenses");
load_from_json(expenses, "budget.json", "expenses.expenses");
save__to__json(categories, "budget.json", "expenses.categories");
load_from_json(categories, "budget.json", "expenses.categories");
save__to__json(incomes, "budget.json", "incomes.incomes");
load_from_json(incomes, "budget.json", "incomes.incomes");
save__to__json(persons, "budget.json", "persons");
load_from_json(persons, "budget.json", "persons");
-------------------
*/

#endif // BUDGETDATA_H
