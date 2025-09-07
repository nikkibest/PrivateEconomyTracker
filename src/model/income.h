#ifndef INCOME_H
#define INCOME_H

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
};

inline void to_json(nlohmann::json& j, const Income& i) {
    j = nlohmann::json{
        {"source", i.source},
        {"nrAnnualPayments", i.nrAnnualPayments},
        {"amountNet", i.amountNet},
        {"amountNet_month", i.amountNet_month},
        {"amountNet_year", i.amountNet_year}
    };
}

inline void from_json(const nlohmann::json& j, Income& i) {
    j.at("source").get_to(i.source);
    j.at("nrAnnualPayments").get_to(i.nrAnnualPayments);
    j.at("amountNet").get_to(i.amountNet);
    j.at("amountNet_month").get_to(i.amountNet_month);
    j.at("amountNet_year").get_to(i.amountNet_year);
}

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


#endif // INCOME_H