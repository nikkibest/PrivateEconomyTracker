#include "richDadPoorDad.h"
#include "imgui.h"
#include "implot.h"
#include <iostream>

namespace richDadPoorDad {
void DrawRichDadDiagram(std::vector<Income>& incomes, std::vector<Expense>& expenses)
{
    ImGui::Begin("Rich Dad Poor Dad Statement");
    
    // Get window position for drawing
    ImVec2 origin = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    // Block positions and sizes
    ImVec2 asset_pos = origin + ImVec2(50, 50);
    ImVec2 asset_size = ImVec2(150, 60);
    
    ImVec2 liability_pos = origin + ImVec2(250, 50);
    ImVec2 liability_size = ImVec2(150, 60);
    
    ImVec2 income_pos = origin + ImVec2(50, 150);
    ImVec2 income_size = ImVec2(150, 60);
    
    ImVec2 expense_pos = origin + ImVec2(250, 150);
    ImVec2 expense_size = ImVec2(150, 60);
    
    // Draw blocks
    draw_list->AddRectFilled(asset_pos, asset_pos + asset_size, IM_COL32(100, 200, 250, 255));
    draw_list->AddRectFilled(liability_pos, liability_pos + liability_size, IM_COL32(250, 100, 100, 255));
    draw_list->AddRectFilled(income_pos, income_pos + income_size, IM_COL32(100, 250, 100, 255));
    draw_list->AddRectFilled(expense_pos, expense_pos + expense_size, IM_COL32(250, 250, 100, 255));
    
    // Add labels
    draw_list->AddText(asset_pos + ImVec2(10, 10), IM_COL32_WHITE, "Assets");
    draw_list->AddText(liability_pos + ImVec2(10, 10), IM_COL32_WHITE, "Liabilities");
    draw_list->AddText(income_pos + ImVec2(10, 10), IM_COL32_WHITE, "Income");
    draw_list->AddText(expense_pos + ImVec2(10, 10), IM_COL32_WHITE, "Expenses");
    
    // Draw arrows (lines with arrowheads)
    ImVec2 arrow_start = income_pos + ImVec2(asset_size.x / 2, asset_size.y);
    ImVec2 arrow_end = expense_pos + ImVec2(asset_size.x / 2, 0);
    draw_list->AddLine(arrow_start, arrow_end, IM_COL32(0,0,0,255), 3.0f);
    // Optionally, draw a triangle for arrowhead
    ImVec2 dir = arrow_end - arrow_start;
    float len = sqrtf(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.0f) {
        ImVec2 norm = ImVec2(dir.x / len, dir.y / len);
        ImVec2 perp = ImVec2(-norm.y, norm.x);
        float arrow_size = 10.0f;
        ImVec2 tip = arrow_end;
        ImVec2 left = tip - norm * arrow_size + perp * arrow_size * 0.5f;
        ImVec2 right = tip - norm * arrow_size - perp * arrow_size * 0.5f;
        draw_list->AddTriangleFilled(tip, left, right, IM_COL32(0,0,0,255));
    }
    
    // You can add more arrows between blocks as needed
    
    ImGui::End();
}
} // namespace richDadPoorDad
