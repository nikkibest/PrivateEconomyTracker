#ifndef APP_H
#define APP_H

#include <vector>
#include "data/budgetData.h"
#include "budget/budget.h"
#include "portfolio/richDadPoorDad.h"
class App {
public:
    App();
    ~App();

    void initialize();
    void render();
private:
    std::vector<BankBalance> bankBalance; 
    std::vector<Income> incomes;
    std::vector<Expense> expenses;
};

#endif // APP_H