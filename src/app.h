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
    budget::BudgetManager budgetManager;
};

#endif // APP_H