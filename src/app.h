#ifndef APP_H
#define APP_H

#include <vector>
#include "data/budgetData.h"
#include "budget/budget.h"

class App {
public:
    App();
    ~App();

    void initialize();
    void render();
private:
    std::vector<Income> incomes; 
    std::vector<Expense> expense; 
};

#endif // APP_H