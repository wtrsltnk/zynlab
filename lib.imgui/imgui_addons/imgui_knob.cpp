#include "imgui_knob.h"

#include <iostream>

#include "imgui_common.h"
#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>
#include <stdio.h>
#include <vector>

void ImGui::UvMeter(
    char const *label,
    ImVec2 const &size,
    int *value,
    int v_min,
    int v_max)
{
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton(label, size);

    float stepHeight = (v_max - v_min + 1) / size.y;
    auto y = pos.y + size.y;
    auto hue = 0.4f;
    auto sat = 0.6f;
    for (int i = v_min; i <= v_max; i += 5)
    {
        hue = 0.4f - (static_cast<float>(i) / static_cast<float>(v_max - v_min)) / 2.0f;
        sat = (*value < i ? 0.0f : 0.6f);
        draw_list->AddRectFilled(ImVec2(pos.x, y), ImVec2(pos.x + size.x, y - (stepHeight * 4)), static_cast<ImU32>(ImColor::HSV(hue, sat, 0.6f)));
        y = pos.y + size.y - (i * stepHeight);
    }
}

bool ImGui::Knob(
    char const *label,
    float *p_value,
    float v_min,
    float v_max,
    ImVec2 const &size,
    char const *tooltip)
{
    bool showLabel = label[0] != '#' && label[1] != '#' && label[0] != '\0';

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec2 s(size.x - 4, size.y - 4);

    float radius_outer = std::fmin(s.x, s.y) / 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos = ImVec2(pos.x + 2, pos.y + 2);
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);

    float line_height = ImGui::GetTextLineHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.70f;
    float ANGLE_MAX = 3.141592f * 2.30f;

    if (s.x != 0.0f && s.y != 0.0f)
    {
        center.x = pos.x + (s.x / 2.0f);
        center.y = pos.y + (s.y / 2.0f);
        ImGui::InvisibleButton(label, ImVec2(s.x, s.y + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
    else
    {
        ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.y != 0.0f)
    {
        float step = (v_max - v_min) / 200.0f;
        *p_value -= io.MouseDelta.y * step;
        if (*p_value < v_min)
            *p_value = v_min;
        if (*p_value > v_max)
            *p_value = v_max;
        value_changed = true;
    }

    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * (*p_value - v_min) / (v_max - v_min);
    float angle_cos = cosf(angle);
    float angle_sin = sinf(angle);

    draw_list->AddCircleFilled(center, radius_outer * 0.7f, ImGui::GetColorU32(ImGuiCol_Button), 16);
    draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, ANGLE_MAX, 16);
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBg), false, 3.0f);
    draw_list->AddLine(
        ImVec2(center.x + angle_cos * (radius_outer * 0.35f), center.y + angle_sin * (radius_outer * 0.35f)),
        ImVec2(center.x + angle_cos * (radius_outer * 0.7f), center.y + angle_sin * (radius_outer * 0.7f)),
        ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
    draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, angle + 0.02f, 16);
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_SliderGrabActive), false, 3.0f);

    if (showLabel)
    {
        auto textSize = CalcTextSize(label);
        draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);
    }

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - (line_height * 2) - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        if (tooltip != nullptr)
        {
            ImGui::Text("%s\nValue : %.3f", tooltip, static_cast<double>(*p_value));
        }
        else if (showLabel)
        {
            ImGui::Text("%s %.3f", label, static_cast<double>(*p_value));
        }
        else
        {
            ImGui::Text("%.3f", static_cast<double>(*p_value));
        }
        ImGui::EndTooltip();
    }

    return value_changed;
}

