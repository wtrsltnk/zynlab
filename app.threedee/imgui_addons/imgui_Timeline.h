#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>

#include "imgui_common.h"

namespace ImGui {
// Definitions (header file)
// Timeline (from: https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.h)=
/* Possible enhancements:
 * Add some kind of "snap to grid" epsilon
 * Add zooming with CTRL+MouseWheel, and a horizontal scrollbar
 * Add different types of TimelineEvent (e.g. multiple ranges in a single line, dot-like markers, etc.)
*/
IMGUI_API bool BeginTimelines(const char *str_id, timestep *max_value, int row_height = 30, float horizontal_zoom = 50.f, int opt_exact_num_rows = 0, timestep snapping = 100); // last arg, when !=0, enables item culling
IMGUI_API void EmptyTimeline(const char *str_id);
IMGUI_API void TimelineStart(const char *str_id);
IMGUI_API void TimelineReadOnlyEvent(timestep *values, unsigned int image = 0, ImU32 const tintColor = IM_COL32(255, 255, 255, 200));
IMGUI_API bool TimelineEvent(timestep *values, unsigned int image = 0, ImU32 const tintColor = IM_COL32(255, 255, 255, 200), bool *selected = nullptr);
IMGUI_API bool TimelineEnd(timestep *new_values = nullptr);
IMGUI_API bool EndTimelines(timestep *current_time, ImU32 timeline_running_color = IM_COL32(0, 128, 0, 200));
} // namespace ImGui

