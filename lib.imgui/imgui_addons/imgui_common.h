#ifndef IMGUI_COMMON_H
#define IMGUI_COMMON_H

#include <imgui.h>
#include <imgui_internal.h>

ImVec2 operator+(ImVec2 const &a, ImVec2 const &b);
ImVec2 operator-(ImVec2 const &a, ImVec2 const &b);
ImVec4 operator*(ImVec4 const &a, ImVec4 const &b);

#endif // IMGUI_COMMON_H
