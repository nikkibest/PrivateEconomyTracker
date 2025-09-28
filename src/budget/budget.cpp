#include "budget.h"
#include "imgui.h"
#include "implot.h"
#include <set>
#include <iostream>
#define CHECKBOX_FLAG(flags, flag) ImGui::CheckboxFlags(#flag, (unsigned int*)&flags, flag)

namespace budget {
void ShowBankBalanceInput(std::vector<BankBalance>& bankBalance) {    
    static bool loaded = false;
    if (!loaded) {
        // Load incomes from file or initialize as needed
        load_from_json(bankBalance, filename, "balance.balance");
        loaded = true;
    }
    static int editId = 0;
    static char source[64] = "";
    static float amount = 0.0f;
    ImGui::Begin("Bank Balance");
    ImGui::Text("Add a new household bank balance here.");
    ImGui::InputText("Source", source, IM_ARRAYSIZE(source));
    ImGui::InputFloat("Amount", &amount);

     // Helper to copy income data to input fields
    auto SetEditBalance = [&](const BankBalance& bal) {
        editId = bal.id;
        strncpy(source, bal.source.c_str(), sizeof(source) - 1);
        source[sizeof(source) - 1] = '\0';
        amount = static_cast<float>(bal.amountNet);
    };

    if (editId == 0) {
        if (ImGui::Button("Add Bank Balance")) {
            if (strlen(source) > 0 && amount > 0.0f) {
                // Find the first unused positive integer ID
                std::set<int> usedIds;
                for (const auto& bal : bankBalance) {
                    usedIds.insert(bal.id);
                }
                int newId = 1;
                while (usedIds.count(newId)) {
                    ++newId;
                }
                bankBalance.push_back(BankBalance{
                    newId,
                    std::string(source),
                    static_cast<double>(amount),
                });
                save__to__json(bankBalance, filename, "balance.balance");
                source[0] = '\0';
                amount = 0.0f;
                // Update summary values
                ComputeDependentValues(bankBalance, filename, "balance");
            }
        }
    } else {
        if (ImGui::Button("Save Changes")) {
            if (strlen(source) > 0 && amount > 0.0f) {
                for (auto& bal : bankBalance) {
                    if (bal.id == editId) {
                        bal.source = source;
                        bal.amountNet = static_cast<double>(amount);
                        break;
                    }
                }
                save__to__json(bankBalance, filename, "balance.balance");
                source[0] = '\0';
                amount = 0.0f;
                editId = 0;
                ComputeDependentValues(bankBalance, filename, "balance");
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel Editting")) {
            source[0] = '\0';
            amount = 0.0f;
            editId = 0;
        }
    }
    
    ImGui::Separator();
    ImGui::Text("Summary of all Bank Balances:");
    if (ImGui::BeginTable("BankBalanceTableSummary", 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        double totalBalance = 0.0;
        load_from_json(totalBalance, "../budget.json", "balance.totalBalance");
        ImGui::TableSetupColumn("Total Bank Balance (DKK)");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(format_euro(totalBalance).c_str());
        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::Text("All Bank Balances:");
    CreateTable<BankBalance>("IncomeTable", bankBalance, bankbalanceTableHeader, bankbalanceTableOrder, filename, "balance.balance", SetEditBalance);    
    ImGui::End();
}

void ShowIncomeInput(std::vector<Income>& incomes) {    
    static bool loaded = false;
    
    if (!loaded) {
        // Load incomes from file or initialize as needed
        load_from_json(incomes, filename, "incomes.incomes");
        loaded = true;
    }
    // Static for editing
    static int editId = 0;
    static char source[64] = "";
    static float nrAnnualPayments = 0.0f;
    static float monthly = 0.0f;
    static float yearly = 0.0f;
    static float amount = 0.0f;
    ImGui::Begin("Incomes");
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
                save__to__json(incomes, filename, "incomes.incomes");
                source[0] = '\0';
                nrAnnualPayments = 0.0f;
                amount = 0.0f;
                monthly = 0.0f;
                yearly = 0.0f;
                // Update summary values
                ComputeDependentValues(incomes, filename, "incomes");
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
                save__to__json(incomes, filename, "incomes.incomes");
                source[0] = '\0';
                nrAnnualPayments = 0.0f;
                amount = 0.0f;
                monthly = 0.0f;
                yearly = 0.0f;
                editId = 0;
                ComputeDependentValues(incomes, filename, "incomes");
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
        load_from_json(totalYearly, "../budget.json", "incomes.totalYearly");
        load_from_json(avgMonthly, "../budget.json", "incomes.avgMonthly");

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
    CreateTable<Income>("IncomeTable",incomes, incomeTableHeader, incomeTableOrder, filename, "incomes.incomes", SetEditIncome);    
    ImGui::End();
}

template<typename T>
void ComputeDependentValues(std::vector<T>& items, const std::string filename, const std::string keyStart) {
    if constexpr (std::is_same_v<T, BankBalance>) {
        // Update summary value for BankBalance
        double totalBalance = 0.0;
        for (const auto& item : items) {
            totalBalance += item.amountNet; // Use the correct member
        }
        save__to__json(totalBalance, filename, keyStart+".totalBalance");
    } else {
        // Update summary values for Income or Expense
        double totalYearly = 0.0;
        for (const auto& item : items) {
            totalYearly += item.amountNet_year;
        }
        double avgMonthly = totalYearly / 12.0f;
        save__to__json(totalYearly, filename, keyStart+".totalYearly");
        save__to__json(avgMonthly, filename, keyStart+".avgMonthly");
    }
}

void ShowExpenseInput(std::vector<Expense>& expenses) {
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
    ImGui::Begin("Expenses");
    ImGui::Text("Add a new household expense here.");
    static bool loaded = false;
    if (!loaded) {
        load_from_json(expenses, filename, "expenses.expenses");
        loaded = true;
    }
    ImGui::InputText("Source", source, IM_ARRAYSIZE(source));
    // Categories
    extension = "expenses.categories";
    budget::CreateComboWithDeleteAndAdd(categories, isLoadedCategories, idxSelectedCategory, "category", filename, extension, newCategory);
    strncpy(category, categories[idxSelectedCategory].c_str(), sizeof(category) - 1);
    category[sizeof(category) - 1] = '\0';
    // Persons
    extension = "persons";
    budget::CreateComboWithDeleteAndAdd(persons, isLoadedPersons, idxSelectedPerson, "person", filename, extension, newPerson);
    strncpy(person, persons[idxSelectedPerson].c_str(), sizeof(person) - 1);
    person[sizeof(person) - 1] = '\0';
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
                save__to__json(expenses, filename, "expenses.expenses");
                source[0] = '\0';
                nrAnnualPayments = 0.0f;
                amount = 0.0f;
                monthly = 0.0f;
                yearly = 0.0f;
                // Update summary values
                ComputeDependentValues(expenses, filename, "expenses");
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
                save__to__json(expenses, filename, "expenses.expenses");
                source[0] = '\0';
                nrAnnualPayments = 0.0f;
                amount = 0.0f;
                monthly = 0.0f;
                yearly = 0.0f;
                editId = 0;
                ComputeDependentValues(expenses, filename, "expenses");
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
        load_from_json(totalYearly, "../budget.json", "expenses.totalYearly");
        load_from_json(avgMonthly, "../budget.json", "expenses.avgMonthly");

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
    CreateTable<Expense>("ExpenseTable", expenses, expenseTableHeader, expenseTableOrder, filename, "expenses.expenses", SetEditExpense);
    ImGui::End();
}

void CreateComboWithDeleteAndAdd(std::vector<std::string>& item, bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem)
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
        load_from_json(item, filename, extension);
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
            save__to__json(item, filename, extension);
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
        save__to__json(item, filename, extension);
        isLoaded = false; // Force reload to update the combo
    }
}

template<typename T>
void CreateTable(const char* tableName, std::vector<T>& items, const std::vector<std::string>& tableHeaders, const std::vector<std::string>& tableOrder, const std::string filename, const std::string key, std::function<void(const T&)> editCallback) {
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
            if (ImGui::Button(btnLabel.c_str())) {
                items.erase(items.begin() + i);
                save__to__json(items, filename, key);
                // Update summary values
                ComputeDependentValues(items, filename, key.substr(0, key.find('.')));
                --i;
                continue;
            }
        }
        ImGui::EndTable();
    }
}

