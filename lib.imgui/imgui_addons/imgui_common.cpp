#include "imgui_common.h"

ImVec2 operator+(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}

ImVec2 operator-(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x - b.x, a.y - b.y);
}

ImVec4 operator*(ImVec4 const &a, ImVec4 const &b)
{
    return ImVec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
}
