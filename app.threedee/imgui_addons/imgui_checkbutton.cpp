#include "imgui_checkbutton.h"
#include <cmath>

bool ImGui::CheckButton(const char *label, bool *p_value, ImVec2 const &size)
{
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    auto result = false;

    if (ImGui::InvisibleButton(label, size))
    {
        *p_value = !*p_value;
        result = true;
    }

    if (!ImGui::IsItemActive() && !ImGui::IsItemHovered() && !*p_value)
    {
        return result;
    }

    auto color = ImGui::GetColorU32(ImGuiCol_FrameBg);
    if (ImGui::IsItemActive() || *p_value)
    {
        color = ImGui::GetColorU32(ImGuiCol_FrameBgActive);
    }
    else if (ImGui::IsItemHovered())
    {
        color = ImGui::GetColorU32(ImGuiCol_FrameBgHovered);
    }

    draw_list->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), color, 0);

    return result;
}
