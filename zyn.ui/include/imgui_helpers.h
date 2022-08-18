#ifndef IMGUI_HELPERS_H
#define IMGUI_HELPERS_H

#include <imgui.h>
#include <string>
#include <vector>

ImVec2 operator+(ImVec2 const &a, ImVec2 const &b);
ImVec2 operator-(ImVec2 const &a, ImVec2 const &b);

bool Knob(
    char const *label,
    float *p_value,
    float v_min,
    float v_max,
    ImVec2 const &size,
    char const *tooltip);

bool KnobUchar(
    char const *label,
    unsigned char *p_value,
    unsigned char v_min,
    unsigned char v_max,
    ImVec2 const &size,
    char const *tooltip);

void ShowTooltipOnHover(
    char const *tooltip);

bool DropDown(
    char const *label,
    unsigned char &value,
    const std::vector<std::string> &names,
    char const *tooltip);

#endif // IMGUI_HELPERS_H
