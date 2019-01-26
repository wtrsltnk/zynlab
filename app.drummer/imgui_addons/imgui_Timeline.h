#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

ImVec2 operator+(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}

ImVec2 operator-(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x - b.x, a.y - b.y);
}

namespace ImGui {
// Definitions (header file)
// Timeline (from: https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.h)=
/* Possible enhancements:
 * Add some kind of "snap to grid" epsilon
 * Add zooming with CTRL+MouseWheel, and a horizontal scrollbar
 * Add different types of TimelineEvent (e.g. multiple ranges in a single line, dot-like markers, etc.)
*/
IMGUI_API bool BeginTimelines(const char *str_id, float max_value = 0.f, int num_visible_rows = 5, int opt_exact_num_rows = 0); // last arg, when !=0, enables item culling
IMGUI_API void TimelineStart(const char *str_id, bool keep_range_constant = false);
IMGUI_API bool TimelineEvent(float *values, bool *selected = nullptr);
IMGUI_API bool TimelineEnd(float *new_values = nullptr);
IMGUI_API void EndTimelines(int num_vertical_grid_lines = 5, float current_time = 0.f, ImU32 timeline_running_color = IM_COL32(0, 128, 0, 200));
} // namespace ImGui

namespace ImGui {
// Timeline implementation (cpp file) from: https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.inl
static float s_max_timeline_value = 0.f;
static int s_timeline_num_rows = 0;
static int s_timeline_display_start = 0;
static int s_timeline_display_end = 0;
static int s_timeline_display_index = 0;
static const char *s_str_id = nullptr;
static bool s_keep_range_constant = false;
static ImVec2 s_cursor_pos;
static const float TIMELINE_RADIUS = 6;
static int s_event_counter = 0;
static bool s_is_event_hovered = false;
static float s_start_new_value = 0.0f;
static int s_max_value = 50;

static ImVec4 color = ImVec4(0.26f, 0.59f, 0.98f, 0.10f);

bool BeginTimelines(const char *str_id, float max_value, int num_visible_rows, int opt_exact_num_rows)
{
    // reset global variables
    s_max_timeline_value = 0.f;
    s_timeline_num_rows = s_timeline_display_start = s_timeline_display_end = 0;
    s_timeline_display_index = -1;
    s_max_value = static_cast<int>(max_value);

    const float row_height = ImGui::GetTextLineHeightWithSpacing();
    if (num_visible_rows <= 0)
    {
        num_visible_rows = static_cast<int>((GetWindowContentRegionMax().y) / row_height) - 3;
    }

    auto contentHeight = row_height * num_visible_rows;
    ImGui::SetNextWindowContentSize(ImVec2(max_value * 20, row_height * opt_exact_num_rows));
    const bool rv = ImGui::BeginChild(str_id, ImVec2(0, contentHeight), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleColor(ImGuiCol_Column, GImGui->Style.Colors[ImGuiCol_Border]);
    ImGui::Columns(2, str_id);
    ImGui::SetColumnWidth(0, 60);
    const float contentRegionWidth = ImGui::GetWindowContentRegionWidth();
    if (ImGui::GetColumnOffset(1) >= contentRegionWidth * 0.48f)
    {
        ImGui::SetColumnOffset(1, contentRegionWidth * 0.15f);
    }
    s_max_timeline_value = max_value >= 0 ? max_value : (contentRegionWidth * 0.85f);
    if (opt_exact_num_rows > 0)
    {
        // Item culling
        s_timeline_num_rows = opt_exact_num_rows;
        ImGui::CalcListClipping(s_timeline_num_rows, row_height, &s_timeline_display_start, &s_timeline_display_end);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (s_timeline_display_start * row_height));
    }

    return rv;
}

void TimelineStart(const char *str_id, bool keep_range_constant)
{
    ++s_timeline_display_index;

    if (s_timeline_num_rows > 0 &&
        (s_timeline_display_index < s_timeline_display_start || s_timeline_display_index >= s_timeline_display_end)) return; // item culling

    s_str_id = str_id;
    s_keep_range_constant = keep_range_constant;
    ImGuiWindow *win = GetCurrentWindow();
    const float columnOffset = ImGui::GetColumnOffset(1);
    s_cursor_pos = ImVec2(GetWindowContentRegionMin().x + win->Pos.x + columnOffset + GetScrollX(), win->DC.CursorPos.y);

    ImGui::Text("%s", str_id);
    ImGui::NextColumn();

    if (s_timeline_display_index % 2)
    {
        const ImU32 active_color = ColorConvertFloat4ToU32(color);
        auto num_vertical_grid_lines = s_max_value / 10;
        const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;
        const float horizontal_interval = columnWidth / num_vertical_grid_lines;
        ImVec2 end = s_cursor_pos;
        end.x += num_vertical_grid_lines * horizontal_interval;
        end.y += GetTextLineHeightWithSpacing();

        win->DrawList->AddRectFilled(s_cursor_pos, end, active_color);
    }

    s_event_counter = 0;
    s_is_event_hovered = false;
}

bool TimelineEnd(float *new_values)
{
    if (s_timeline_num_rows > 0 &&
        (s_timeline_display_index < s_timeline_display_start || s_timeline_display_index >= s_timeline_display_end)) return false; // item culling

    bool result = false;
    ImGuiWindow *win = GetCurrentWindow();
    const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
    const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;

    auto nextPos = s_cursor_pos + ImVec2(0, GetTextLineHeightWithSpacing());

    float end_new_value = (GetIO().MousePos.x - s_cursor_pos.x) / columnWidth * s_max_timeline_value;

    SetCursorScreenPos(s_cursor_pos);
    if (InvisibleButton(s_str_id, ImVec2(GetWindowContentRegionWidth(), GetTextLineHeight())) && new_values != nullptr)
    {
        new_values[0] = s_start_new_value;
        new_values[1] = end_new_value;
        result = true;
    }
    if (!s_is_event_hovered && IsItemHovered())
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    if (IsItemActive() && !IsMouseDragging(0))
    {
        s_start_new_value = (GetIO().MousePos.x - s_cursor_pos.x) / columnWidth * s_max_timeline_value;
    }
    if (IsItemHovered() && IsItemActive() && IsMouseDragging(0))
    {
        ImVec2 start = s_cursor_pos;
        start.x += columnWidth * s_start_new_value / s_max_timeline_value + (2 * TIMELINE_RADIUS);
        start.y += (ImGui::GetTextLineHeight() - TIMELINE_RADIUS) / 2.0f + TIMELINE_RADIUS * 0.5f;
        ImVec2 end = start + ImVec2(columnWidth * (end_new_value - s_start_new_value) / s_max_timeline_value - (2 * TIMELINE_RADIUS),
                                    TIMELINE_RADIUS);

        win->DrawList->AddRectFilled(start, end, active_color);

        ImGui::BeginTooltip();
        ImGui::Text("%.3f", end_new_value);
        ImGui::EndTooltip();
    }

    SetCursorScreenPos(nextPos);

    ImGui::NextColumn();

    return result;
}

bool TimelineEvent(float *values, bool *selected)
{
    if (s_timeline_num_rows > 0 &&
        (s_timeline_display_index < s_timeline_display_start || s_timeline_display_index >= s_timeline_display_end)) return false; // item culling

    ImGuiWindow *win = GetCurrentWindow();
    const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ColumnActive]);
    const ImU32 selected_color = ColorConvertFloat4ToU32(ImVec4(0.6f, 0.0f, 0.0f, 0.6f));
    const ImU32 selected_active_color = ColorConvertFloat4ToU32(ImVec4(1.0f, 0.0f, 0.0f, 0.6f));
    bool changed = false;
    bool hovered = false;
    bool allhovered = false;
    bool active = false;
    float newValues[2]{values[0], values[1]};

    const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;

    PushID(s_event_counter++);
    bool mustMoveBothEnds = false;
    const bool isMouseDraggingZero = IsMouseDragging(0);

    ImVec2 start = s_cursor_pos;
    start.x += columnWidth * values[0] / s_max_timeline_value + (2 * TIMELINE_RADIUS);
    start.y += (ImGui::GetTextLineHeight() - TIMELINE_RADIUS) / 2.0f;
    ImVec2 end = start + ImVec2(columnWidth * (values[1] - values[0]) / s_max_timeline_value - (2 * TIMELINE_RADIUS), TIMELINE_RADIUS * 2.0f);

    PushID(-1);
    SetCursorScreenPos(start);
    InvisibleButton(s_str_id, end - start);
    if ((IsItemActive() && isMouseDraggingZero) || mustMoveBothEnds)
    {
        const float deltaX = GetIO().MouseDelta.x / columnWidth * s_max_timeline_value;
        newValues[0] += deltaX;
        newValues[1] += deltaX;
        changed = hovered = allhovered = true;
    }
    else if (IsItemHovered())
    {
        hovered = allhovered = true;
    }
    if (IsItemActive()) changed = true;
    PopID();

    win->DrawList->AddRectFilled(start, end, IsItemActive() || IsItemHovered() ? selected != nullptr && *selected ? selected_active_color : active_color : selected != nullptr && *selected ? selected_color : inactive_color);

    for (int i = 0; i < 2; ++i)
    {
        ImVec2 pos = s_cursor_pos;
        pos.x += columnWidth * values[i] / s_max_timeline_value + TIMELINE_RADIUS;
        pos.y += (ImGui::GetTextLineHeight() - TIMELINE_RADIUS) / 2.0f + TIMELINE_RADIUS;

        SetCursorScreenPos(pos - ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS));
        PushID(i);
        InvisibleButton(s_str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
        active = IsItemActive();
        if (active || IsItemHovered())
        {
            ImGui::SetTooltip("%f", values[i]);
            if (!s_keep_range_constant)
            {
                // @meshula:The item hovered line needs to be compensated for vertical scrolling. Thx!
                ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y + win->Scroll.y);
                ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y + win->Scroll.y);
                win->DrawList->AddLine(a, b, line_color);
            }
            hovered = true;
        }
        if (active && isMouseDraggingZero)
        {
            if (!s_keep_range_constant)
                newValues[i] += GetIO().MouseDelta.x / columnWidth * s_max_timeline_value;
            else
                mustMoveBothEnds = true;
            changed = hovered = true;
        }
        if (active) changed = true;
        PopID();
        win->DrawList->AddRectFilled(pos - ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS),
                                     pos + ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS),
                                     IsItemActive() || IsItemHovered() || allhovered ? selected != nullptr && *selected ? selected_color * active_color : active_color : selected != nullptr && *selected ? selected_color : inactive_color);
    }

    values[0] = newValues[0];
    values[1] = newValues[1];

    if (values[0] > values[1])
    {
        float tmp = values[0];
        values[0] = values[1];
        values[1] = tmp;
    }
    if (values[1] > s_max_timeline_value)
    {
        values[0] -= values[1] - s_max_timeline_value;
        values[1] = s_max_timeline_value;
    }
    if (values[0] < 0)
    {
        values[1] -= values[0];
        values[0] = 0;
    }

    PopID();
    if (hovered)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
        s_is_event_hovered = true;
    }

    return changed;
}

