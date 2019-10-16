#ifndef IMGUI_CHECK_BUTTON_H
#define IMGUI_CHECK_BUTTON_H

#include <imgui.h>

namespace ImGui {

IMGUI_API bool CheckButton(const char *label, bool *p_value,
                      ImVec2 const &size);

} // namespace ImGui

#endif // IMGUI_CHECK_BUTTON_H