bool ImGui::KnobUchar(
    char const *label,
    unsigned char *p_value,
    unsigned char v_min,
    unsigned char v_max,
    ImVec2 const &size,
    char const *tooltip)
{
    bool showLabel = label[0] != '#' && label[1] != '#' && label[0] != '\0';

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec2 s(size.x - 4, size.y - 4);

    float radius_outer = std::fmin(s.x, s.y) / 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos = ImVec2(pos.x, pos.y + 2);
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);

    float line_height = ImGui::GetFrameHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.70f;
    float ANGLE_MAX = 3.141592f * 2.30f;

    if (s.x != 0.0f && s.y != 0.0f)
    {
        center.x = pos.x + (s.x / 2.0f);
        center.y = pos.y + (s.y / 2.0f);
        ImGui::InvisibleButton(label, ImVec2(s.x, s.y + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
    else
    {
        ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    if (is_active)
    {
        std::cout << "knob active: " << io.MouseDelta.y << std::endl;
    }
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.y != 0.0f)
    {
        int step = (v_max - v_min) / 127;
        int newVal = static_cast<int>(*p_value - io.MouseDelta.y * step);
        if (newVal < v_min)
            newVal = v_min;
        if (newVal > v_max)
            newVal = v_max;
        *p_value = static_cast<unsigned char>(newVal);
        value_changed = true;
    }

    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * (*p_value - v_min) / (v_max - v_min);
    float angle_cos = cosf(angle);
    float angle_sin = sinf(angle);

    draw_list->AddCircleFilled(center, radius_outer * 0.7f, ImGui::GetColorU32(ImGuiCol_Button), 16);
    draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, ANGLE_MAX, 16);
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBg), false, 3.0f);
    draw_list->AddLine(
        ImVec2(center.x + angle_cos * (radius_outer * 0.35f), center.y + angle_sin * (radius_outer * 0.35f)),
        ImVec2(center.x + angle_cos * (radius_outer * 0.7f), center.y + angle_sin * (radius_outer * 0.7f)),
        ImGui::GetColorU32(ImGuiCol_Text), 2.0f);
    draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, angle + 0.02f, 16);
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_SliderGrabActive), false, 3.0f);

    if (showLabel)
    {
        auto textSize = CalcTextSize(label);
        draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);
    }

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - (line_height * 2) - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        if (tooltip != nullptr)
        {
            ImGui::Text("%s\nValue : %d", tooltip, static_cast<unsigned int>(*p_value));
        }
        else if (showLabel)
        {
            ImGui::Text("%s %d", label, static_cast<unsigned int>(*p_value));
        }
        else
        {
            ImGui::Text("%d", static_cast<unsigned int>(*p_value));
        }
        ImGui::EndTooltip();
    }
    else if (ImGui::IsItemHovered() && tooltip != nullptr)
    {
        ImGui::ShowTooltipOnHover(tooltip);
    }

    return value_changed;
}

bool ImGui::DropDown(
    char const *label,
    unsigned char& currentValue,
    std::vector<std::string> const names)
{
    return DropDown(label, currentValue, names, nullptr);
}

bool ImGui::DropDown(
    char const *label,
    unsigned char& currentValue,
    std::vector<std::string> const names,
    char const *tooltip)
{
    bool value_changed = false;

    auto current_effect_item = names[currentValue];
    if (ImGui::BeginCombo(label, current_effect_item.c_str(), ImGuiComboFlags_HeightLarge))
    {
        for (size_t n = 0; n < names.size(); n++)
        {
            bool is_selected = (current_effect_item == names[n]);
            if (ImGui::Selectable(names[n].c_str(), is_selected))
            {
                current_effect_item = names[n];
                currentValue = n;
                value_changed = true;
            }
        }

        ImGui::EndCombo();
    }
    ImGui::ShowTooltipOnHover(tooltip == nullptr ? label : tooltip);

    return value_changed;
}

bool ImGui::ImageToggleButton(
    const char *str_id,
    bool *v,
    ImTextureID user_texture_id,
    const ImVec2 &size)
{
    bool valueChange = false;

    const ImGuiStyle &style = ImGui::GetStyle();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float height = size.y + (style.FramePadding.y * 2);
    float width = size.x + (style.FramePadding.x * 2);

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (v != nullptr && ImGui::IsItemClicked())
    {
        *v = !*v;
        valueChange = true;
    }

    ImU32 col_tint = ImGui::GetColorU32((v == nullptr || *v ? ImGui::GetColorU32(ImVec4(1, 1, 1, 1)) : ImGui::GetColorU32(ImGuiCol_TextDisabled)));
    ImU32 col_bg = ImGui::GetColorU32(ImGui::GetColorU32(ImGuiCol_Button));
    if (ImGui::IsItemHovered())
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    }
    if (ImGui::IsItemActive())
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_ButtonActive);
    }

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg);
    draw_list->AddImage(user_texture_id, p, ImVec2(p.x + width, p.y + height), ImVec2(0, 0), ImVec2(1, 1), GetColorU32(col_tint));

    return valueChange;
}

