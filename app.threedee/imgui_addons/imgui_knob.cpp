#include "imgui_knob.h"
#include <cmath>

// Implementing a simple custom widget using the public API.
// You may also use the <imgui_internal.h> API to get raw access to more data/helpers, however the internal API isn't guaranteed to be forward compatible.
// FIXME: Need at least proper label centering + clipping (internal functions RenderTextClipped provides both but api is flaky/temporary)
bool ImGui::Knob(const char *label, float *p_value, float v_min, float v_max, ImVec2 const &size)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    float radius_outer = std::fmin(size.x, size.y) / 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);

    float line_height = ImGui::GetTextLineHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.75f;
    float ANGLE_MAX = 3.141592f * 2.25f;

    if (size.x != 0.0f && size.y != 0.0f)
    {
        center.x = pos.x + (size.x / 2.0f);
        center.y = pos.y + (size.y / 2.0f);
        ImGui::InvisibleButton(label, ImVec2(size.x, size.y + line_height + style.ItemInnerSpacing.y));
    }
    else
    {
        ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + line_height + style.ItemInnerSpacing.y));
    }
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.x != 0.0f)
    {
        float step = (v_max - v_min) / 200.0f;
        *p_value += io.MouseDelta.x * step;
        if (*p_value < v_min)
            *p_value = v_min;
        if (*p_value > v_max)
            *p_value = v_max;
        value_changed = true;
    }

    float radius_inner = radius_outer * 0.40f;

    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * (*p_value - v_min) / (v_max - v_min);
    float angle_cos = cosf(angle);
    float angle_sin = sinf(angle);

    draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImGuiCol_FrameBg), 16);
    draw_list->AddLine(
        ImVec2(center.x + angle_cos * radius_inner, center.y + angle_sin * radius_inner),
        ImVec2(center.x + angle_cos * (radius_outer - 2), center.y + angle_sin * (radius_outer - 2)),
        ImGui::GetColorU32(ImGuiCol_SliderGrabActive),
        2.0f);
    draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    draw_list->AddText(ImVec2(pos.x, pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - line_height - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        ImGui::Text("%s %.3f", label, static_cast<double>(*p_value));
        ImGui::EndTooltip();
    }

    return value_changed;
}

bool ImGui::KnobUchar(const char *label, unsigned char *p_value, unsigned char v_min, unsigned char v_max, ImVec2 const &size)
{
    float val = (p_value[0]) / 128.0f;

    if (ImGui::Knob(label, &val, v_min / 128.0f, v_max / 128.0f, size))
    {
        p_value[0] = static_cast<unsigned char>(val * 128);

        return true;
    }

    return false;
}
