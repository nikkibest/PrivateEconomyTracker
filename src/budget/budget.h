#ifndef BUDGET_H
#define BUDGET_H

#include <vector>
#include "data/budgetData.h"
#include "utils/numbersFormat.h"

namespace budget {
    // void ShowBankBalanceInout(std::vector<BankBalance>& bankBalance);
    void ShowIncomeInput(std::vector<Income>& incomes);
    void ShowExpenseInput(std::vector<Expense>& expenses);

    void CreateComboWithDeleteAndAdd(std::vector<std::string>& item,  bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem);

    void plotBudget(const std::vector<Expense>& expenses, const std::vector<Income>& incomes);
}


#endif // BUDGET_H