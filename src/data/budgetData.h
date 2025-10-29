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
    bool operator==(const BankBalance& other) const {
        return id == other.id &&
               source == other.source &&
               amountNet == other.amountNet;
    }
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
    std::string typeAccount;
    double nrAnnualPayments;
    double amountNet;
    double amountNet_month;
    double amountNet_year;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Expense, id, source, person, typeAccount, nrAnnualPayments, amountNet, amountNet_month, amountNet_year, category);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Expense, id, source, person, typeAccount, nrAnnualPayments, amountNet, amountNet_month, amountNet_year, category);
};
inline std::vector<std::string> expenseTableHeader = {
    "Source", "Category", "Person", "Account", "Nr. Annual Payments", "Amount (DKK)", "Monthly (DKK)", "Yearly (DKK)"
};
inline std::vector<std::string> expenseTableOrder = {
    "source", "category", "person", "typeAccount", "nrAnnualPayments", "amountNet", "amountNet_month", "amountNet_year"
};

struct ExpenseItems {
    int id = 0;
    std::string item;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ExpenseItems, id, item);
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ExpenseItems, id, item);
};


// Type trait to check if a type is a std::vector
template<typename T>
struct is_std_vector : std::false_type {};

template<typename T, typename Alloc>
struct is_std_vector<std::vector<T, Alloc>> : std::true_type {};

// Save item to budget.json with nested keys using dot notation
// Example: key = "expenses.expenses" will save to j["expenses"]["expenses"]
// This function will create missing objects for intermediate keys if needed.

