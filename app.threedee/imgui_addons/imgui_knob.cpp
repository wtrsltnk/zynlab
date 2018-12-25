#include "imgui_knob.h"
#include <cmath>
#include <stdio.h>

bool ImGui::Knob(char const *label, float *p_value, float v_min, float v_max, ImVec2 const &size)
{
    bool hideLabel = label[1] != '#' && label[0] != '#' && label[0] != '\0';

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec2 s(size.x - 4, size.y - 4);

    float radius_outer = std::fmin(s.x, s.y) / 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos = ImVec2(pos.x + 2, pos.y + 2);
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);

    float line_height = ImGui::GetTextLineHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.75f;
    float ANGLE_MAX = 3.141592f * 2.25f;

    if (s.x != 0.0f && s.y != 0.0f)
    {
        center.x = pos.x + (s.x / 2.0f);
        center.y = pos.y + (s.y / 2.0f);
        ImGui::InvisibleButton(label, ImVec2(s.x, s.y + (hideLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
    else
    {
        ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + (hideLabel ? line_height + style.ItemInnerSpacing.y : 0)));
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

    if (hideLabel)
    {
        auto textSize = CalcTextSize(label);
        draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);
    }

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - (line_height * 2) - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        if (hideLabel)
        {
            ImGui::Text("%.3f", static_cast<double>(*p_value));
        }
        else
        {
            ImGui::Text("%s %.3f", label, static_cast<double>(*p_value));
        }
        ImGui::EndTooltip();
    }

    return value_changed;
}

bool ImGui::KnobUchar(char const *label, unsigned char *p_value, unsigned char v_min, unsigned char v_max, ImVec2 const &size)
{
    float val = (p_value[0]) / 128.0f;

    if (ImGui::Knob(label, &val, v_min / 128.0f, v_max / 128.0f, size))
    {
        p_value[0] = static_cast<unsigned char>(val * 128);

        return true;
    }

    return false;
}

bool ImGui::TextCentered(ImVec2 const &size, char const *label)
{
    ImGuiStyle &style = ImGui::GetStyle();

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    auto result = ImGui::InvisibleButton(label, size);

    auto textSize = CalcTextSize(label);
    draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    return result;
}

void ImGui::ShowTooltipOnHover(char const *tooltip)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text(tooltip);
        ImGui::EndTooltip();
    }
}