namespace ImGui {
// Timeline implementation (cpp file) from: https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.inl
static timestep s_max_timeline_value = 0.f;
static int s_timeline_num_rows = 0;
static int s_timeline_display_start = 0;
static int s_timeline_display_end = 0;
static int s_timeline_display_index = 0;
static const char *s_str_id = nullptr;
static ImVec2 s_cursor_pos;
static const float TIMELINE_RADIUS = 6;
static int s_event_counter = 0;
static bool s_is_event_hovered = false;
static timestep s_start_new_value = 0;
static timestep s_start_move_event = 0;
static timestep *s_max_value = nullptr;
static float s_horizontal_zoom = 50.0f;
static int s_row_height = 30;
static timestep s_snapping = 100;
static float s_column_width = 0;

#define TEST(expr) \
    if (!(expr)) std::cout << "TEST FAILED @ " << __LINE__ << std::endl;

timestep snap(timestep value, timestep step = s_snapping)
{
    auto rounded = std::round(double(value / 1000.0) / double(step / 1000.0));
    return timestep(rounded * step);
}

void TestSnap()
{
    auto snap1 = snap(656, 1000);
    TEST(snap1 == 1000);

    auto snap2 = snap(570, 200);
    TEST(snap2 == 600);

    auto snap3 = snap(10, 200);
    TEST(snap3 == 0);
}

timestep snapFloor(timestep value, timestep step = s_snapping)
{
    auto floored = std::floor(double(value / 1000.0) / double(step / 1000.0));
    return timestep(floored * step);
}

void TestSnapFloor()
{
    auto snap1 = snapFloor(656, 1000);
    TEST(snap1 == 0);

    auto snap2 = snapFloor(570, 200);
    TEST(snap2 == 400);

    auto snap3 = snapFloor(10, 200);
    TEST(snap3 == 0);
}

bool CullTimeLine()
{
    return s_timeline_num_rows > 0 && (s_timeline_display_index < s_timeline_display_start || s_timeline_display_index >= s_timeline_display_end);
}

float timeToScreenX(timestep time)
{
    return float(s_column_width * float(time) / s_max_timeline_value);
}

timestep screenXToTime(float screenX)
{
    return timestep((screenX / s_column_width) * s_max_timeline_value);
}

timestep snappedValueFromMouse()
{
    return snap(screenXToTime(GetIO().MousePos.x - s_cursor_pos.x));
}

bool BeginTimelines(const char *str_id, timestep *max_value, int row_height, float horizontal_zoom, int opt_exact_num_rows, timestep snapping)
{
    TestSnap();
    TestSnapFloor();

    int const label_column_width = 60;
    ImGuiWindow *win = GetCurrentWindow();

    // reset global variables
    s_max_timeline_value = 0.f;
    s_timeline_num_rows = s_timeline_display_start = s_timeline_display_end = 0;
    s_timeline_display_index = -1;
    s_max_value = max_value;
    s_horizontal_zoom = horizontal_zoom;
    s_row_height = row_height;
    s_snapping = snapping;
    if (s_snapping <= 0) s_snapping = 100;

    auto timeline_length = timestep((*s_max_value) * horizontal_zoom) / 1000.0f;

    SetNextWindowContentSize(ImVec2(label_column_width + GetStyle().ItemInnerSpacing.x + timeline_length, s_row_height * opt_exact_num_rows));

    auto contentHeight = GetWindowContentRegionMax().y - (GetTextLineHeightWithSpacing() * 2);
    if (BeginChild(str_id, ImVec2(0, contentHeight), false, ImGuiWindowFlags_HorizontalScrollbar))
    {
        PushStyleColor(ImGuiCol_Column, GImGui->Style.Colors[ImGuiCol_Border]);
        Columns(2, str_id);
        SetColumnWidth(0, label_column_width);

        const float contentRegionWidth = GetWindowContentRegionWidth();
        s_max_timeline_value = s_max_value != nullptr ? (*s_max_value) : (timestep(contentRegionWidth) * 850);
        if (opt_exact_num_rows > 0)
        {
            // Item culling
            s_timeline_num_rows = opt_exact_num_rows;
            CalcListClipping(s_timeline_num_rows, row_height, &s_timeline_display_start, &s_timeline_display_end);
            SetCursorPosY(GetCursorPosY() + (s_timeline_display_start * s_row_height));
        }

        return true;
    }

    return false;
}

void EmptyTimeline(const char *str_id)
{
    ++s_timeline_display_index;

    if (CullTimeLine()) return; // item culling

    s_str_id = str_id;

    TextDisabled("%s", str_id);
    NextColumn();

    NextColumn();
}

void TimelineStart(const char *str_id)
{
    ++s_timeline_display_index;

    if (CullTimeLine()) return; // item culling

    ImGuiWindow *win = GetCurrentWindow();
    const float columnOffset = GetColumnOffset(1) + GetScrollX();
    const ImVec4 color1 = GetStyle().Colors[ImGuiCol_Border] * ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
    const ImVec4 color2 = GetStyle().Colors[ImGuiCol_Border] * ImVec4(1.0f, 1.0f, 1.0f, 0.2f);

    s_str_id = str_id;
    s_cursor_pos = ImVec2(GetWindowContentRegionMin().x + win->Pos.x + columnOffset, win->DC.CursorPos.y);
    s_event_counter = 0;
    s_is_event_hovered = false;

    Text("%s", str_id);
    NextColumn();
    s_column_width = (GetColumnWidth(1) - GImGui->Style.ScrollbarSize);

    auto start = s_cursor_pos + ImVec2(TIMELINE_RADIUS, 0);
    auto end = s_cursor_pos + ImVec2(timeToScreenX(*s_max_value), s_row_height);
    auto active_color = s_timeline_display_index % 2 ? ColorConvertFloat4ToU32(color1) : ColorConvertFloat4ToU32(color2);

    win->DrawList->AddRectFilled(start, end, active_color);

    const ImU32 line_color = ColorConvertFloat4ToU32(ImVec4(0.21f, 0.21f, 0.21f, 1.0f));
    const ImU32 accent_line_color = ColorConvertFloat4ToU32(ImVec4(0.25f, 0.25f, 0.25f, 1.0f));

    // Draw black vertical lines (inside scrolling area)
    int i = 0;
    while ((s_snapping * i) < s_max_timeline_value)
    {
        ImVec2 a = start + ImVec2(float(i) * timeToScreenX(s_snapping), 0);
        win->DrawList->AddLine(a, a + ImVec2(0, s_row_height), i % 4 ? line_color : accent_line_color);
        i++;
    }
}

bool TimelineEnd(timestep *newValues)
{
    if (CullTimeLine()) return false; // item culling

    bool result = false;
    ImGuiWindow *win = GetCurrentWindow();
    const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);

