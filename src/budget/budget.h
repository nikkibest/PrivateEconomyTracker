#ifndef BUDGET_H
#define BUDGET_H

#include <vector>
#include "data/budgetData.h"
#include "utils/numbersFormat.h"
#include "imgui.h"
#include "implot.h"

/* Add the status save feature*/

/* The table should not show values with 0 amount.*/

/* The table could also show when the field was last modified based on the seleted date*/
/* The table should offer filtering options */
/* Update Incomes and Expenses correspondingly */

struct PlotVisuals {
    ImU32 color_confirmed = IM_COL32(0, 255, 0, 255); // Green
    ImU32 color_tentative = IM_COL32(200, 200, 200, 200); // Gray
    ImU32 color_hovered   = IM_COL32(255, 255, 0, 255); // Yellow
    float markerSize_confirmed = 5.0f;
    float markerSize_tentative = 3.0f;
    float markerSize_hovered   = 6.0f;
};
    
struct PlotParams {
    double Ymin, Ymax, Yrange, Ymargin;
    double Xmin, Xmax, Xrange, Xmargin;
    PlotParams() : Ymin(0), Ymax(0), Yrange(0), Ymargin(0), Xmin(0), Xmax(0), Xrange(0), Xmargin(0) {}
};

struct selectDateParams {
    std::vector<std::string> allNames;
    std::vector<std::string> allDates;
    std::string selectedDate;
    std::string status;
    bool loadedDates; 
    bool loadedData;
    char dateBuf[16]="";
    int  dateIdx;
    selectDateParams() : loadedDates(false), loadedData(true) {}
};

struct HoverParams {
    ImPlotPoint mousePos;
    int closestIdx;
    double XminDist, YminDist, Xdist, Ydist, sensitivity = 0.05;
    double Xthreshold, Ythreshold;
    HoverParams() = default;
    bool update(int& nLoop, std::vector<double>& Xcompare, std::vector<double>& Ycompare, double& Xrange, double& Yrange) {
        mousePos = ImPlot::GetPlotMousePos();
        closestIdx = -1;
        XminDist = 1e20;
        YminDist = 1e20;
        for (int i = 0; i < nLoop; ++i) {
            double Xdist = std::abs(mousePos.x - Xcompare[i]);
            double Ydist = std::abs(mousePos.y - Ycompare[i]);
            if ((Xdist < XminDist) && (Ydist < YminDist)) {
                XminDist = Xdist;
                YminDist = Ydist;
                closestIdx = i;
            }
        }
        Xthreshold = Xrange * sensitivity;
        Ythreshold = Yrange * sensitivity;
        return (closestIdx != -1 && XminDist < Xthreshold && YminDist < Ythreshold);
    }
};

namespace budget {
    // BudgetManager class to manage totalBalance
    class BudgetManager {
    private:
        // Main data vectors
        std::vector<BankBalance> bankBalance; 
        std::vector<Income> incomes;
        std::vector<Expense> expenses;

        // Balance parameters for table
        selectDateParams balanceDate;
        double totalBalanceTable = 0.0;
        // Balance parameters for plot
        std::vector<BankBalance> bankBalanceHist;
        std::vector<std::string> balanceDatesHist;
        std::vector<double> balanceHistTimes;
        std::vector<double> balanceHistAmount;
        std::vector<std::string> balanceStatusHist;
        int nHistBalances = 0;

        double monthlyAvgExpense = 0.0, monthlyAvgIncome = 0.0;