// New version: save to history by date and status
template<typename T>
inline void save__to__json(const T& in, const std::string& filename, const std::string& key, const std::string& date, const std::string& status) {
    nlohmann::json j;
    std::ifstream file(filename);
    if (!file){
        std::cerr << "[ERROR] save__to__json: File '" << filename << "' does not exist.\n";
        std::cin.get();
        std::exit(EXIT_FAILURE);
    }
    file >> j;
    file.close();
    if (!j.contains("history") || !j["history"].is_array()) j["history"] = nlohmann::json::array();
    auto& history = j["history"];
    // Find or create the entry for the given date
    nlohmann::json* entry = nullptr;
    for (auto& e : history) {
        if (e.contains("date") && e["date"] == date) {
            entry = &e;
            break;
        }
    }
    if (!entry) {
        // Create new date object
        nlohmann::json newEntry;
        newEntry["date"] = date;
        newEntry["status"] = status;
        history.push_back(newEntry);
        entry = &history.back();
    } else {
        // Update status
        (*entry)["status"] = status;
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
    nlohmann::json* ptr = entry;
    for (size_t i = 0; i < keys.size() - 1; ++i) {
        if (!ptr->contains(keys[i]) || !(*ptr)[keys[i]].is_object()) {
            (*ptr)[keys[i]] = nlohmann::json::object(); // create if missing
        }
        ptr = &((*ptr)[keys[i]]);
    }
    // Set the final key to the value
    // For vector types, update existing entries with the same id, otherwise append
    if constexpr (is_std_vector<T>::value) {
        // Ensure the array exists
        if (!(*ptr).contains(keys.back()) || !(*ptr)[keys.back()].is_array()) {
            (*ptr)[keys.back()] = nlohmann::json::array();
        }
        auto& arr = (*ptr)[keys.back()];
        for (const auto& item : in) {
            nlohmann::json item_json = item;
            // Only handle items with 'id' field
            if (item_json.contains("id")) {
                int id = item_json["id"];
                bool updated = false;
                for (auto& arr_item : arr) {
                    if (arr_item.contains("id") && arr_item["id"] == id) {
                        arr_item = item_json;
                        updated = true;
                        break;
                    }
                }
                if (!updated) {
                    arr.push_back(item_json);
                }
            } else {
                // If no id, just append (fallback)
                arr.push_back(item_json);
            }
        }
    } else {
        // Scalar: assign directly
        (*ptr)[keys.back()] = in;
    }
    std::ofstream out(filename);
    out << j.dump(4);
}


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

// Loads from the latest entry in history, or a specific date if provided (date == "" means latest)
// New: Merge history entries in ascending date order, applying changes, additions, and deletions
template<typename T>
inline void load_from_json(T& out, const std::string& filename, const std::string& key, const std::string& date, std::map<int, std::string>& id_to_date) {
    std::ifstream file(filename);
    if (!file) {
        // Create file with {"history": []}
        std::ofstream outFile(filename);
        outFile << "{\n    \"history\": []\n}";
        outFile.close();
        // Now open again for reading
        file.open(filename);
    }
    nlohmann::json j;
    file >> j;
    file.close();
    if (!j.contains("history") || !j["history"].is_array()) {
        std::cerr << "[ERROR] load_from_json: No 'history' array in '" << filename << "'.\n";
        std::cin.get();
        std::exit(EXIT_FAILURE);
    }
    const auto& history = j["history"];
    // Collect all entries with a date, sort by date ascending
    std::vector<std::pair<std::string, const nlohmann::json*>> dated_entries;
    for (const auto& e : history) {
        if (e.contains("date") && e["date"].is_string()) {
            if (date.empty() || e["date"].get<std::string>() <= date) {
                dated_entries.emplace_back(e["date"].get<std::string>(), &e);
            }
        }
    }
    std::sort(dated_entries.begin(), dated_entries.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
    // Split key by '.' for nested access
    std::vector<std::string> keys;
    size_t start = 0, end = 0;
    while ((end = key.find('.', start)) != std::string::npos) {
        keys.push_back(key.substr(start, end - start));
        start = end + 1;
    }
    keys.push_back(key.substr(start));
    // Merge logic: start with oldest, apply changes as we go
    nlohmann::json merged;
    bool found_any = false;
    for (const auto& pair : dated_entries) {
        const nlohmann::json* candidate = pair.second;
        const nlohmann::json* ptr = candidate;
        bool valid = true;
        for (const auto& k : keys) {
            if (ptr->contains(k)) {
                ptr = &((*ptr)[k]);
            } else {
                valid = false;
                break;
            }
        }
        if (valid) {
            // If this is the first found, copy it
            if (!found_any) {
                merged = *ptr;
                // Record id to date mapping for items with id
                for (const auto& item : *ptr) {
                    if (item.contains("id")) {
                        int id = item["id"].get<int>();
                        id_to_date[id] = pair.first;
                    }
                }
                found_any = true;
            } else {
                // Merge/patch: for arrays, replace by id; for objects, update fields
                if (merged.is_array() && ptr->is_array()) {
                    // Merge arrays by id (assume each item has an 'id' field)
                    std::map<int, nlohmann::json> by_id;
                    for (const auto& item : merged) {
                        if (item.contains("id")) by_id[item["id"].get<int>()] = item;
                    }
                    for (const auto& item : *ptr) {
                        if (item.contains("id")) by_id[item["id"].get<int>()] = item;
                        else by_id[-1] = item; // fallback for items without id
                    }
                    // Record id to date mapping for items with id
                    for (const auto& item : *ptr) {
                        if (item.contains("id")) {
                            int id = item["id"].get<int>();
                            id_to_date[id] = pair.first;
                        }
                    }
                    merged = nlohmann::json::array();
                    for (const auto& kv : by_id) merged.push_back(kv.second);
                } else if (merged.is_object() && ptr->is_object()) {
                    for (auto it = ptr->begin(); it != ptr->end(); ++it) {
                        merged[it.key()] = it.value();
                    }
                } else {
                    merged = *ptr;
                }
            }
        }
        // TODO: handle explicit deletions if you add deletion markers in the future
    }
    if (found_any) {
        load_from_json_impl(out, &merged);
    } else {
        out = T{};
    }
}

template<typename T>
inline void load_from_json(T& out, const std::string& filename, const std::string& key, const std::string& date = "") {
    std::map<int, std::string> id_to_date;
    load_from_json(out, filename, key, date, id_to_date);
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
