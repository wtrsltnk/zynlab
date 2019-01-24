#include "ui.steppattern.h"

#include <imgui.h>

#include "../imgui_addons/imgui_checkbutton.h"
#include <zyn.mixer/Mixer.h>

char const *const StepPatternEditorID = "Pattern editor";

zyn::ui::StepPattern::StepPattern(AppState *state)
    : _state(state)
{}

zyn::ui::StepPattern::~StepPattern() = default;

bool zyn::ui::StepPattern::Setup()
{
    return true;
}

void zyn::ui::StepPattern::Render(int trackIndex, int trackHeight)
{
    auto lastIndex = _state->_sequencer.LastPatternIndex(trackIndex);
    for (int patternIndex = 0; patternIndex <= lastIndex; patternIndex++)
    {
        ImGui::SameLine();
        ImGui::PushID(patternIndex + trackIndex * 1000);
        bool isActive = (trackIndex == _state->_activeTrack && patternIndex == _state->_activePattern);
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        }

        if (_state->_sequencer.DoesPatternExistAtIndex(trackIndex, patternIndex))
        {
            auto &pattern = _state->_sequencer.GetPattern(trackIndex, patternIndex);
            if (ImGui::Button(pattern._name.c_str(), ImVec2(120.0f, trackHeight)))
            {
                _state->_activeTrack = trackIndex;
                _state->_activePattern = patternIndex;
            }
            if (ImGui::IsMouseDoubleClicked(0))
            {
                _state->_showEditor = true;
                ImGui::SetWindowFocus(StepPatternEditorID);
            }
        }
        else if (_state->_mixer->GetTrack(trackIndex)->Penabled)
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

    if (_state->_mixer->GetTrack(trackIndex)->Penabled)
    {
        ImGui::SameLine();
        ImGui::PushID((100 + trackIndex) * 2010);
        if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
        {
            _state->_sequencer.AddPattern(trackIndex, lastIndex + 1, "");
        }
        ImGui::PopID();
    }
}

void zyn::ui::StepPattern::RenderStepPatternEditorWindow()
{
    if (!_state->_showEditor)
    {
        return;
    }

    if (ImGui::Begin(StepPatternEditorID, &_state->_showEditor))
    {
        if (!_state->_sequencer.DoesPatternExistAtIndex(_state->_activeTrack, _state->_activePattern))
        {
            _state->_activePattern = -1;
            ImGui::End();
            return;
        }

        auto &style = ImGui::GetStyle();
        auto &selectedPattern = _state->_sequencer.GetPattern(_state->_activeTrack, _state->_activePattern);

        char tmp[256];
        strcpy(tmp, selectedPattern._name.c_str());
        if (ImGui::InputText("pattern name", tmp, 256))
        {
            selectedPattern._name = tmp;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 5));
        ImGui::BeginChild("Notes");
        auto width = ImGui::GetWindowWidth() - noteLabelWidth - (style.ItemSpacing.x * 2) - style.ScrollbarSize;
        auto itemWidth = (width / 16) - (style.ItemSpacing.x);

        // Start from B7 and go down to C1 (http://www.inspiredacoustics.com/en/MIDI_note_numbers_and_center_frequencies)
        for (int i = 107; i >= 24; i--)
        {
            ImGui::PushID(i);
            if (ImGui::Button(NoteNames[i % NoteNameCount], ImVec2(noteLabelWidth, ImGui::GetTextLineHeightWithSpacing())))
            {
                _state->_stepper->HitNote(_state->_activeTrack, i, 200, 200);
            }
            for (int j = 0; j < 16; j++)
            {
                ImGui::SameLine();
                ImGui::PushID(j);
                auto found = selectedPattern._notes.find(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j), 0.2f));
                bool s = (found != selectedPattern._notes.end());
                if (j % 4 == 0)
                {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_FrameBg]);
                }
                if (ImGui::CheckButton("##note", &s, ImVec2(itemWidth, ImGui::GetTextLineHeightWithSpacing())))
                {
                    if (!s)
                    {
                        selectedPattern._notes.erase(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j), 0.2f));
                    }
                    else
                    {
                        selectedPattern._notes.insert(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j), 0.2f));
                    }
                    _state->_stepper->HitNote(_state->_activeTrack, i, 200, 200);
                }
                ImGui::PopStyleColor();
                ImGui::PopID();
            }
            ImGui::PopID();

            if (i % NoteNameCount == 0)
            {
                ImGui::Separator();
            }
        }
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }
    ImGui::End();
}

void zyn::ui::StepPattern::EventHandling()
{
    ImGuiIO &io = ImGui::GetIO();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
    {
        _state->_sequencer.RemoveActivePattern(_state->_activeTrack, _state->_activePattern);
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
    {
        if (io.KeyShift && !io.KeyCtrl)
        {
            _state->_sequencer.MovePatternLeftForced(_state->_activeTrack, _state->_activePattern);
        }
        else if (!io.KeyShift && io.KeyCtrl)
        {
            _state->_sequencer.SwitchPatternLeft(_state->_activeTrack, _state->_activePattern);
        }
        else
        {
            _state->_sequencer.MovePatternLeftIfPossible(_state->_activeTrack, _state->_activePattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
    {
        if (io.KeyShift && !io.KeyCtrl)
        {
            _state->_sequencer.MovePatternRightForced(_state->_activeTrack, _state->_activePattern);
        }
        else if (!io.KeyShift && io.KeyCtrl)
        {
            _state->_sequencer.SwitchPatternRight(_state->_activeTrack, _state->_activePattern);
        }
        else
        {
            _state->_sequencer.MovePatternRightIfPossible(_state->_activeTrack, _state->_activePattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home)))
    {
        _state->_sequencer.SelectFirstPatternInTrack(_state->_activeTrack, _state->_activePattern);
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End)))
    {
        _state->_sequencer.SelectLastPatternInTrack(_state->_activeTrack, _state->_activePattern);
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
    {
        _state->_showEditor = true;
        ImGui::SetWindowFocus(StepPatternEditorID);
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
    {
        if (io.KeyShift)
        {
            _state->_sequencer.SelectPreviousPattern(_state->_activeTrack, _state->_activePattern);
        }
        else
        {
            _state->_sequencer.SelectNextPattern(_state->_activeTrack, _state->_activePattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)) && io.KeyCtrl)
    {
        if (_state->_sequencer.DoesPatternExistAtIndex(_state->_activeTrack, _state->_activePattern))
        {
            auto pattern = _state->_sequencer.GetPattern(_state->_activeTrack, _state->_activePattern);
            _state->_clipboardPatterns.push_back(pattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)) && io.KeyCtrl)
    {
        if (!_state->_clipboardPatterns.empty() && _state->_sequencer.DoesPatternExistAtIndex(_state->_activeTrack, _state->_activePattern))
        {
            _state->_sequencer.SetPattern(_state->_activeTrack, _state->_activePattern, _state->_clipboardPatterns.back());
        }
    }
}
