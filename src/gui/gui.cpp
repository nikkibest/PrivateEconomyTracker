#include "gui.h"
#include "imgui.h"
#include "implot.h"
#include <iostream>

namespace gui {

void ShowExpenseInput(std::vector<Expense>& expenses) {
    ImGui::Begin("Expenses");
    ImGui::Text("Add all household expenses here.");
    const std::string filename = "../expenses.json";

    static bool loaded = false;
    if (!loaded) {
        // Load incomes from file or initialize as needed
        load_expenses(expenses, "../expenses.json", "expenses");
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
    std::string extension = "categories";
    static char newCategory[128] = "";
    gui::CreateComboWithDeleteAndAdd(categories, isLoadedCategories, idxSelectedCategory, "category", filename, extension, newCategory);
    strncpy(category, categories[idxSelectedCategory].c_str(), sizeof(category) - 1);
    category[sizeof(category) - 1] = '\0';

    // Persons
    static std::vector<std::string> persons;
    static bool isLoadedPersons = false;
    static int  idxSelectedPerson = 0;
    extension = "persons";
    static char newPerson[128] = "";
    gui::CreateComboWithDeleteAndAdd(persons, isLoadedPersons, idxSelectedPerson, "person", filename, extension, newPerson);
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
            save_expenses(expenses, "../expenses.json", "expenses");
            source[0] = '\0';
            nrAnnualPayments = 0.0f;
            amount = 0.0f;
            monthly = 0.0f;
            yearly = 0.0f;
        }
    }
    ImGui::Separator();
    ImGui::Text("All Expenses:");
    double totalYearly = 0.0;
    nlohmann::json j = expenses[0]; // e is your struct
    size_t num_fields = j.size(); // Number of fields serialized to JSON
    if (ImGui::BeginTable("ExpenseTable", (num_fields+1), ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Source");
        ImGui::TableSetupColumn("Category");
        ImGui::TableSetupColumn("Person");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Account");
        ImGui::TableSetupColumn("Nr. Annual Payments");
        ImGui::TableSetupColumn("Amount (DKK)");
        ImGui::TableSetupColumn("Monthly (DKK)");
        ImGui::TableSetupColumn("Yearly (DKK)");
        ImGui::TableSetupColumn("Delete");
        ImGui::TableHeadersRow();

        for (int i = 0; i < expenses.size(); ++i) {
            const auto& expense = expenses[i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(expense.source.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(expense.category.c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(expense.person.c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(expense.typeExpense.c_str());
            ImGui::TableSetColumnIndex(4);
            ImGui::TextUnformatted(expense.typeAccount.c_str());
            ImGui::TableSetColumnIndex(5);
            ImGui::TextUnformatted(format_euro(expense.nrAnnualPayments).c_str());
            ImGui::TableSetColumnIndex(6);
            ImGui::TextUnformatted(format_euro(expense.amountNet).c_str());
            ImGui::TableSetColumnIndex(7);
            ImGui::TextUnformatted(format_euro(expense.amountNet_month).c_str());
            ImGui::TableSetColumnIndex(8);
            ImGui::TextUnformatted(format_euro(expense.amountNet_year).c_str());
            totalYearly += expense.amountNet_year;
            
            // Add a delete button in a new column
            ImGui::TableSetColumnIndex(9);
            std::string btnLabel = "Delete##" + std::to_string(i);
            if (ImGui::Button(btnLabel.c_str())) {
                expenses.erase(expenses.begin() + i);
                save_expenses(expenses, "../expenses.json","expenses");
                // To avoid skipping the next entry after erase
                --i;
                continue;
            }
        }
        ImGui::EndTable();
    }
    ImGui::Text("All Expenses:");
    if (ImGui::BeginTable("ExpenseTableSummary", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        double avgMonthly = totalYearly / 12.0f;
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
    ImGui::End();
}

void ShowIncomeInput(std::vector<Income>& incomes) {    
    static bool loaded = false;
    if (!loaded) {
        // Load incomes from file or initialize as needed
        load_incomes(incomes, "../incomes.json");
        loaded = true;
    }
    static char source[64] = "";
    static float nrAnnualPayments = 0.0f;
    static float monthly = 0.0f;
    static float yearly = 0.0f;
    static float amount = 0.0f;
    ImGui::Begin("Incomes");
    ImGui::Text("Add all household incomes here.");
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
            save_incomes(incomes, "../incomes.json");
            source[0] = '\0';
            nrAnnualPayments = 0.0f;
            amount = 0.0f;
            monthly = 0.0f;
            yearly = 0.0f;
        }
    }

    ImGui::Separator();
    ImGui::Text("All Incomes:");
    double totalYearly = 0.0;
    if (ImGui::BeginTable("IncomeTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Source");
        ImGui::TableSetupColumn("Nr. Annual Payments");
        ImGui::TableSetupColumn("Amount (DKK)");
        ImGui::TableSetupColumn("Monthly (DKK)");
        ImGui::TableSetupColumn("Yearly (DKK)");
        ImGui::TableSetupColumn("Delete");
        ImGui::TableHeadersRow();

        for (int i = 0; i < incomes.size(); ++i) {
            const auto& income = incomes[i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(income.source.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(format_euro(income.nrAnnualPayments).c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(format_euro(income.amountNet).c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(format_euro(income.amountNet_month).c_str());
            ImGui::TableSetColumnIndex(4);
            ImGui::TextUnformatted(format_euro(income.amountNet_year).c_str());
            totalYearly += income.amountNet_year;
            
            // Add a delete button in a new column
            ImGui::TableSetColumnIndex(5);
            std::string btnLabel = "Delete##" + std::to_string(i);
            if (ImGui::Button(btnLabel.c_str())) {
                incomes.erase(incomes.begin() + i);
                save_incomes(incomes, "../incomes.json");
                // To avoid skipping the next entry after erase
                --i;
                continue;
            }
        }
        ImGui::EndTable();
    }
    ImGui::Text("All Incomes:");
    if (ImGui::BeginTable("IncomeTableSummary", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        double avgMonthly = totalYearly / 12.0f;
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
    // Calculate cumulative savings for 12 months
    constexpr int months = 12;
    static double savings[months] = {0};
    static double xticks[months] = {0};
    double monthlySum = 0.0;
    for (const auto& income : incomes) {
        monthlySum += income.amountNet_month;
    }
    for (int i = 0; i < months; ++i) {
        savings[i] = monthlySum * (i + 1);
        xticks[i] = i;
    }
    if (ImPlot::BeginPlot("Expected Savings Accumulation", ImVec2(-1,300))) {
        static const char* months_labels[months] = {
            "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
        };
        ImPlot::SetupAxes("Month", "Savings (DKK)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        // ImPlot::SetupAxisTicks(ImAxis_X1, 0, months - 1, months_labels, months);
        ImPlot::SetupAxisTicks(ImAxis_X1, xticks, months, months_labels);
        // Use format_euro for y-axis tick labels
        ImPlot::SetupAxisFormat(ImAxis_Y1, [](double value, char* buff, int size) {
            std::string euro = format_euro(value);
            strncpy(buff, euro.c_str(), size);
            buff[size - 1] = '\0';
        });
        ImPlot::PlotLine("Savings", savings, months);
        ImPlot::EndPlot();
    }
    ImGui::End();
}


void CreateComboWithDeleteAndAdd(std::vector<std::string>& item, bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem)
{
    if (!isLoaded) {
        // Load categories from expenses.json file
        isLoaded = true;   
        load_items(item, filename, extension);
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
            save_items(item, filename, extension);
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
        save_items(item, filename, extension);
        isLoaded = false; // Force reload to update the combo
    }
}

void plotExpensesAndIncomes(const std::vector<Expense>& expenses, const std::vector<Income>& incomes) 
{
    ImGui::Begin("Budget Visualsation");
    // Calculate cumulative expenses and incomes for 12 months
    constexpr int months = 12;
    static double cumulativeExpenses[months] = {0};
    static double cumulativeIncomes[months] = {0};
    static double cumulativeDifference[months] = {0};
    static double xticks[months] = {0};
    double monthlyExpenseSum = 0.0;
    for (const auto& expense : expenses) {
        monthlyExpenseSum += expense.amountNet_month;
    }
    double monthlyIncomeSum = 0.0;
    for (const auto& income : incomes) {
        monthlyIncomeSum += income.amountNet_month;
    }
    for (int i = 0; i < months; ++i) {
        cumulativeExpenses[i] = monthlyExpenseSum * (i + 1);
        cumulativeIncomes[i] = monthlyIncomeSum * (i + 1);
        cumulativeDifference[i] = cumulativeIncomes[i] - cumulativeExpenses[i];
        xticks[i] = i;
    }
    if (ImPlot::BeginPlot("Cumulative Expenses and Incomes", ImVec2(-1,300))) {
        static const char* months_labels[months] = {
            "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
        };
        ImPlot::SetupAxes("Month", "Amount (DKK)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        // ImPlot::SetupAxisTicks(ImAxis_X1, 0, months - 1, months_labels, months);
        ImPlot::SetupAxisTicks(ImAxis_X1, xticks, months, months_labels);
        ImPlot::PlotLine("Cumulative Expenses", cumulativeExpenses, months);
        ImPlot::PlotLine("Cumulative Incomes", cumulativeIncomes, months);
        ImPlot::PlotLine("Cumulative Surplus", cumulativeDifference, months);
        ImPlot::EndPlot();
    }
    ImGui::End();
}
} // namespace gui