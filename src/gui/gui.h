#ifndef GUI_H
#define GUI_H

#include <vector>
#include "model/income.h"
#include "model/expense.h"
#include "utils/numbers.h"

namespace gui {

    void ShowIncomeInput(std::vector<Income>& incomes);
    void ShowExpenseInput(std::vector<Expense>& expenses);

    void CreateComboWithDeleteAndAdd(std::vector<std::string>& item,  bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem);

    void plotExpensesAndIncomes(const std::vector<Expense>& expenses, const std::vector<Income>& incomes);
}


#endif // GUI_H