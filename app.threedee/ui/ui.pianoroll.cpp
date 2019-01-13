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
    auto lastIndex = _state->_sequencer.LastPatternIndex(trackIndex);
    for (int patternIndex = 0; patternIndex <= lastIndex; patternIndex++)
    {
        auto count = _state->_sequencer.PatternStepCount(trackIndex, patternIndex);

        ImGui::SameLine();
        ImGui::PushID(patternIndex + trackIndex * 1000);
        bool isActive = trackIndex == _state->_activeChannel && patternIndex == _state->_activePattern;
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        }

        if (_state->_sequencer.DoesPatternExistAtIndex(trackIndex, patternIndex))
        {
            auto &pattern = _state->_sequencer.GetPattern(trackIndex, patternIndex);
            if (ImGui::Button(pattern._name.c_str(), ImVec2(((count - 1) / 16 + 1) * 120.0f, trackHeight)))
            {
                _state->_activeChannel = trackIndex;
                _state->_activePattern = patternIndex;
            }
            if (ImGui::IsMouseDoubleClicked(0))
            {
                _state->_showEditor = true;
            }
        }
        else if (_state->_mixer->GetChannel(trackIndex)->Penabled)
        {
            if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
            {
                _state->_sequencer.AddPattern(trackIndex, patternIndex, "");
            }
        }
        if (isActive)
        {
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(1);
        }

        ImGui::PopID();
    }
    if (_state->_mixer->GetChannel(trackIndex)->Penabled)
    {
        ImGui::SameLine();
        ImGui::PushID((100 + trackIndex) * 2010);
        if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
        {
            _state->_sequencer.AddPattern(trackIndex, lastIndex + 1, "");
        }
        ImGui::PopID();
    }

    ImGuiPianoRollPatternEditorWindow();
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
        auto mousey = ImGui::GetMousePos().y - ImGui::GetWindowPos().y + ImGui::GetScrollY();
        auto mousex = ImGui::GetMousePos().x - ImGui::GetWindowPos().x + ImGui::GetScrollX();

        if (i % NoteNameCount == 0)
        {
            ImGui::Separator();
        }
        ImGui::PushID(i);

        auto min = ImGui::GetCursorPos();
        if (ImGui::Button(NoteNames[i % NoteNameCount], ImVec2(noteLabelWidth, ImGui::GetTextLineHeightWithSpacing())))
        {
            _state->_mixer->HitNote(_state->_activeChannel, i, 200, 200);
        }

        for (auto &note : selectedPattern._notes)
        {
            if (note._note != i) continue;

            ImGui::SetCursorPos(ImVec2(noteLabelWidth + style.ItemSpacing.x + note._step * stepWidth, min.y));
            ImGui::Button("", ImVec2(note._length * stepWidth, ImGui::GetTextLineHeightWithSpacing()));
        }

        auto step = mousex - (noteLabelWidth + style.ItemSpacing.x);
        if (ImGui::IsWindowFocused() && mousey >= min.y &&
            mousey <= (min.y + ImGui::GetTextLineHeight()) && step > 0 &&
            !selectedPattern.IsStepCovered(static_cast<unsigned char>(i), static_cast<int>(step)))
        {
            min.x = std::floor((noteLabelWidth + style.ItemSpacing.x + step) / stepWidth) * stepWidth;
            ImGui::SetCursorPos(min);
            if (ImGui::Button("+", ImVec2(stepWidth, ImGui::GetTextLineHeightWithSpacing())))
            {
                selectedPattern._notes.insert(TrackPatternNote(static_cast<unsigned char>(i), static_cast<int>((step / stepWidth)), 0.4f));
            }
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::End();
}
