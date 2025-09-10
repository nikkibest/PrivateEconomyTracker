#ifndef APP_H
#define APP_H

#include <vector>
#include "model/income.h"
#include "model/expense.h"
#include "model/transaction.h"
#include "gui/gui.h"
#include "data/storage.h"

class App {
public:
    App();
    ~App();

    void initialize();
    void render();
    void addIncome(const Income& income);
private:
    std::vector<Income> incomes; 
    std::vector<Expense> expense; 
};

#endif // APP_H