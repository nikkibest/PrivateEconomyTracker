#include "budget.h"
#include "imgui.h"
#include "implot.h"
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
    static char source[64] = "";
    static float amount = 0.0f;
    ImGui::Begin("Bank Balance");
    ImGui::Text("Add a new household bank balance here.");
    ImGui::InputText("Source", source, IM_ARRAYSIZE(source));
    ImGui::InputFloat("Amount", &amount);

    if (ImGui::Button("Add Bank Balance")) {
        if (strlen(source) > 0 && amount > 0.0f) {
            bankBalance.push_back(BankBalance{
                source, 
                static_cast<double>(amount), 
            });
            save__to__json(bankBalance, filename, "balance.balance");
            source[0] = '\0';
            amount = 0.0f;
            // Update summary values
            ComputeDependentValues(bankBalance, filename, "balance");
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
    CreateTable("IncomeTable", bankBalance, bankbalanceTableHeader, bankbalanceTableOrder, filename, "balance.balance");    
    ImGui::End();
}

void ShowIncomeInput(std::vector<Income>& incomes) {    
    static bool loaded = false;
    
    if (!loaded) {
        // Load incomes from file or initialize as needed
        load_from_json(incomes, filename, "incomes.incomes");
        loaded = true;
    }
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

    if (ImGui::Button("Add Income")) {
        if (strlen(source) > 0 && amount > 0.0f) {
            yearly = amount * nrAnnualPayments;
            monthly = yearly / 12.0f;
            incomes.push_back(Income{
                source, 
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
    CreateTable("IncomeTable",incomes, incomeTableHeader, incomeTableOrder, filename, "incomes.incomes");    
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
    ImGui::Begin("Expenses");
    ImGui::Text("Add a new household expense here.");
    static bool loaded = false;
    if (!loaded) {
        load_from_json(expenses, filename, "expenses.expenses");
        loaded = true;
    }
    static char source[64] = "";
    static char category[64] = "";
    static char person[64] = "";
    static char typeExpense[64] = "";
    static char typeAccount[64] = "";
    static float nrAnnualPayments = 0.0f;
    static float monthly = 0.0f;
    static float yearly = 0.0f;
    static float amount = 0.0f;
    ImGui::InputText("Source", source, IM_ARRAYSIZE(source));
    
    // Categories
    static std::vector<std::string> categories;
    static bool isLoadedCategories = false;
    static int idxSelectedCategory = 0;
    std::string extension = "expenses.categories";
    static char newCategory[64] = "";
    budget::CreateComboWithDeleteAndAdd(categories, isLoadedCategories, idxSelectedCategory, "category", filename, extension, newCategory);
    strncpy(category, categories[idxSelectedCategory].c_str(), sizeof(category) - 1);
    category[sizeof(category) - 1] = '\0';

    // Persons
    static std::vector<std::string> persons;
    static bool isLoadedPersons = false;
    static int  idxSelectedPerson = 0;
    extension = "persons";
    static char newPerson[64] = "";
    budget::CreateComboWithDeleteAndAdd(persons, isLoadedPersons, idxSelectedPerson, "person", filename, extension, newPerson);
    strncpy(person, persons[idxSelectedPerson].c_str(), sizeof(person) - 1);
    person[sizeof(person) - 1] = '\0';

    // Must/Expenditure Combo
    static const char* mustExpenditureOptions[] = { "Must", "Forbrug" };
    static int mustExpenditureIdx = 0;
    ImGui::Combo("Expense Type", &mustExpenditureIdx, mustExpenditureOptions, IM_ARRAYSIZE(mustExpenditureOptions));
    strncpy(typeExpense, mustExpenditureOptions[mustExpenditureIdx], sizeof(typeExpense) - 1);
    typeExpense[sizeof(typeExpense) - 1] = '\0';

    // Account Type Combo
    static const char* accountTypeOptions[] = { "Budgetkonto", "Forbrugskonto", "Lønkonto" };
    static int accountTypeIdx = 0;
    ImGui::Combo("Account Type", &accountTypeIdx, accountTypeOptions, IM_ARRAYSIZE(accountTypeOptions));
    strncpy(typeAccount, accountTypeOptions[accountTypeIdx], sizeof(typeAccount) - 1);
    typeAccount[sizeof(typeAccount) - 1] = '\0';

    ImGui::InputFloat("Nr. Annual Payments", &nrAnnualPayments);
    ImGui::InputFloat("Amount", &amount);
    // Add Expense button
    if (ImGui::Button("Add Expense")) {
        if (strlen(source) > 0 && amount > 0.0f) {
            yearly = amount * nrAnnualPayments;
            monthly = yearly / 12.0f;
            expenses.push_back(Expense{
                source, 
                category,
                person,
                typeExpense,
                typeAccount,
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
    CreateTable("ExpenseTable", expenses, expenseTableHeader, expenseTableOrder, filename, "expenses.expenses");
    ImGui::End();
}

void CreateComboWithDeleteAndAdd(std::vector<std::string>& item, bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem)
{
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
void CreateTable(const char* tableName, std::vector<T>& items, const std::vector<std::string>& tableHeaders, const std::vector<std::string>& tableOrder, const std::string filename, const std::string key) {
    if (ImGui::BeginTable(tableName, tableHeaders.size(), ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        for (int i = 0; i < tableHeaders.size(); ++i) {
            ImGui::TableSetupColumn(tableHeaders[i].c_str());
        }
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
            // Add a delete button in a new column
            ImGui::TableSetColumnIndex(tableHeaders.size()-1);
            std::string btnLabel = "Delete##" + std::to_string(i);
            if (ImGui::Button(btnLabel.c_str())) {
                items.erase(items.begin() + i);
                save__to__json(items, filename, key);
                // Update summary values
                ComputeDependentValues(items, filename, key.substr(0, key.find('.')));
                // To avoid skipping the next entry after erase
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