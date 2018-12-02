#ifndef IMGUI_KNOB_H
#define IMGUI_KNOB_H

#include <imgui.h>

namespace ImGui {

IMGUI_API bool Knob(const char *label, float *p_value,
                      float v_min, float v_max,
                      ImVec2 const &size);
IMGUI_API bool KnobUchar(const char *label, unsigned char *p_value,
                           unsigned char v_min, unsigned char v_max,
                           ImVec2 const &size);

} // namespace ImGui

#endif // IMGUI_KNOB_H
