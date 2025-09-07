#include "gui.h"
#include "imgui.h"
#include "implot.h"

namespace gui {

void ShowIncomeInput(std::vector<Income>& incomes) {
    static bool loaded = false;
    if (!loaded) {
        // Load incomes from file or initialize as needed
        load_incomes(incomes, "incomes.json");
        loaded = true;
    }

    static char source[64] = "";
    static float nrAnnualPayments = 0.0f;
    static float monthly = 0.0f;
    static float yearly = 0.0f;
    static float amount = 0.0f;

    ImGui::Begin("Incomes");
    ImGui::Text("Add all household incomes here.");
    ImGui::InputText("Source", source, IM_ARRAYSIZE(source));
    ImGui::InputFloat("Nr. Annual Payments", &nrAnnualPayments);
    ImGui::InputFloat("Amount", &amount);

    if (ImGui::Button("Add Income")) {
        if (strlen(source) > 0 && amount > 0.0f) {
            yearly = amount * nrAnnualPayments;
            monthly = yearly / 12.0f;
            incomes.push_back(Income{
                source, 
                static_cast<double>(nrAnnualPayments), 
                static_cast<double>(amount), 
                static_cast<double>(monthly), 
                static_cast<double>(yearly)
            });
            save_incomes(incomes, "incomes.json");
            source[0] = '\0';
            nrAnnualPayments = 0.0f;
            amount = 0.0f;
            monthly = 0.0f;
            yearly = 0.0f;
        }
    }

    ImGui::Separator();
    ImGui::Text("All Incomes:");
    double totalYearly = 0.0;
    if (ImGui::BeginTable("IncomeTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("Source");
        ImGui::TableSetupColumn("Nr. Annual Payments");
        ImGui::TableSetupColumn("Amount (DKK)");
        ImGui::TableSetupColumn("Monthly (DKK)");
        ImGui::TableSetupColumn("Yearly (DKK)");
        ImGui::TableSetupColumn("Delete");
        ImGui::TableHeadersRow();

        for (int i = 0; i < incomes.size(); ++i) {
            const auto& income = incomes[i];
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextUnformatted(income.source.c_str());
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(format_euro(income.nrAnnualPayments).c_str());
            ImGui::TableSetColumnIndex(2);
            ImGui::TextUnformatted(format_euro(income.amountNet).c_str());
            ImGui::TableSetColumnIndex(3);
            ImGui::TextUnformatted(format_euro(income.amountNet_month).c_str());
            ImGui::TableSetColumnIndex(4);
            ImGui::TextUnformatted(format_euro(income.amountNet_year).c_str());
            totalYearly += income.amountNet_year;
            
            // Add a delete button in a new column
            ImGui::TableSetColumnIndex(5);
            std::string btnLabel = "Delete##" + std::to_string(i);
            if (ImGui::Button(btnLabel.c_str())) {
                incomes.erase(incomes.begin() + i);
                save_incomes(incomes, "incomes.json");
                // To avoid skipping the next entry after erase
                --i;
                continue;
            }
        }
        ImGui::EndTable();
    }
    ImGui::Text("All Incomes:");
    if (ImGui::BeginTable("IncomeTableSummary", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable)) {
        double avgMonthly = totalYearly / 12.0f;
        ImGui::TableSetupColumn("Total Yearly (DKK)");
        ImGui::TableSetupColumn("Avg. Monthly (DKK)");
        ImGui::TableHeadersRow();

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextUnformatted(format_euro(totalYearly).c_str());
        ImGui::TableSetColumnIndex(1);
        ImGui::TextUnformatted(format_euro(avgMonthly).c_str());
        ImGui::EndTable();
    }

    // Calculate cumulative savings for 12 months
    constexpr int months = 12;
    static double savings[months] = {0};
    static double xticks[months] = {0};
    double monthlySum = 0.0;
    for (const auto& income : incomes) {
        monthlySum += income.amountNet_month;
    }
    for (int i = 0; i < months; ++i) {
        savings[i] = monthlySum * (i + 1);
        xticks[i] = i;
    }

    if (ImPlot::BeginPlot("Expected Savings Accumulation", ImVec2(-1,300))) {
        static const char* months_labels[months] = {
            "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"
        };
        ImPlot::SetupAxes("Month", "Savings (DKK)", ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
        // ImPlot::SetupAxisTicks(ImAxis_X1, 0, months - 1, months_labels, months);
        ImPlot::SetupAxisTicks(ImAxis_X1, xticks, months, months_labels);
        ImPlot::PlotLine("Savings", savings, months);
        ImPlot::EndPlot();
    }
    ImGui::End();
}

}