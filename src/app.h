#ifndef APP_H
#define APP_H

#include <vector>
#include "model/income.h"
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
    std::vector<Income> incomes; // Add this
};

#endif // APP_H