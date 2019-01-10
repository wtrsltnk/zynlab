#ifndef IMGUI_KNOB_H
#define IMGUI_KNOB_H

#include <functional>
#include <imgui.h>

namespace ImGui {

IMGUI_API void UvMeter(char const *label, ImVec2 const &size, int *value, int v_min, int v_max);

IMGUI_API bool Knob(char const *label, float *p_value,
                    float v_min, float v_max,
                    ImVec2 const &size, char const *tooltip = nullptr);

IMGUI_API bool KnobUchar(char const *label, unsigned char *p_value,
                         unsigned char v_min, unsigned char v_max,
                         ImVec2 const &size, char const *tooltip = nullptr);

IMGUI_API bool DropDown(char const *label,  unsigned char &value, char const *const names[], int nameCount, char const *tooltip = nullptr);

IMGUI_API bool ImageToggleButton(const char* str_id, bool* v, ImTextureID user_texture_id, const ImVec2& size);

IMGUI_API bool TextCentered(ImVec2 const &size, char const *label);

IMGUI_API void ShowTooltipOnHover(char const *tooltip);

} // namespace ImGui

#endif // IMGUI_KNOB_H
