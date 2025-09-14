#ifndef RICHDADPOORDAD_H
#define RICHDADPOORDAD_H

#include <vector>
#include "data/budgetData.h"
#include "utils/numbersFormat.h"
#include "utils/imvec2_ops.h"

namespace richDadPoorDad {
    void DrawRichDadDiagram(std::vector<Income>& incomes, std::vector<Expense>& expenses);

}

#endif // RICHDADPOORDAD_H