void PlotBudget(const std::vector<Expense>& expenses, const std::vector<Income>& incomes) 
{
    ImGui::Begin("Budget Visualsation");
    ImGui::Text("Here, the Savings, Incomes and Expenses are visualized over a selected Time Horizon, Pay Raise and Inflation Rate.");
    ImGui::Text("In the furture, other factors affecting the budget can also be considered, such as passive incomes from i.e. stocks or real estate.");
    ImGui::Separator();
    // Slider for months
    ImGui::Text("Select Time Horizon, Pay Raise and Inflation Rate:");
    static int months = 12;
    ImGui::SliderInt("Time Horizon (Months)", &months, 1, 10*12);
    static int payRaiseRate = 2;
    ImGui::SliderInt("Pay Raise Rate (%/year)", &payRaiseRate, 0, 10);
    static int inflationRate = 2;
    ImGui::SliderInt("Inflation Rate (%/year)", &inflationRate, 0, 10);

    // Dynamically allocate arrays based on months
    std::vector<double> monthlyExpenses(months, 0.0);
    std::vector<double> monthlyIncomes(months, 0.0);
    std::vector<double> monthlyBalance(months, 0.0);
    std::vector<double> cumulativeExpenses(months, 0.0);
    std::vector<double> cumulativeIncomes(months, 0.0);
    std::vector<double> cumulativeDifference(months, 0.0);
    std::vector<double> cumulativeBalance(months, 0.0);
    static double now = (double)time(nullptr);
    std::vector<double> time(months, 0.0);
    double monthlyAvgExpense, monthlyAvgIncome, totalBalance;
    load_from_json(totalBalance, "../budget.json", "balance.totalBalance");
    load_from_json(monthlyAvgExpense, "../budget.json", "expenses.avgMonthly");
    load_from_json(monthlyAvgIncome, "../budget.json", "incomes.avgMonthly");

    // Display current total balance
    ImGui::Text("Current Total Bank Balance: %s DKK", format_euro(totalBalance).c_str());
    // Input field for target value
    ImGui::Text("Set a Target Amount (DKK) and see when it's reached:");
    static double targetValue = 1000000.0;
    ImGui::InputDouble("Target Amount (DKK)", &targetValue, 10000.0, 1000000.0, "%.0f");

    int surpassIdx = -1;
    for (int i = 0; i < months; ++i) {
        monthlyExpenses     [i]   = monthlyAvgExpense;
        monthlyIncomes      [i]   = monthlyAvgIncome;
        monthlyBalance      [i]   = monthlyAvgIncome-monthlyAvgExpense;
        cumulativeExpenses  [i]   = monthlyAvgExpense * (i);
        cumulativeIncomes   [i]   = monthlyAvgIncome * (i);
        cumulativeDifference[i]   = cumulativeIncomes[i] - cumulativeExpenses[i];
        cumulativeBalance   [i]   = totalBalance + cumulativeDifference[i];
        time[i] = now + i * 30.0 * 24.0 * 3600.0; // Approximate month as 30 days
        if (i%12==0 && i!=0) {
            // Apply inflation adjustment at the end of each year
            monthlyAvgExpense *= (1.0 + inflationRate / 100.0);
            monthlyAvgIncome  *= (1.0 + payRaiseRate / 100.0);
        }
        if (surpassIdx == -1 && cumulativeBalance[i] >= targetValue) {
            surpassIdx = i;
        }
    }
    // Show date when cumulativeBalance surpasses targetValue
    if (targetValue > 0.0 && surpassIdx != -1) {
        time_t t = (time_t)time[surpassIdx];
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

            // Combine both vectors
            std::vector<double> allMonthlyValues;
            allMonthlyValues.reserve(monthlyExpenses.size() + monthlyIncomes.size() + monthlyBalance.size());
            allMonthlyValues.insert(allMonthlyValues.end(), monthlyExpenses.begin(), monthlyExpenses.end());
            allMonthlyValues.insert(allMonthlyValues.end(), monthlyIncomes.begin(), monthlyIncomes.end());
            allMonthlyValues.insert(allMonthlyValues.end(), monthlyBalance.begin(), monthlyBalance.end());
            
            // Compute min and max
            static double maxY, range, margin;
            maxY = *std::max_element(allMonthlyValues.begin(), allMonthlyValues.end());
            range = maxY - 0;
            margin = range * 0.05;
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, maxY + margin, ImPlotCond_Always);
            ImPlot::SetupAxisFormat(ImAxis_Y1, format_euro_implot);
            // Set x-axis limits to fit the selected months
            if (months > 0) {
                ImPlot::SetupAxisLimits(ImAxis_X1, time.front(), time.back(), ImPlotCond_Always);
            }
            ImPlot::PlotLine("Incomes", time.data(), monthlyIncomes.data(), months);
            ImPlot::PlotLine("Expenses", time.data(), monthlyExpenses.data(), months);
            ImPlot::PlotLine("Difference", time.data(), monthlyBalance.data(), months);
            ImPlot::EndPlot();
            
        }
        
        if (ImPlot::BeginPlot("Cumulative Budget")) {
            ImPlot::SetupAxisScale(ImAxis_X1, ImPlotScale_Time);
            ImPlot::SetupAxis(ImAxis_Y1, "Amount (DKK)");
            std::vector<double> allCumulativeValues;
            allCumulativeValues.reserve(cumulativeExpenses.size() + cumulativeIncomes.size() + cumulativeDifference.size() + cumulativeBalance.size());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeExpenses.begin(), cumulativeExpenses.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeIncomes.begin(), cumulativeIncomes.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeDifference.begin(), cumulativeDifference.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeBalance.begin(), cumulativeBalance.end());
            
            // Compute min and max
            static double minY, maxY, range, margin;
            maxY = *std::max_element(allCumulativeValues.begin(), allCumulativeValues.end());
            minY = *std::min_element(allCumulativeValues.begin(), allCumulativeValues.end());
            range = maxY - minY;
            margin = range * 0.05;
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, maxY + margin, ImPlotCond_Always);
            ImPlot::SetupAxisFormat(ImAxis_Y1, format_euro_implot);
            // Set x-axis limits to fit the selected months
            if (months > 0) {
                ImPlot::SetupAxisLimits(ImAxis_X1, time.front(), time.back(), ImPlotCond_Always);
            }
            
            ImPlot::PlotLine("Cumulative Incomes", time.data(), cumulativeIncomes.data(), months);
            ImPlot::PlotLine("Cumulative Expenses", time.data(), cumulativeExpenses.data(), months);
            ImPlot::PlotLine("Cumulative Difference", time.data(), cumulativeDifference.data(), months);
            ImPlot::PlotLine("Balance Over Time", time.data(), cumulativeBalance.data(), months);
            
            // Add marker for target value
            if (targetValue > 0.0 && surpassIdx != -1) {
                ImPlot::PlotScatter("Target Surpassed", &time[surpassIdx], &cumulativeBalance[surpassIdx], 1);
                char markerLabel[64];
                time_t t = (time_t)time[surpassIdx];
                struct tm* tm_info = localtime(&t);
                char dateStr[32];
                strftime(dateStr, sizeof(dateStr), "%d-%m-%Y", tm_info);
                snprintf(markerLabel, sizeof(markerLabel), "Target: %s\nDate: %s", format_euro(cumulativeBalance[surpassIdx]).c_str(), dateStr);
                ImPlot::TagX(time[surpassIdx], ImVec4(0,0.5,0,1), "%s\n%s", format_euro(cumulativeBalance[surpassIdx]).c_str(), dateStr);
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