    auto nextPos = s_cursor_pos + ImVec2(0, s_row_height);

    timestep end_new_value = snappedValueFromMouse();

    SetCursorScreenPos(s_cursor_pos);
    if (InvisibleButton(s_str_id, ImVec2(GetWindowContentRegionWidth(), s_row_height)) && newValues != nullptr)
    {
        newValues[0] = s_start_new_value < end_new_value ? s_start_new_value : end_new_value;
        newValues[1] = end_new_value > s_start_new_value ? end_new_value : s_start_new_value;
        result = true;
    }

    if (newValues[0] + s_snapping >= newValues[1])
    {
        newValues[1] = newValues[0] + s_snapping;
    }

    if (!s_is_event_hovered && IsItemHovered())
    {
        SetMouseCursor(ImGuiMouseCursor_Hand);
    }

    if (IsItemActive() && !IsMouseDragging(0))
    {
        s_start_new_value = snapFloor(screenXToTime(GetIO().MousePos.x - s_cursor_pos.x));
    }

    if (IsItemHovered() && IsItemActive() && IsMouseDragging(0))
    {
        auto start = s_cursor_pos + ImVec2(timeToScreenX(snap(s_start_new_value)) + TIMELINE_RADIUS, 2.0f);
        auto end = start + ImVec2(timeToScreenX(snap(end_new_value - s_start_new_value)), s_row_height - 4.0f);

        win->DrawList->AddRectFilled(start, end, active_color);

        BeginTooltip();
        Text("%.1f-%.1f", double(s_start_new_value / 1000.0), double(end_new_value / 1000.0));
        EndTooltip();
    }

    SetCursorScreenPos(nextPos);

    NextColumn();

    return result;
}

void TimelineReadOnlyEvent(timestep *values, unsigned int image, ImU32 const tintColor)
{
    if (CullTimeLine())
    {
        return; // item culling
    }

    ImGuiWindow *win = GetCurrentWindow();

    PushID(s_event_counter++);

    ImVec2 start = s_cursor_pos;
    start.x += timeToScreenX(values[0]) + TIMELINE_RADIUS;
    start.y += 2.0f;
    ImVec2 end = start + ImVec2(timeToScreenX(values[1] - values[0]), s_row_height - 4.0f);

    ImU32 imageColor = tintColor;

    auto rgba = ColorConvertU32ToFloat4(tintColor);
    float hsv[3];
    ColorConvertRGBtoHSV(rgba.x, rgba.y, rgba.z, hsv[0], hsv[1], hsv[2]);
    imageColor = ImColor::HSV(hsv[0], hsv[1], hsv[2] * 0.5f);

    if (image > 0)
    {
        win->DrawList->AddImage(reinterpret_cast<ImTextureID>(image), start, end, ImVec2(0, 0), ImVec2(1, 1), imageColor);
    }
    else
    {
        win->DrawList->AddRectFilled(start, end, imageColor);
    }
    win->DrawList->AddRect(start, end, ColorConvertFloat4ToU32(ImVec4(0.3f, 0.3f, 0.3f, 0.8f)), 0, 0, 2.0f);

    PopID();
}

