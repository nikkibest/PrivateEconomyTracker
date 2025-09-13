#ifndef BUDGET_H
#define BUDGET_H

#include <vector>
#include "data/budgetData.h"
#include "utils/numbersFormat.h"

namespace budget {
    void ShowBankBalanceInput(std::vector<BankBalance>& bankBalance);
    void ShowIncomeInput(std::vector<Income>& incomes);
    void ShowExpenseInput(std::vector<Expense>& expenses);
    
    template<typename T>
    void ComputeDependentValues(std::vector<T>& items, const std::string filename, const std::string keyStart);

    void CreateComboWithDeleteAndAdd(std::vector<std::string>& item,  bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem);
    
    template<typename T>
    void CreateTable(const char*  tableName, std::vector<T>& items, const std::vector<std::string>& tableHeaders, const std::vector<std::string>& tableOrder, const std::string filename, const std::string key);

    void PlotBudget(const std::vector<Expense>& expenses, const std::vector<Income>& incomes);
}


#endif // BUDGET_H