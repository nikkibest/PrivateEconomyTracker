#ifndef BUDGET_H
#define BUDGET_H

#include <vector>
#include "data/budgetData.h"
#include "utils/numbersFormat.h"

namespace budget {
    // BudgetManager class to manage totalBalance
    class BudgetManager {
    private:
        double totalBalance = 0.0;
    public:
        double getTotalBalance() const { return totalBalance; }
        void   setTotalBalance(double value) { totalBalance = value; }

        // auto getToday();
        void ShowBankBalanceInput(std::vector<BankBalance>& bankBalance);
        void ShowIncomeInput(std::vector<Income>& incomes);
        void ShowExpenseInput(std::vector<Expense>& expenses);
        
        template<typename T>
        void ComputeDependentValues(std::vector<T>& items, const std::string filename, const std::string keyStart, const std::string& date, const std::string& status);

        void CreateComboWithDeleteAndAdd(std::vector<std::string>& item,  bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem);
        
        template<typename T>
        void CreateTable(const char*  tableName, std::vector<T>& items, const std::vector<std::string>& tableHeaders, const std::vector<std::string>& tableOrder, const std::string filename, const std::string key, std::function<void(const T&)> editCallback = nullptr, std::function<void(const T&)> deleteCallback = nullptr);

        void PlotBudget(const std::vector<Expense>& expenses, const std::vector<Income>& incomes);

        void SelectDateUI(std::vector<std::string>& allDates, std::string& selectedDate, std::string& status, bool& loadedDates, bool& loadedData);
    };
}


#endif // BUDGET_H