bool TimelineEvent(timestep *values, unsigned int image, ImU32 const tintColor, bool *selected)
{
    if (CullTimeLine())
    {
        return false; // item culling
    }

    ImGuiWindow *win = GetCurrentWindow();
    const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ColumnActive]);
    const bool isMouseDraggingZero = IsMouseDragging(0);
    bool changed = false;
    bool hovered = false;
    bool allhovered = false;
    timestep newValues[2]{values[0], values[1]};

    auto start = s_cursor_pos + ImVec2(timeToScreenX(values[0]) + TIMELINE_RADIUS, 2.0f);
    auto end = start + ImVec2(timeToScreenX(values[1] - values[0]), s_row_height - 4.0f);

    PushID(s_event_counter++);

    PushStyleColor(ImGuiCol_Button, tintColor);

    SetCursorScreenPos(start + ImVec2((*selected ? TIMELINE_RADIUS : 0), 0));

    InvisibleButton(s_str_id, (end - start) - ImVec2((*selected ? TIMELINE_RADIUS * 2 : 0), 0));
    if (IsItemActive() && isMouseDraggingZero)
    {
        auto move_event = snappedValueFromMouse();
        const float deltaX = move_event - s_start_move_event;
        if (std::fabs(deltaX) > 0.001f)
        {
            newValues[0] += deltaX;
            newValues[1] += deltaX;
            s_start_move_event = move_event;
        }
        changed = hovered = allhovered = true;
        BeginTooltip();
        Text("%.1f-%.1f", double(newValues[0] / 1000.0), double(newValues[1] / 1000.0));
        EndTooltip();
    }
    else if (IsItemHovered() && !isMouseDraggingZero)
    {
        s_start_move_event = snappedValueFromMouse();
    }
    if (IsItemHovered())
    {
        hovered = allhovered = true;
    }
    if (IsItemActive())
    {
        changed = true;
    }

    auto color = (IsItemActive() || IsItemHovered() || allhovered) ? active_color : inactive_color;

    win->DrawList->AddRectFilled(start, end, color);

    ImU32 imageColor = tintColor;
    if (selected == nullptr || !*selected)
    {
        auto rgba = ColorConvertU32ToFloat4(tintColor);
        float hsv[3];
        ColorConvertRGBtoHSV(rgba.x, rgba.y, rgba.z, hsv[0], hsv[1], hsv[2]);
        imageColor = ImColor::HSV(hsv[0], hsv[1], hsv[2] * 0.7f);
    }

    if (image > 0)
    {
        win->DrawList->AddImage(reinterpret_cast<ImTextureID>(image), start, end, ImVec2(0, 0), ImVec2(1, 1), imageColor);
    }
    else
    {
        win->DrawList->AddRectFilled(start, end, imageColor);
    }
    win->DrawList->AddRect(start, end, ColorConvertFloat4ToU32(ImVec4(0.0f, 0.0f, 0.0f, 0.8f)), 0, 0, 2.0f);

    PopStyleColor();

    if (selected != nullptr && *selected)
    {
        for (int i = 0; i < 2; ++i)
        {
            auto pos = s_cursor_pos + ImVec2(timeToScreenX(values[i]) + TIMELINE_RADIUS, 2.0f);

            SetCursorScreenPos(pos - ImVec2(TIMELINE_RADIUS, 0));

            PushID(i);

            InvisibleButton(s_str_id, ImVec2(TIMELINE_RADIUS * 2, s_row_height - 4.0f));
            if (IsItemActive() || IsItemHovered())
            {
                ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y + win->Scroll.y);
                ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y + win->Scroll.y);
                win->DrawList->AddLine(a - ImVec2(1, 0), b - ImVec2(1, 0), line_color, 1.0f);
                win->DrawList->AddLine(a + ImVec2(1, 0), b + ImVec2(1, 0), line_color, 1.0f);
                hovered = true;
                SetTooltip("%.1f", double(values[i] / 1000.0));
            }
            if (IsItemActive())
            {
                changed = true;
                if (IsItemActive() && isMouseDraggingZero)
                {
                    newValues[i] = snappedValueFromMouse();
                    hovered = true;
                }
            }

            PopID();

            if (hovered)
            {
                color = ColorConvertFloat4ToU32(ImVec4(1, 1, 1, 0.5f));
                start = pos - ImVec2(TIMELINE_RADIUS, 0);
                end = start + ImVec2(2 * TIMELINE_RADIUS, s_row_height - 4.0f);
                win->DrawList->AddRectFilled(start, end, color);
            }
        }
    }

    if (newValues[0] + s_snapping >= newValues[1])
    {
        newValues[1] = newValues[0] + s_snapping;
    }

    values[0] = newValues[0];
    values[1] = newValues[1];

    if (values[0] > values[1])
    {
        timestep tmp = values[0];
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
        SetMouseCursor(ImGuiMouseCursor_Arrow);
        s_is_event_hovered = true;
    }

    return changed;
}