bool ImGui::ToggleButtonWithCheckbox(
    const char *str_id,
    bool *v,
    bool *checked,
    const ImVec2 &size)
{
    bool valueChange = false;

    PushID(str_id);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    const float square_sz = GetFrameHeight();

    if (Checkbox("##ButtonCheck", checked))
    {
        valueChange = true;
    }

    SameLine();

    ImGui::InvisibleButton("##InvisibleButton", ImVec2(size.x, square_sz));
    if (ImGui::IsItemClicked())
    {
        *v = !*v;
        valueChange = true;
    }

    ImU32 col_tint = ImGui::GetColorU32((*v ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_Border)));
    ImU32 col_bg = ImGui::GetColorU32(ImGui::GetColorU32(ImGuiCol_WindowBg));
    if (ImGui::IsItemHovered())
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    }
    if (ImGui::IsItemActive() || *v)
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_Button);
    }

    draw_list->AddRectFilled(pos, pos + ImVec2(size.x + square_sz, square_sz), GetColorU32(col_bg));

    auto textSize = CalcTextSize(str_id);
    draw_list->AddText(ImVec2(pos.x + (size.x + square_sz - textSize.x) / 2, pos.y), col_tint, str_id);

    PopID();

    return valueChange;
}

bool ImGui::ToggleButton(
    const char *str_id,
    bool *v,
    const ImVec2 &size)
{
    bool valueChange = false;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImGui::InvisibleButton(str_id, size);
    if (ImGui::IsItemClicked())
    {
        *v = !*v;
        valueChange = true;
    }

    ImU32 col_tint = ImGui::GetColorU32((*v ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_Border)));
    ImU32 col_bg = ImGui::GetColorU32(ImGui::GetColorU32(ImGuiCol_WindowBg));
    if (ImGui::IsItemHovered())
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    }
    if (ImGui::IsItemActive() || *v)
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_Button);
    }

    draw_list->AddRectFilled(pos, pos + size, GetColorU32(col_bg));

    auto textSize = CalcTextSize(str_id);
    draw_list->AddText(ImVec2(pos.x + (size.x - textSize.x) / 2, pos.y), col_tint, str_id);

    return valueChange;
}

bool ImGui::TextCentered(
    ImVec2 const &size,
    char const *label)
{
    ImGuiStyle &style = ImGui::GetStyle();

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    auto result = ImGui::InvisibleButton(label, size);

    auto textSize = CalcTextSize(label);
    draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    return result;
}

void ImGui::ShowTooltipOnHover(
    char const *tooltip)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", tooltip);
        ImGui::EndTooltip();
    }
}

bool ImGui::Fader(
    const char *label,
    const ImVec2 &size,
    int *v,
    const int v_min,
    const int v_max,
    const char *format,
    float power)
{
    ImGuiDataType data_type = ImGuiDataType_S32;
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, nullptr, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(frame_bb, id))
        return false;

    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    if (format == nullptr)
        format = "%d";

    const bool hovered = ItemHoverable(frame_bb, id);
    if ((hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || g.NavInputSource == id)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        //        g.ActiveIdAllowNavDirFlags = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Draw frame
    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered
                                                                                                      : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(frame_bb, id, data_type, v, &v_min, &v_max, format, ImGuiSliderFlags_Vertical, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    ImRect gutter;
    gutter.Min = grab_bb.Min;
    gutter.Max = ImVec2(frame_bb.Max.x - 2.0f, frame_bb.Max.y - 2.0f);
    auto w = ((gutter.Max.x - gutter.Min.x) - 4.0f) / 2.0f;
    gutter.Min.x += w;
    gutter.Max.x -= w;
    window->DrawList->AddRectFilled(gutter.Min, gutter.Max, GetColorU32(ImGuiCol_ButtonActive), style.GrabRounding);

    // Render grab
    window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(ImGuiCol_Text), style.GrabRounding);

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    // For the vertical slider we allow centered text to overlap the frame padding
    char value_buf[64];
    sprintf(value_buf, format, int(*v * power));
    const char *value_buf_end = value_buf + strlen(value_buf);
    RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, nullptr, ImVec2(0.5f, 0.0f));
    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    return value_changed;
}