void EndTimelines(int num_vertical_grid_lines, float current_time, ImU32 timeline_running_color)
{
    const float row_height = ImGui::GetTextLineHeightWithSpacing();
    if (s_timeline_num_rows > 0) ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((s_timeline_num_rows - s_timeline_display_end) * row_height));

    ImGui::NextColumn();

    ImGuiWindow *win = GetCurrentWindow();

    const float columnOffset = ImGui::GetColumnOffset(1);
    const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;
    const float horizontal_interval = columnWidth / num_vertical_grid_lines;

    ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
    ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
    ImU32 moving_line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ColumnActive]);
    const float rounding = GImGui->Style.ScrollbarRounding;
    const float startY = ImGui::GetWindowHeight() + win->Pos.y;

    // Draw black vertical lines (inside scrolling area)
    for (int i = 1; i <= num_vertical_grid_lines; ++i)
    {
        ImVec2 a = GetWindowContentRegionMin() + win->Pos;
        a.x += i * horizontal_interval + columnOffset + GetScrollX();
        win->DrawList->AddLine(a, ImVec2(a.x, startY), line_color);
    }

    // Draw moving vertical line
    if (current_time > 0.f && current_time < s_max_timeline_value)
    {
        ImVec2 a = GetWindowContentRegionMin() + win->Pos;
        a.x += columnWidth * (current_time / s_max_timeline_value) + columnOffset + GetScrollX();
        win->DrawList->AddLine(a, ImVec2(a.x, startY), moving_line_color);
    }

    ImGui::Columns(1);
    ImGui::PopStyleColor();

    EndChild();

    // Draw bottom axis ribbon (outside scrolling region)
    win = GetCurrentWindow();
    ImVec2 start(ImGui::GetCursorScreenPos().x + columnOffset, ImGui::GetCursorScreenPos().y);
    ImVec2 end(start.x + columnWidth, start.y + row_height);
    if (current_time <= 0)
        win->DrawList->AddRectFilled(start, end, color, rounding);
    else if (current_time > s_max_timeline_value)
        win->DrawList->AddRectFilled(start, end, timeline_running_color, rounding);
    else
    {
        ImVec2 median(start.x + columnWidth * (current_time / s_max_timeline_value), end.y);
        win->DrawList->AddRectFilled(start, median, timeline_running_color, rounding, 1 | 8);
        median.y = start.y;
        win->DrawList->AddRectFilled(median, end, color, rounding, 2 | 4);
        win->DrawList->AddLine(median, ImVec2(median.x, end.y), moving_line_color);
    }

    char tmp[256] = "";
    for (int i = 0; i < num_vertical_grid_lines; ++i)
    {
        ImVec2 a = start;
        a.x += i * horizontal_interval;
        a.y = start.y;

        ImFormatString(tmp, sizeof(tmp), "%.2f", i * s_max_timeline_value / num_vertical_grid_lines);
        win->DrawList->AddText(a, text_color, tmp);
    }
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + row_height);
}
// End Timeline
} // namespace ImGui
