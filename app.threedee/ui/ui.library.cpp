#include "ui.library.h"

#include <imgui.h>
#include <zyn.mixer/Mixer.h>

char const *const LibraryID = "Library";

zyn::ui::Library::Library(AppState *state)
    : _state(state)
{}

zyn::ui::Library::~Library() = default;

bool zyn::ui::Library::Setup()
{
    return true;
}

void zyn::ui::Library::Render()
{
    if (!_state->_showLibrary)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
    if (ImGui::Begin(LibraryID, &_state->_showLibrary))
    {
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
