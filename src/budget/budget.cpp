#include "budget.h"

#include <set>
#include <iostream>
#define CHECKBOX_FLAG(flags, flag) ImGui::CheckboxFlags(#flag, (unsigned int*)&flags, flag)

namespace budget {
BudgetManager::BudgetManager()
    : balanceState(), months(12)
{
    loadBalanceData();
    setMonths(months);
}

// Helper to get today's date in YYYY-MM-DD format
auto getToday = []() -> std::string {
    time_t t = time(nullptr);
    struct tm tm_buf;
    localtime_s(&tm_buf, &t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_buf);
    return std::string(buf);
};

auto getDateStr = [](double time) -> std::string {
    time_t t = (time_t)time;
    struct tm tm_buf;
    localtime_s(&tm_buf, &t);
    char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", &tm_buf);
    return std::string(buf);
};

void BudgetManager::SelectDateUI(std::vector<std::string>& allDates, std::string& selectedDate, std::string& status, bool& loadedDates, bool& loadedData){
    // Load all dates from budget.json
    if (!loadedDates) {
        allDates.clear();
        std::ifstream file(filename);
        if (file) {
            nlohmann::json j;
            file >> j;
            if (j.contains("history") && j["history"].is_array()) {
                for (const auto& entry : j["history"]) {
                    if (entry.contains("date") && entry["date"].is_string()) {
                        allDates.push_back(entry["date"].get<std::string>());
                    }
                }
            }
            std::sort(allDates.begin(), allDates.end(), [](const auto& a, const auto& b) { return a < b; }); // Sort dates ascending
        }
        loadedDates = true;
    }
    // Default selected date is today if not set
    if (selectedDate.empty()) {
        selectedDate = getToday();
    }
    // Date picker UI (combo + input)
    ImGui::Text("Select Date:");
    static char dateBuf[16] = "";
    strncpy(dateBuf, selectedDate.c_str(), sizeof(dateBuf)-1);
    dateBuf[sizeof(dateBuf)-1] = '\0';
    static std::string prevSelectedDate = selectedDate;
    if (ImGui::BeginCombo("##DateCombo", dateBuf)) {
        for (const auto& d : allDates) {
            bool is_selected = (selectedDate == d);
            if (ImGui::Selectable(d.c_str(), is_selected)) {
                selectedDate = d;
                strncpy(dateBuf, d.c_str(), sizeof(dateBuf)-1);
                dateBuf[sizeof(dateBuf)-1] = '\0';
            }
            if (is_selected) ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    ImGui::SameLine();
    ImGui::Text("Current Date Selection");
    if (ImGui::InputText("Select Date", dateBuf, sizeof(dateBuf))) {
        selectedDate = std::string(dateBuf);
    }
    if (selectedDate != prevSelectedDate) {
        loadedData = false;
        prevSelectedDate = selectedDate;
    }
    // Find status for selected date (scan in ascending order, use last found)
    status = "tentative";
    std::vector<std::pair<std::string, std::string>> statusEntries;
    std::ifstream file2(filename);
    if (file2) {
        nlohmann::json j;
        file2 >> j;
        if (j.contains("history") && j["history"].is_array()) {
            for (const auto& entry : j["history"]) {
                if (entry.contains("date") && entry["date"].is_string() && entry["date"] <= selectedDate && entry.contains("status")) {
                    statusEntries.emplace_back(entry["date"].get<std::string>(), entry["status"].get<std::string>());
                }
            }
        }
    }
    if (!statusEntries.empty()) {
        // Sort by date ascending and use the last one
        std::sort(statusEntries.begin(), statusEntries.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
        status = statusEntries.back().second;
    }
    // ImGui::Text("Status: %s", status.c_str());
    if (status == "confirmed") {
        ImGui::TextColored(ImVec4(0,1,0,1), "Status: %s", status.c_str());
    } else if (status == "tentative") {
        ImGui::TextColored(ImVec4(1,1,0,1), "Status: %s", status.c_str());
    } else {
        ImGui::TextColored(ImVec4(1,0,0,1), "Status: %s", status.c_str());
    }
}

void BudgetManager::loadBalanceData() {
    // Load bank balances for selected date
    load_from_json(bankBalance, filename, "balance.balance", balanceState.selectedDate);
    setTotalBalanceTable(0.0);
    balanceState.allNames.clear();
    for (auto& b : bankBalance) {
        setTotalBalanceTable(getTotalBalanceTable() + b.amountNet);
        balanceState.allNames.push_back(b.source);
    }

    // Get all historical dates and corresponding total balances for plot
    balanceDatesHist.clear();
    balanceHistAmount.clear();
    balanceHistTimes.clear();
    balanceStatusHist.clear();
    nHistBalances = 0;
    std::ifstream file("../budget.json");
    nlohmann::json j;
    file >> j;
    if (j.contains("history") && j["history"].is_array()) {
        for (const auto& entry : j["history"]) {
            if (entry.contains("date") && entry.contains("balance") && entry["balance"].contains("balance")) {
                std::string date = entry["date"];
                std::string status = entry["status"];
                balanceDatesHist.push_back(date);
                balanceStatusHist.push_back(status);
            }
        }
    }
    // Combine into pairs
    std::vector<std::pair<std::string, std::string>> date_status_pairs;
    for (size_t i = 0; i < balanceDatesHist.size(); ++i) {
        date_status_pairs.emplace_back(balanceDatesHist[i], balanceStatusHist[i]);
    }
    // Sort by date
    std::sort(date_status_pairs.begin(), date_status_pairs.end(),
    [](const auto& a, const auto& b) { return a.first < b.first; });
    // Split back into separate vectors
    for (size_t i = 0; i < date_status_pairs.size(); ++i) {
        balanceDatesHist[i] = date_status_pairs[i].first;
        balanceStatusHist[i] = date_status_pairs[i].second;
    }
    // std::sort(balanceDatesHist.begin(), balanceDatesHist.end(), [](const auto& a, const auto& b) { return a < b; });
    // Convert balanceDatesHist to time_t
    for (const auto& dateStr : balanceDatesHist) {
        load_from_json(bankBalanceHist, filename, "balance.balance", dateStr);
        double totalBalanceTmp = 0.0;
        for (const auto& b : bankBalanceHist) {
            totalBalanceTmp += b.amountNet;
        }
        balanceHistAmount.push_back(totalBalanceTmp);
        // Convert date string "YYYY-MM-DD" to time_t
        std::tm tm = {};
        std::istringstream ss(dateStr);
        ss >> std::get_time(&tm, "%Y-%m-%d");
        time_t t = mktime(&tm);
        balanceHistTimes.push_back(static_cast<double>(t));
        // Increment counter
        nHistBalances++;
    }
    balanceState.loadedData = true;
}

void BudgetManager::ShowBankBalanceInput() {    
    // --- Date Picker and Status UI ---
    // static std::vector<std::string> allNames;
    // static std::vector<std::string> allDates;
    // static std::string selectedDate;
    // static std::string status;
    // static bool loadedDates = false;
    // static bool loadedData = true;
    SelectDateUI(balanceState.allDates, balanceState.selectedDate, balanceState.status, balanceState.loadedDates, balanceState.loadedData);
    // --- End Date Picker and Status UI ---
    if (!balanceState.loadedData) {
        loadBalanceData();
    }
    static int editId = 0;
    static char source[64] = "";
    static float amount = 0.0f;
    // ImGui::Begin("Bank Balance");
    ImGui::Text("Add a new household bank balance here.");
    ImGui::InputText("Source", source, IM_ARRAYSIZE(source));
    ImGui::InputFloat("Amount", &amount);

     // Helper to copy income data to input fields
    auto SetEditBalance = [&](const BankBalance& bal) {
        editId = bal.id;
        strncpy(source, bal.source.c_str(), sizeof(source) - 1);
        source[sizeof(source) - 1] = '\0';
        amount = static_cast<float>(bal.amountNet);
        balanceState.selectedDate = getToday();
    };

    auto SetDeleteBalance = [&](const BankBalance& bal) {
        editId = bal.id;
        strncpy(source, bal.source.c_str(), sizeof(source) - 1);
        source[sizeof(source) - 1] = '\0';
        amount = 0.0f;
        balanceState.selectedDate = getToday();
    };

    if ((editId == 0) && !(std::find(balanceState.allNames.begin(), balanceState.allNames.end(), source) != balanceState.allNames.end())) {
        if (ImGui::Button("Add New Bank Balance")) {
            if (strlen(source) > 0 && amount > 0.0f) {
                // Find the first unused positive integer ID
                std::set<int> usedIds;
                std::vector<BankBalance> bankBalanceHist = bankBalance;
                load_from_json(bankBalanceHist, filename, "balance.balance", "");
                for (const auto& bal : bankBalanceHist) {
                    usedIds.insert(bal.id);
                }
                int newId = 1;
                while (true) {
                    if (usedIds.find(newId) == usedIds.end()) {
                        break;
                    }
                    ++newId;
                }
                BankBalance newBalance{
                    newId,
                    std::string(source),
                    static_cast<double>(amount),
                };
                std::vector<BankBalance> toSaveBalance = {newBalance};
                save__to__json(toSaveBalance, filename, "balance.balance", balanceState.selectedDate, "tentative");
                source[0] = '\0';
                amount = 0.0f;
                balanceState.loadedData = false;
                balanceState.loadedDates = false; // Reload dates to include new date if added
            }
        }
    } else {
        if (ImGui::Button("Add Bank Balance Changes")) {
            BankBalance newBalance{
                editId,
                std::string(source),
                static_cast<double>(amount),
            };
            std::vector<BankBalance> toSaveBalance = {newBalance};
            save__to__json(toSaveBalance, filename, "balance.balance", balanceState.selectedDate, "tentative");
            source[0] = '\0';
            amount = 0.0f;
            editId = 0;
            balanceState.loadedData = false;
            balanceState.loadedDates = false; // Reload dates to include new date if added
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel Changes")) {
            source[0] = '\0';
            amount = 0.0f;
            editId = 0;
        }
    }
    ImGui::SameLine();
    if (ImGui::Button("Confirm This Bank Balance")) {
        // save__to__json(bankBalance, filename, "balance.balance");
    }
    
    ImGui::Separator();
    ImGui::Text("Summary of all Bank Balances:");
    if (ImGui::BeginTable("BankBalanceTableSummary", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Total Bank Balance (DKK)");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(format_euro(getTotalBalanceTable()).c_str());
        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::Text("All Bank Balances:");
    CreateTable<BankBalance>("IncomeTable", bankBalance, bankbalanceTableHeader, bankbalanceTableOrder, filename, "balance.balance", SetEditBalance, SetDeleteBalance);    
    ImGui::Separator();
    // ImGui::End();
}

void BudgetManager::ShowIncomeInput() {    
    static bool loaded = false;
    
    if (!loaded) {
        // Load incomes from file or initialize as needed
        load_from_json(incomes, filename, "incomes.incomes","");
        loaded = true;
    }
    // Static for editing
    static int editId = 0;
    static char source[64] = "";
    static float nrAnnualPayments = 0.0f;
    static float monthly = 0.0f;
    static float yearly = 0.0f;
    static float amount = 0.0f;
    // ImGui::Begin("Incomes");
    ImGui::Text("Add a new household income here.");
    ImGui::InputText("Source", source, IM_ARRAYSIZE(source));
    ImGui::InputFloat("Nr. Annual Payments", &nrAnnualPayments);
    ImGui::InputFloat("Amount", &amount);

    // Helper to copy income data to input fields
    auto SetEditIncome = [&](const Income& inc) {
        editId = inc.id;
        strncpy(source, inc.source.c_str(), sizeof(source) - 1);
        source[sizeof(source) - 1] = '\0';
        nrAnnualPayments = static_cast<float>(inc.nrAnnualPayments);
        amount = static_cast<float>(inc.amountNet);
        monthly = static_cast<float>(inc.amountNet_month);
        yearly = static_cast<float>(inc.amountNet_year);
    };

    if (editId == 0) {
        if (ImGui::Button("Add Income")) {
            if (strlen(source) > 0 && amount > 0.0f) {
                yearly = amount * nrAnnualPayments;
                monthly = yearly / 12.0f;
                // Find the first unused positive integer ID
                std::set<int> usedIds;
                for (const auto& inc : incomes) {
                    usedIds.insert(inc.id);
                }
                int newId = 1;
                while (usedIds.count(newId)) {
                    ++newId;
                }
                incomes.push_back(Income{
                    newId,
                    std::string(source),
                    static_cast<double>(nrAnnualPayments),
                    static_cast<double>(amount),
                    static_cast<double>(monthly),
                    static_cast<double>(yearly)
                });
                //save__to__json(incomes, filename, "incomes.incomes");
                source[0] = '\0';
                nrAnnualPayments = 0.0f;
                amount = 0.0f;
                monthly = 0.0f;
                yearly = 0.0f;
                // Update summary values
                //ComputeDependentValues(incomes, filename, "incomes");
            }
        }
    } else {
        if (ImGui::Button("Save Changes")) {
            if (strlen(source) > 0 && amount > 0.0f) {
                yearly = amount * nrAnnualPayments;
                monthly = yearly / 12.0f;
                for (auto& inc : incomes) {
                    if (inc.id == editId) {
                        inc.source = source;
                        inc.nrAnnualPayments = static_cast<double>(nrAnnualPayments);
                        inc.amountNet = static_cast<double>(amount);
                        inc.amountNet_month = static_cast<double>(monthly);
                        inc.amountNet_year = static_cast<double>(yearly);
                        break;
                    }
                }
                //save__to__json(incomes, filename, "incomes.incomes");
                source[0] = '\0';
                nrAnnualPayments = 0.0f;
                amount = 0.0f;
                monthly = 0.0f;
                yearly = 0.0f;
                editId = 0;
                //ComputeDependentValues(incomes, filename, "incomes");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel Editting")) {
            source[0] = '\0';
            nrAnnualPayments = 0.0f;
            amount = 0.0f;
            monthly = 0.0f;
            yearly = 0.0f;
            editId = 0;
        }
    }

    ImGui::Separator();
    ImGui::Text("Summary of all Incomes:");
    if (ImGui::BeginTable("IncomeTableSummary", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        double totalYearly = 0.0, avgMonthly = 0.0;
        load_from_json(totalYearly, "../budget.json", "incomes.totalYearly", "");
        load_from_json(avgMonthly, "../budget.json", "incomes.avgMonthly", "");

        ImGui::TableSetupColumn("Total Yearly (DKK)");
        ImGui::TableSetupColumn("Avg. Monthly (DKK)");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(format_euro(totalYearly).c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(format_euro(avgMonthly).c_str());
        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::Text("All Incomes:");
    CreateTable<Income>("IncomeTable",incomes, incomeTableHeader, incomeTableOrder, filename, "incomes.incomes", SetEditIncome, nullptr);    
    ImGui::Separator();
    // ImGui::End();
}

template<typename T>
void BudgetManager::ComputeDependentValues(std::vector<T>& items, const std::string filename, const std::string keyStart, const std::string& date, const std::string& status) {
    // Update summary values for Income or Expense
    double totalYearly = 0.0;
    for (const auto& item : items) {
        totalYearly += item.amountNet_year;
    }
    double avgMonthly = totalYearly / 12.0f;
    //save__to__json(totalYearly, filename, keyStart+".totalYearly");
    //save__to__json(avgMonthly, filename, keyStart+".avgMonthly");
}

void BudgetManager::ShowExpenseInput() {
    // UI state variables for categories and persons
    static std::vector<std::string> categories;
    static bool isLoadedCategories = false;
    static int idxSelectedCategory = 0;
    static char newCategory[64] = "";
    static std::vector<std::string> persons;
    static bool isLoadedPersons = false;
    static int idxSelectedPerson = 0;
    static char newPerson[64] = "";
    // Combo options and indices
    static const char* mustExpenditureOptions[] = { "Must", "Forbrug" };
    static int mustExpenditureIdx = 0;
    static const char* accountTypeOptions[] = { "Budgetkonto", "Forbrugskonto", "Lønkonto" };
    static int accountTypeIdx = 0;
    std::string extension;
    // Static for editing
    static int editId = 0;
    // ...existing code...
    // Input fields
    static char source[64] = "";
    static char category[64] = "";
    static char person[64] = "";
    static char typeExpense[64] = "";
    static char typeAccount[64] = "";
    static float nrAnnualPayments = 0.0f;
    static float monthly = 0.0f;
    static float yearly = 0.0f;
    static float amount = 0.0f;
    // Helper to copy expense data to input fields
    auto SetEditExpense = [&](const Expense& exp) {
        editId = exp.id;
        strncpy(source, exp.source.c_str(), sizeof(source) - 1);
        source[sizeof(source) - 1] = '\0';
        strncpy(category, exp.category.c_str(), sizeof(category) - 1);
        category[sizeof(category) - 1] = '\0';
        strncpy(person, exp.person.c_str(), sizeof(person) - 1);
        person[sizeof(person) - 1] = '\0';
        strncpy(typeExpense, exp.typeExpense.c_str(), sizeof(typeExpense) - 1);
        typeExpense[sizeof(typeExpense) - 1] = '\0';
        strncpy(typeAccount, exp.typeAccount.c_str(), sizeof(typeAccount) - 1);
        typeAccount[sizeof(typeAccount) - 1] = '\0';
        nrAnnualPayments = static_cast<float>(exp.nrAnnualPayments);
        amount = static_cast<float>(exp.amountNet);
        monthly = static_cast<float>(exp.amountNet_month);
        yearly = static_cast<float>(exp.amountNet_year);
    };
    // ...existing code...
    // ImGui::Begin("Expenses");
    ImGui::Text("Add a new household expense here.");
    static bool loaded = false;
    if (!loaded) {
        load_from_json(expenses, filename, "expenses.expenses", "");
        loaded = true;
    }
    ImGui::InputText("Source", source, IM_ARRAYSIZE(source));
    // Categories
    extension = "expenses.categories";
    CreateComboWithDeleteAndAdd(categories, isLoadedCategories, idxSelectedCategory, "category", filename, extension, newCategory);
    if (!categories.empty() && idxSelectedCategory >= 0 && idxSelectedCategory < (int)categories.size()) {
        strncpy(category, categories[idxSelectedCategory].c_str(), sizeof(category) - 1);
        category[sizeof(category) - 1] = '\0';
    } else {
        category[0] = '\0';
    }
    // Persons
    extension = "persons";
    CreateComboWithDeleteAndAdd(persons, isLoadedPersons, idxSelectedPerson, "person", filename, extension, newPerson);
    if (!persons.empty() && idxSelectedPerson >= 0 && idxSelectedPerson < (int)persons.size()) {
        strncpy(person, persons[idxSelectedPerson].c_str(), sizeof(person) - 1);
        person[sizeof(person) - 1] = '\0';
    } else {
        person[0] = '\0';
    }
    // Must/Expenditure Combo
    ImGui::Combo("Expense Type", &mustExpenditureIdx, mustExpenditureOptions, IM_ARRAYSIZE(mustExpenditureOptions));
    strncpy(typeExpense, mustExpenditureOptions[mustExpenditureIdx], sizeof(typeExpense) - 1);
    typeExpense[sizeof(typeExpense) - 1] = '\0';
    // Account Type Combo
    ImGui::Combo("Account Type", &accountTypeIdx, accountTypeOptions, IM_ARRAYSIZE(accountTypeOptions));
    strncpy(typeAccount, accountTypeOptions[accountTypeIdx], sizeof(typeAccount) - 1);
    typeAccount[sizeof(typeAccount) - 1] = '\0';
    ImGui::InputFloat("Nr. Annual Payments", &nrAnnualPayments);
    ImGui::InputFloat("Amount", &amount);
    // Add Expense button
    if (editId == 0) {
        if (ImGui::Button("Add Expense")) {
            if (strlen(source) > 0 && amount > 0.0f) {
                yearly = amount * nrAnnualPayments;
                monthly = yearly / 12.0f;
                // Find the first unused positive integer ID
                std::set<int> usedIds;
                for (const auto& exp : expenses) {
                    usedIds.insert(exp.id);
                }
                int newId = 1;
                while (usedIds.count(newId)) {
                    ++newId;
                }
                expenses.push_back(Expense{
                    newId,
                    std::string(source),
                    std::string(category),
                    std::string(person),
                    std::string(typeExpense),
                    std::string(typeAccount),
                    static_cast<double>(nrAnnualPayments),
                    static_cast<double>(amount),
                    static_cast<double>(monthly),
                    static_cast<double>(yearly)
                });
                //save__to__json(expenses, filename, "expenses.expenses");
                source[0] = '\0';
                nrAnnualPayments = 0.0f;
                amount = 0.0f;
                monthly = 0.0f;
                yearly = 0.0f;
                // Update summary values
                //ComputeDependentValues(expenses, filename, "expenses");
            }
        }
    } else {
        if (ImGui::Button("Save Changes")) {
            if (strlen(source) > 0 && amount > 0.0f) {
                yearly = amount * nrAnnualPayments;
                monthly = yearly / 12.0f;
                for (auto& exp : expenses) {
                    if (exp.id == editId) {
                        exp.source = source;
                        exp.category = category;
                        exp.person = person;
                        exp.typeExpense = typeExpense;
                        exp.typeAccount = typeAccount;
                        exp.nrAnnualPayments = static_cast<double>(nrAnnualPayments);
                        exp.amountNet = static_cast<double>(amount);
                        exp.amountNet_month = static_cast<double>(monthly);
                        exp.amountNet_year = static_cast<double>(yearly);
                        break;
                    }
                }
                //save__to__json(expenses, filename, "expenses.expenses");
                source[0] = '\0';
                nrAnnualPayments = 0.0f;
                amount = 0.0f;
                monthly = 0.0f;
                yearly = 0.0f;
                editId = 0;
                //ComputeDependentValues(expenses, filename, "expenses");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel Editting")) {
            source[0] = '\0';
            nrAnnualPayments = 0.0f;
            amount = 0.0f;
            monthly = 0.0f;
            yearly = 0.0f;
            editId = 0;
        }
    }

    ImGui::Separator();
    ImGui::Text("Summary of all Expenses:");
    if (ImGui::BeginTable("ExpenseTableSummary", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        double totalYearly = 0.0, avgMonthly = 0.0;
        load_from_json(totalYearly, "../budget.json", "expenses.totalYearly", "");
        load_from_json(avgMonthly, "../budget.json", "expenses.avgMonthly", "");

        ImGui::TableSetupColumn("Total Yearly (DKK)");
        ImGui::TableSetupColumn("Avg. Monthly (DKK)");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(format_euro(totalYearly).c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(format_euro(avgMonthly).c_str());
        ImGui::EndTable();
    }
    ImGui::Separator();
    ImGui::Text("All Expenses:");
    CreateTable<Expense>("ExpenseTable", expenses, expenseTableHeader, expenseTableOrder, filename, "expenses.expenses", SetEditExpense, nullptr);
    ImGui::Separator();
    // ImGui::End();
}

void BudgetManager::CreateComboWithDeleteAndAdd(std::vector<std::string>& item, bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem)
{
    if (extension.empty()) {
        std::cerr << "[ERROR] CreateComboWithDeleteAndAdd: extension key is empty for label '" << label << "'." << std::endl;
        return;
    }
    if (extension.empty()) {
        std::cerr << "[ERROR] CreateComboWithDeleteAndAdd: extension key is empty for label '" << label << "'." << std::endl;
        return;
    }
    if (!isLoaded) {
        // Load categories from expenses.json file
        isLoaded = true;   
        load_from_json(item, filename, extension,"");
    }
    std::string btnLabel;
    // Combo selection for categor
    if (!item.empty()) {
        const char* preview_value = item[selectedIndex].c_str();
        const std::string comboLabel = "Select " + label;
        if (ImGui::BeginCombo(comboLabel.c_str(), preview_value, ImGuiComboFlags_WidthFitPreview)) {
            for (int n = 0; n < static_cast<int>(item.size()); n++) {
                bool is_selected = (selectedIndex == n);
                if (ImGui::Selectable(item[n].c_str(), is_selected))
                    selectedIndex = n;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }
        // Add a delete button option for the selected category
        btnLabel = "Delete " + label;
        ImGui::SameLine();
        if (ImGui::Button(btnLabel.c_str())  && !item.empty()) {
            item.erase(item.begin() + selectedIndex);
            if (selectedIndex >= static_cast<int>(item.size()))
                selectedIndex = static_cast<int>(item.size()) - 1;
            //save__to__json(item, filename, extension);
            isLoaded = false; // Force reload to update the combo
        }
        ImGui::SameLine();
    }
    // Add new category input and button
    float minWidth = 120.0f;
    float charWidth = ImGui::CalcTextSize(newItem).x + 30.0f; // 30 for padding and hint
    float inputWidth = std::max(minWidth, charWidth);
    ImGui::SetNextItemWidth(inputWidth);
    btnLabel = "##New" + label;
    std::string hintLabel = "Add new " + label;
    ImGui::InputTextWithHint(btnLabel.c_str(), hintLabel.c_str(), newItem, 128);
    ImGui::SameLine();
    btnLabel = "Add " + label;
    if (ImGui::Button(btnLabel.c_str()) && strlen(newItem) > 0) {
        item.push_back(std::string(newItem));
        selectedIndex = static_cast<int>(item.size()) - 1;
        newItem[0] = '\0';
        //save__to__json(item, filename, extension);
        isLoaded = false; // Force reload to update the combo
    }
}

template<typename T>
void BudgetManager::CreateTable(const char* tableName, std::vector<T>& items, const std::vector<std::string>& tableHeaders, const std::vector<std::string>& tableOrder, const std::string filename, const std::string key, std::function<void(const T&)> editCallback, std::function<void(const T&)> deleteCallback) {
    // Add an extra column for Edit button
    if (ImGui::BeginTable(tableName, tableHeaders.size() + 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        for (int i = 0; i < tableHeaders.size(); ++i) {
            ImGui::TableSetupColumn(tableHeaders[i].c_str());
        }
        ImGui::TableSetupColumn("Edit");
        ImGui::TableSetupColumn("Delete");
        ImGui::TableHeadersRow();
        // Loop over data and populate rows
        for (int i = 0; i < items.size(); ++i) {
            const auto& item = items[i];
            ImGui::TableNextRow();
            nlohmann::json j = item; // Serialize struct to JSON
            int col = 0;
            for (const auto& field : tableOrder) {
                ImGui::TableSetColumnIndex(col++);
                if (j[field].is_number()) {
                    ImGui::TextUnformatted(format_euro(j[field]).c_str());
                } else if (j[field].is_string()) {
                    ImGui::TextUnformatted(j[field].get<std::string>().c_str());
                } else {
                    ImGui::TextUnformatted("?");
                }
            }
            // Add an edit button in the last column
            ImGui::TableSetColumnIndex(tableHeaders.size());
            std::string editLabel = "Edit##" + std::to_string(i);
            if (editCallback && ImGui::Button(editLabel.c_str())) {
                editCallback(item);
            }
            // Add a delete button in a new column
            ImGui::TableSetColumnIndex(tableHeaders.size()+1);
            std::string btnLabel = "Delete##" + std::to_string(i);
            if (deleteCallback && ImGui::Button(btnLabel.c_str())) {
                deleteCallback(item);
                continue;
            }
        }
        ImGui::EndTable();
    }
}

void BudgetManager::PlotBudget() 
{
    ImGui::Begin("Budget Visualsation");
    ImGui::Text("Here, the Savings, Incomes and Expenses are visualized over a selected Time Horizon, Pay Raise and Inflation Rate.");
    ImGui::Text("In the furture, other factors affecting the budget can also be considered, such as passive incomes from i.e. stocks or real estate.");
    ImGui::Separator();
    // Slider for months
    ImGui::Text("Select Time Horizon, Pay Raise and Inflation Rate:");
    static int monthsTmp = getMonths();
    if (ImGui::SliderInt("Time Horizon (Months)", &monthsTmp, 1, 10*12)) {setMonths(monthsTmp); updateMonthlyPlotValues();}
    if (ImGui::SliderInt("Pay Raise Rate (%/year)", &payRaiseRate, 0, 10)) {updateMonthlyPlotValues();}
    if (ImGui::SliderInt("Inflation Rate (%/year)", &inflationRate, 0, 10)) {updateMonthlyPlotValues();}

    if (monthlyAvgExpense == 0.0) {
        load_from_json(monthlyAvgExpense, "../budget.json", "expenses.avgMonthly", "");
        load_from_json(monthlyAvgIncome, "../budget.json", "incomes.avgMonthly", "");
        updateMonthlyPlotValues();
    }

    // Display current total balance
    ImGui::Text("Current Total Bank Balance: %s DKK", format_euro(balanceHistAmount.back()).c_str());
    ImGui::Text("Set a Target Amount (DKK) and see when it's reached:");
    ImGui::InputDouble("Target Amount (DKK)", &targetValue, 10000.0, 1000000.0, "%.0f");

    // Show date when cumulativeBalance surpasses targetValue
    if (targetValue > 0.0 && surpassTargetIdx != -1) {
        time_t t = (time_t)timeMonths[surpassTargetIdx];
        struct tm* tm_info = localtime(&t);
        char dateStr[32];
        strftime(dateStr, sizeof(dateStr), "%d-%m-%Y", tm_info);
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 180, 0, 255)); // Green color
        ImGui::Text("Target reached on: %s", dateStr);
        ImGui::PopStyleColor();
    } else if (targetValue > 0.0) {
        ImGui::Text("Target not reached within selected months.");
    }

    // Plotting
    if (ImPlot::BeginSubplots("Budget Plots", 1, 2, ImVec2(-1,400))) {
        
        if (ImPlot::BeginPlot("Expenses and Incomes")) {
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::SetupAxis(ImAxis_Y1, "Amount (DKK)");
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, p1.Ymax + p1.Ymargin, ImPlotCond_Always);
            ImPlot::SetupAxisFormat(ImAxis_Y1, format_euro_implot);
            ImPlot::SetupAxisLimits(ImAxis_X1, timeMonths.front(), timeMonths.back(), ImPlotCond_Always);
            ImPlot::PlotLine("Incomes"   , timeMonths.data(), monthlyIncomes.data(), months);
            ImPlot::PlotLine("Expenses"  , timeMonths.data(), monthlyExpenses.data(), months);
            ImPlot::PlotLine("Difference", timeMonths.data(), monthlyBalance.data(), months);
            ImPlot::EndPlot();
            
        }
        
        if (ImPlot::BeginPlot("Cumulative Budget")) {
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::SetupAxis(ImAxis_Y1, "Amount (DKK)");
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, p2.Ymax + p2.Ymargin, ImPlotCond_Always);
            ImPlot::SetupAxisFormat(ImAxis_Y1, format_euro_implot);
            ImPlot::SetupAxisLimits(ImAxis_X1, p2.Xmin, p2.Xmax, ImPlotCond_Always);
            ImPlot::PlotLine("Cumulative Incomes"   , timeMonths.data(), cumulativeIncomes.data()   , months);
            ImPlot::PlotLine("Cumulative Expenses"  , timeMonths.data(), cumulativeExpenses.data()  , months);
            ImPlot::PlotLine("Cumulative Difference", timeMonths.data(), cumulativeDifference.data(), months);
            ImPlot::PlotLine("Balance Over Time"    , timeMonths.data(), cumulativeBalance.data()   , months);
            ImPlot::PlotLine("Historical Balance"   , balanceHistTimes.data(), balanceHistAmount.data(), nHistBalances);
            // Plot points with color based on status
            for (int i = 0; i < nHistBalances; ++i) {
                if (balanceStatusHist[i] == "confirmed") {
                    ImPlot::PushStyleColor(ImPlotCol_MarkerFill, plotVisuals.color_confirmed);
                    ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, plotVisuals.markerSize_confirmed);
                    ImPlot::PlotScatter("Historical Balance Points##hidden", &balanceHistTimes[i], &balanceHistAmount[i], 1, ImPlotItemFlags_NoLegend);
                    ImPlot::PopStyleColor(); ImPlot::PopStyleVar();
                } else {
                    ImPlot::PushStyleColor(ImPlotCol_MarkerFill, plotVisuals.color_tentative);
                    ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, plotVisuals.markerSize_tentative);
                    ImPlot::PlotScatter("Historical Balance Points##hidden", &balanceHistTimes[i], &balanceHistAmount[i], 1, ImPlotItemFlags_NoLegend);
                    ImPlot::PopStyleColor(); ImPlot::PopStyleVar();
                }
                
            }
            if (ImPlot::IsPlotHovered()) {
                if (p2_hover.update(nHistBalances, balanceHistTimes, balanceHistAmount, p2.Xrange, p2.Yrange)) {
                    ImPlot::PushStyleColor(ImPlotCol_MarkerFill, plotVisuals.color_hovered); // Bright yellow
                    ImPlot::PushStyleVar(ImPlotStyleVar_MarkerSize, plotVisuals.markerSize_hovered); // Larger size
                    ImPlot::PlotScatter("Hovered Historical Point##hidden", &balanceHistTimes[p2_hover.closestIdx], &balanceHistAmount[p2_hover.closestIdx], 1, ImPlotItemFlags_NoLegend);
                    ImPlot::PopStyleVar(); ImPlot::PopStyleColor();
                    ImGui::BeginTooltip(); // Tooltip
                    ImGui::Text("Date: %s\nValue: %.2f\nStatus: %s"
                        , getDateStr(balanceHistTimes[p2_hover.closestIdx]).c_str()
                        , balanceHistAmount[p2_hover.closestIdx]
                        , balanceStatusHist[p2_hover.closestIdx].c_str());
                    ImGui::EndTooltip();
                }
            }
            // Add marker for target value
            if (targetValue > 0.0 && surpassTargetIdx != -1) {
                ImPlot::PlotScatter("Target Surpassed", &timeMonths[surpassTargetIdx], &cumulativeBalance[surpassTargetIdx], 1);
                char markerLabel[64];
                auto dateStr = getDateStr(timeMonths[surpassTargetIdx]);
                snprintf(markerLabel, sizeof(markerLabel), "Target: %s\nDate: %s", format_euro(cumulativeBalance[surpassTargetIdx]).c_str(), dateStr.c_str());
                ImPlot::TagX(timeMonths[surpassTargetIdx], ImVec4(0,0.5,0,1), "%s\n%s", format_euro(cumulativeBalance[surpassTargetIdx]).c_str(), dateStr.c_str());
            }
            
            ImPlot::EndPlot();
        }
        ImPlot::EndSubplots();
    }
    ImGui::Separator();
    ImGui::Text("Pie Chart Distribution of Incomes and Expenses:");
    static ImPlotPieChartFlags flags = ImPlotPieChartFlags_Normalize|ImPlotPieChartFlags_IgnoreHidden|ImPlotPieChartFlags_Exploding;
    // Aggregation mode selection
    enum AggregationMode { ByCategory = 0, ByPerson, ByTypeAccount, ByTypeExpense };
    static int aggregationMode = 0;
    ImGui::Text("Aggregate Expenses Pie Chart By:");
    ImGui::RadioButton("Category", &aggregationMode, ByCategory); ImGui::SameLine();
    ImGui::RadioButton("Person", &aggregationMode, ByPerson); ImGui::SameLine();
    ImGui::RadioButton("TypeAccount", &aggregationMode, ByTypeAccount); ImGui::SameLine();
    ImGui::RadioButton("TypeExpense", &aggregationMode, ByTypeExpense);

    if (ImPlot::BeginSubplots("Income and Expenses Pie Charts", 1, 2, ImVec2(-1,600))) {
    if (ImPlot::BeginPlot("Monthly Incomes Pie")) {
            // Prepare data
            std::vector<const char*> labels;
            std::vector<double> values;
            for (const auto& income : incomes) {
                labels.push_back(income.source.c_str());
                values.push_back(income.amountNet_month);
            }
            // Plot pie chart
            ImPlot::SetupAxesLimits(0, 1, 0, 1, ImPlotCond_Always);
            ImPlot::SetupLegend(ImPlotLocation_NorthEast);
            ImPlot::PlotPieChart(labels.data(), values.data(), values.size(), 0.5, 0.5, 0.4, "%.0fkr", 90, flags);
            ImPlot::EndPlot();
        }

    if (ImPlot::BeginPlot("Monthly Expenses Pie")) {
            // Prepare data
            std::map<std::string, double> groupTotals;
            for (const auto& expense : expenses) {
                std::string key;
                switch (aggregationMode) {
                    case ByCategory:    key = expense.category; break;
                    case ByPerson:      key = expense.person; break;
                    case ByTypeAccount: key = expense.typeAccount; break;
                    case ByTypeExpense: key = expense.typeExpense; break;
                    default:            key = expense.category; break;
                }
                groupTotals[key] += expense.amountNet_month;
            }
            // Prepare labels and values for the pie chart
            std::vector<const char*> labels;
            std::vector<double> values;
            for (const auto& pair : groupTotals) {
                labels.push_back(pair.first.c_str());
                values.push_back(pair.second);
            }
            // Plot the pie chart
            ImPlot::SetupAxesLimits(0, 1, 0, 1, ImPlotCond_Always);
            ImPlot::SetupLegend(ImPlotLocation_NorthWest);
            ImPlot::PlotPieChart(labels.data(), values.data(), values.size(), 0.5, 0.5, 0.4, "%.0fkr", 90, flags);
            ImPlot::EndPlot();
        }
        ImPlot::EndSubplots();
    }
    ImGui::End();
    }
} // namespace budget