bool EndTimelines(timestep *current_time, ImU32 timeline_running_color)
{
    bool changed = false;
    if (s_timeline_num_rows > 0)
    {
        SetCursorPosY(GetCursorPosY() + ((s_timeline_num_rows - s_timeline_display_end) * s_row_height));
    }

    NextColumn();

    ImGuiWindow *win = GetCurrentWindow();

    const float columnOffset = GetColumnOffset(1) + TIMELINE_RADIUS;
    const ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    const ImU32 moving_line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ColumnActive]);
    const float startY = win->Pos.y + s_row_height * s_timeline_num_rows;

    // Draw moving vertical line
    if (*current_time > 0 && *current_time < s_max_timeline_value)
    {
        ImVec2 a = GetWindowContentRegionMin() + win->Pos + ImVec2(columnOffset + GetScrollX(), 0);
        a.x += timeToScreenX(*current_time);
        win->DrawList->AddLine(a, ImVec2(a.x, startY), moving_line_color);
    }

    Columns(1);
    PopStyleColor();

    EndChild();

    ImVec2 buttonstart(GetCursorScreenPos().x, GetCursorScreenPos().y);
    SetCursorScreenPos(buttonstart);
    PushID(-101);
    PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 0));
    if (Button("+", ImVec2(GetItemsLineHeightWithSpacing(), GetItemsLineHeightWithSpacing())))
    {
        *s_max_value += 4000;
        *s_max_value -= (*s_max_value) % 4;
        changed = true;
    }
    SameLine();
    if (Button("-", ImVec2(GetItemsLineHeightWithSpacing(), GetItemsLineHeightWithSpacing())))
    {
        *s_max_value -= 4000;
        *s_max_value -= (*s_max_value) % 4;
        changed = true;
    }
    PopStyleVar();
    PopID();

    win = GetCurrentWindow();
    ImVec2 start(buttonstart.x + columnOffset, buttonstart.y);
    ImVec2 end(start.x + s_column_width, start.y + GetItemsLineHeightWithSpacing());

    PushID(-100);
    SetCursorScreenPos(start);
    if (InvisibleButton(s_str_id, end - start))
    {
        *current_time = screenXToTime(GetMousePos().x - start.x - GetScrollX());
    }
    PopID();

    if (*current_time <= 0)
    {
        win->DrawList->AddRectFilled(start, end, color);
    }
    else if (*current_time > s_max_timeline_value)
    {
        win->DrawList->AddRectFilled(start, end, timeline_running_color);
    }
    else
    {
        ImVec2 median(start.x + timeToScreenX(*current_time), end.y);
        win->DrawList->AddRectFilled(start, median, timeline_running_color);
        median.y = start.y;
        win->DrawList->AddRectFilled(median, end, color);
        win->DrawList->AddLine(median, ImVec2(median.x, end.y), moving_line_color);
    }

    SetCursorPosY(GetCursorPosY());

    if (changed && *s_max_value <= s_snapping)
    {
        *s_max_value = s_snapping;
    }

    return changed;
}

} // namespace ImGui