        // Plot parameters
        int months;
        int payRaiseRate     = 2;
        int inflationRate    = 2;
        double targetValue   = 1000000.0;
        int surpassTargetIdx = -1;
        double now = (double)time(nullptr);
        std::vector<double> monthlyExpenses;
        std::vector<double> monthlyIncomes;
        std::vector<double> monthlyBalance;
        std::vector<double> cumulativeExpenses;
        std::vector<double> cumulativeIncomes;
        std::vector<double> cumulativeDifference;
        std::vector<double> cumulativeBalance;
        std::vector<double> timeMonths;
        std::vector<double> allMonthlyValues;
        std::vector<double> allCumulativeValues;
        PlotParams p1;
        PlotParams p2;
        HoverParams p2_hover;
        PlotVisuals plotVisuals;
    public:
        BudgetManager();
        ~BudgetManager() = default;
        double getTotalBalanceTable() const { return totalBalanceTable; }
        void   setTotalBalanceTable(double value) { totalBalanceTable = value; }
        int  getMonths() const { return months; }
        void setMonths(int m) {
            months = m;
            monthlyExpenses     .resize(months  , 0.0);
            monthlyIncomes      .resize(months  , 0.0);
            monthlyBalance      .resize(months  , 0.0);
            cumulativeExpenses  .resize(months  , 0.0);
            cumulativeIncomes   .resize(months  , 0.0);
            cumulativeDifference.resize(months  , 0.0);
            cumulativeBalance   .resize(months  , 0.0);
            timeMonths          .resize(months  , 0.0);
            allMonthlyValues    .resize(months*3, 0.0);
            allCumulativeValues .resize(months*4, 0.0);
        }
        void updateMonthlyPlotValues() {
            double monthlyAvgExpenseTmp = monthlyAvgExpense;
            double monthlyAvgIncomeTmp  = monthlyAvgIncome;
            surpassTargetIdx = -1;
            for (int i = 0; i < months; ++i) {
                monthlyExpenses     [i]   = monthlyAvgExpenseTmp;
                monthlyIncomes      [i]   = monthlyAvgIncomeTmp;
                monthlyBalance      [i]   = monthlyAvgIncomeTmp-monthlyAvgExpenseTmp;
                cumulativeExpenses  [i]   = monthlyAvgExpenseTmp * (i);
                cumulativeIncomes   [i]   = monthlyAvgIncomeTmp  * (i);
                cumulativeDifference[i]   = cumulativeIncomes[i] - cumulativeExpenses[i];
                cumulativeBalance   [i]   = balanceHistAmount.back() + cumulativeDifference[i];
                timeMonths          [i]   = now + i * 30.0 * 24.0 * 3600.0; // Approximate month as 30 days
                if (i%12==0 && i!=0) {
                    // Apply inflation adjustment at the end of each year
                    monthlyAvgExpenseTmp *= (1.0 + inflationRate / 100.0);
                    monthlyAvgIncomeTmp  *= (1.0 + payRaiseRate / 100.0);
                }
                if (surpassTargetIdx == -1 && cumulativeBalance[i] >= targetValue) {
                    surpassTargetIdx = i;
                }
            }
            allMonthlyValues.insert(allMonthlyValues.end(), monthlyExpenses.begin(), monthlyExpenses.end());
            allMonthlyValues.insert(allMonthlyValues.end(), monthlyIncomes.begin() , monthlyIncomes.end());
            allMonthlyValues.insert(allMonthlyValues.end(), monthlyBalance.begin() , monthlyBalance.end());
            p1.Ymax    = *std::max_element(allMonthlyValues.begin(), allMonthlyValues.end());
            p1.Yrange  = p1.Ymax - 0;
            p1.Ymargin = p1.Yrange * 0.05;

            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeExpenses.begin(), cumulativeExpenses.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeIncomes.begin(), cumulativeIncomes.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeDifference.begin(), cumulativeDifference.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeBalance.begin(), cumulativeBalance.end());

            double minCumulative = *std::min_element(allCumulativeValues.begin(), allCumulativeValues.end());
            double maxCumulative = *std::max_element(allCumulativeValues.begin(), allCumulativeValues.end());
            double minHistory = balanceHistAmount.empty() ? minCumulative : *std::min_element(balanceHistAmount.begin(), balanceHistAmount.end());
            double maxHistory = balanceHistAmount.empty() ? maxCumulative : *std::max_element(balanceHistAmount.begin(), balanceHistAmount.end());
            p2.Ymin = std::min(minCumulative, minHistory);
            p2.Ymax = std::max(maxCumulative, maxHistory);
            p2.Yrange  = p2.Ymax - p2.Ymin;
            p2.Ymargin = p2.Yrange * 0.05;
            p2.Xmin    = balanceHistTimes.empty() ? timeMonths.front() : balanceHistTimes.front();
            p2.Xmax    = timeMonths.back();
            p2.Xrange  = p2.Xmax - p2.Xmin;
        }

        // auto getToday();
        void loadBalanceData();
        void ShowBankBalanceInput();
        void ShowIncomeInput();
        void ShowExpenseInput();
        void PlotBudget();

        template<typename T>
        void ComputeDependentValues(std::vector<T>& items, const std::string filename, const std::string keyStart, const std::string& date, const std::string& status);

        void CreateComboWithDeleteAndAdd(std::vector<std::string>& item,  bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem);
        
        template<typename T>
        void CreateTable(const char*  tableName, std::vector<T>& items, const std::vector<std::string>& tableHeaders, const std::vector<std::string>& tableOrder, const std::string filename, const std::string key, std::function<void(const T&)> editCallback = nullptr, std::function<void(const T&)> deleteCallback = nullptr);

        

        void SelectDateUI(selectDateParams& dateParams);
    };
}


#endif // BUDGET_H