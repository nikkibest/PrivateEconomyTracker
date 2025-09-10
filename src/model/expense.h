#ifndef EXPENSE_H
#define EXPENSE_H

#include <vector>
#include <fstream>
#include <string>
#include <nlohmann/json.hpp>


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

// inline void to_json(nlohmann::json& j, const Expense& e) {
//     j = nlohmann::json{
//         {"category", e.category},
//         {"type", e.type},
//         {"person", e.person},
//         {"nrAnnualPayments", e.nrAnnualPayments},
//         {"amountNet", e.amountNet},
//         {"amountNet_month", e.amountNet_month},
//         {"amountNet_year", e.amountNet_year}
//     };
// }


// inline void from_json(const nlohmann::json& j, Expense& e) {
//     j.at("category").get_to(e.category);
//     j.at("type").get_to(e.type);
//     j.at("person").get_to(e.person);
//     j.at("nrAnnualPayments").get_to(e.nrAnnualPayments);
//     j.at("amountNet").get_to(e.amountNet);
//     j.at("amountNet_month").get_to(e.amountNet_month);
//     j.at("amountNet_year").get_to(e.amountNet_year);
// }

inline void save_expenses(const std::vector<Expense>& expenses, const std::string& filename, const std::string& extension) {
    nlohmann::json j;
    // Load existing file to preserve categories/persons
    std::ifstream file(filename);
    if (file) {
        file >> j;
        file.close();
    }
    j["expenses"] = expenses; // nlohmann::json will use your to_json for Expense
    std::ofstream out(filename);
    out << j.dump(4);
}

inline void load_expenses(std::vector<Expense>& expenses, const std::string& filename, const std::string& extension) {
    expenses.clear();
    std::ifstream file(filename);
    if (file) {
        nlohmann::json j;
        file >> j;
        if (j.contains("expenses") && j["expenses"].is_array()) {
            for (const auto& exp : j["expenses"]) {
                expenses.push_back(exp.get<Expense>());
            }
        }
    }
}

// Load items from expenses.json
inline void load_items(std::vector<std::string>& item, const std::string& filename, const std::string& extension) {
    std::ifstream file(filename);
    if (file) {
        nlohmann::json j;
        file >> j;
        item.clear();
        if (j.contains(extension) && j[extension].is_array()) {
            for (const auto& cat : j[extension]) {
                item.push_back(cat.get<std::string>());
            }
        }
    }
}
// Save item to expenses.json
inline void save_items(const std::vector<std::string>& item, const std::string& filename, const std::string& extension) {
    nlohmann::json j;
    std::ifstream file(filename);
    if (file) {
        file >> j;
        file.close();
    }
    j[extension] = item;
    std::ofstream out(filename);
    out << j.dump(4);
}

#endif // EXPENSE_H
