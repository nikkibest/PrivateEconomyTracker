#ifndef BUDGET_H
#define BUDGET_H

#include <vector>
#include "data/budgetData.h"
#include "utils/numbersFormat.h"
#include "imgui.h"
#include "implot.h"



/* Add Income and Expense categories */
/* The table should offer filtering options */

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
    selectDateParams() : dateIdx(-1), loadedDates(false), loadedData(true) {}
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

template<typename T>
struct HistoryEntry {
    std::vector<T> entry;
    std::vector<std::string> dates;
    std::vector<std::string> status;
    std::vector<double> times;
    std::vector<double> amount;
    int cnt = 0;

    // Default constructor
    HistoryEntry() = default;
    void clear() {
        dates.clear();
        status.clear();
        times.clear();
        amount.clear();
        cnt = 0;
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

        std::map<int, std::string> id_to_date_balance, id_to_date_income, id_to_date_expense;

        // Balance parameters for table
        selectDateParams balanceDate, incomeDate, expenseDate;
        double tableTotalBalance = 0.0;
        double tableMonthlyAvgIncome  = 0.0, tableYearlyIncome  = 0.0;
        double tableMonthlyAvgExpense = 0.0, tableYearlyExpense = 0.0;
        // Balance parameters for plot
        HistoryEntry<BankBalance> histBalance;
        HistoryEntry<Income> histIncome;
        HistoryEntry<Expense> histExpense;

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
        PlotParams p1, p2;
        HoverParams p1_hover, p2_hover;
        PlotVisuals plotVisuals;
    public:
        BudgetManager();
        ~BudgetManager() = default;
        void   setTableTotalBalance         (double value) { tableTotalBalance = value; }
        void   setTableMonthlyAvgIncome     (double value) { tableMonthlyAvgIncome = value; }
        void   setTableYearlyIncome         (double value) { tableYearlyIncome = value; }
        void   setTableMonthlyAvgExpense    (double value) { tableMonthlyAvgExpense = value; }
        void   setTableYearlyExpense        (double value) { tableYearlyExpense = value; }
        double getTableTotalBalance()       const { return tableTotalBalance; }
        double getTableMonthlyAvgIncome()   const { return tableMonthlyAvgIncome; }
        double getTableYearlyIncome()       const { return tableYearlyIncome; }
        double getTableMonthlyAvgExpense()  const { return tableMonthlyAvgExpense; }
        double getTableYearlyExpense()      const { return tableYearlyExpense; }
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
            double monthlyAvgIncomeTmp  = histIncome.amount.empty() ? 0.0 : histIncome.amount.back();
            double monthlyAvgExpenseTmp = histExpense.amount.empty() ? 0.0 : histExpense.amount.back();
            surpassTargetIdx = -1;
            for (int i = 0; i < months; ++i) {
                monthlyExpenses     [i]   = monthlyAvgExpenseTmp;
                monthlyIncomes      [i]   = monthlyAvgIncomeTmp;
                monthlyBalance      [i]   = monthlyAvgIncomeTmp-monthlyAvgExpenseTmp;
                cumulativeExpenses  [i]   = monthlyAvgExpenseTmp * (i);
                cumulativeIncomes   [i]   = monthlyAvgIncomeTmp  * (i);
                cumulativeDifference[i]   = cumulativeIncomes[i] - cumulativeExpenses[i];
                cumulativeBalance   [i]   = histBalance.amount.back() + cumulativeDifference[i];
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
            double min = *std::min_element(allMonthlyValues.begin(), allMonthlyValues.end());
            double max = *std::max_element(allMonthlyValues.begin(), allMonthlyValues.end());
            double minHistory = histIncome.amount.empty() ? min : *std::min_element(histIncome.amount.begin(), histIncome.amount.end());
            double maxHistory = histIncome.amount.empty() ? max : *std::max_element(histIncome.amount.begin(), histIncome.amount.end());
            p1.Ymin    = std::min(min, minHistory);
            p1.Ymax    = std::max(max, maxHistory);
            p1.Yrange  = p1.Ymax - p1.Ymin; 
            p1.Ymargin = p1.Yrange * 0.05;
            p1.Xmin    = histIncome.times.empty() ? timeMonths.front() : histIncome.times.front();
            p1.Xmax    = timeMonths.back();
            p1.Xrange  = p1.Xmax - p1.Xmin;

            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeExpenses.begin(), cumulativeExpenses.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeIncomes.begin(), cumulativeIncomes.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeDifference.begin(), cumulativeDifference.end());
            allCumulativeValues.insert(allCumulativeValues.end(), cumulativeBalance.begin(), cumulativeBalance.end());
            min = *std::min_element(allCumulativeValues.begin(), allCumulativeValues.end());
            max = *std::max_element(allCumulativeValues.begin(), allCumulativeValues.end());
            minHistory = histBalance.amount.empty() ? min : *std::min_element(histBalance.amount.begin(), histBalance.amount.end());
            maxHistory = histBalance.amount.empty() ? max : *std::max_element(histBalance.amount.begin(), histBalance.amount.end());
            p2.Ymin = std::min(min, minHistory);
            p2.Ymax = std::max(max, maxHistory);
            p2.Yrange  = p2.Ymax - p2.Ymin;
            p2.Ymargin = p2.Yrange * 0.05;
            p2.Xmin    = histBalance.times.empty() ? timeMonths.front() : histBalance.times.front();
            p2.Xmax    = timeMonths.back();
            p2.Xrange  = p2.Xmax - p2.Xmin;
        }

        void loadBalanceData();
        void loadIncomeData();
        void loadExpenseData();
        void ShowBankBalanceInput();
        void ShowIncomeInput();
        void ShowExpenseInput();
        void PlotBudget();
        void PlotPieCharts();
        template<typename T>
        void addScatterAndHover(HistoryEntry<T>& entry, PlotParams& p, HoverParams& p_hover);

        template<typename T>
        void ComputeDependentValues(std::vector<T>& items, const std::string filename, const std::string keyStart, const std::string& date, const std::string& status);

        void CreateComboWithDeleteAndAdd(std::vector<std::string>& item,  bool& isLoaded, int& selectedIndex, const std::string& label, const std::string& filename, const std::string& extension, char* newItem);
        
        template<typename T>
        void CreateTable(const char*  tableName, std::vector<T>& items, const std::vector<std::string>& tableHeaders, const std::vector<std::string>& tableOrder, const std::string filename, const std::string key, std::map<int, std::string>& id_to_date, std::function<void(const T&)> editCallback = nullptr, std::function<void(const T&)> deleteCallback = nullptr);

        template<typename T>
        void SelectDateUI(std::vector<T>& items, selectDateParams& dateParams, std::string itemName);
    };
}


#endif // BUDGET_H