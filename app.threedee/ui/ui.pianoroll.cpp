#include "ui.pianoroll.h"

#include <imgui.h>
#include <zyn.mixer/Mixer.h>

zyn::ui::PianoRoll::PianoRoll(AppState *state)
    : _state(state)
{}

zyn::ui::PianoRoll::~PianoRoll() = default;

bool zyn::ui::PianoRoll::Setup()
{
    return true;
}

void zyn::ui::PianoRoll::Render(int trackIndex, int trackHeight)
{
}

void zyn::ui::PianoRoll::EventHandling()
{
}

void zyn::ui::PianoRoll::ImGuiPianoRollPatternEditorWindow()
{
    if (!_state->_showEditor)
    {
        return;
    }

    if (!_state->_sequencer.DoesPatternExistAtIndex(_state->_activeChannel, _state->_activePattern))
    {
        _state->_activePattern = -1;
        return;
    }

    auto &style = ImGui::GetStyle();
    auto &selectedPattern = _state->_sequencer.GetPattern(_state->_activeChannel, _state->_activePattern);

    ImGui::Begin("Piano roll editor");
    char tmp[256];
    strcpy(tmp, selectedPattern._name.c_str());
    if (ImGui::InputText("pattern name", tmp, 256))
    {
        selectedPattern._name = tmp;
    }

    ImGui::BeginChild("Notes");
    for (int i = 0; i < 88; i++)
    {
        ImGui::PushID(i);

        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::End();
}
