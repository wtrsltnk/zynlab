#include "playercontrolspanel.h"

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"
#include "patterneditor.h"
#include <imgui.h>

PlayerControlsPanel::PlayerControlsPanel()
    : _session(nullptr)
{}

void PlayerControlsPanel::SetUp(
    ApplicationSession *session)
{
    _session = session;
}

void PlayerControlsPanel::Render2d()
{
    if (_session == nullptr)
    {
        return;
    }

    ImVec2 iconSize(20.0f, 20.0f);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "PlayerControls",
        nullptr,
        flags | ImGuiWindowFlags_NoTitleBar);
    {
        bool playing = _session->_playState != PlayStates::Stopped;

        if (ImGui::Selectable(ICON_FAD_PLAY, playing, ImGuiSelectableFlags_None, iconSize))
        {
            _session->StartPlaying();
            ImGui::SetWindowFocus(PatternEditor::ID);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Start Playing");
            ImGui::EndTooltip();
        }

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        if (ImGui::Selectable(ICON_FAD_PAUSE, !playing && _session->currentRow > 0, ImGuiSelectableFlags_None, iconSize))
        {
            _session->PausePlaying();
            ImGui::SetWindowFocus(PatternEditor::ID);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Pause Playing");
            ImGui::EndTooltip();
        }

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        if (ImGui::Selectable(ICON_FAD_STOP, !playing && _session->currentRow == 0, ImGuiSelectableFlags_None, iconSize))
        {
            _session->StopPlaying();
            if (_session->IsRecording())
            {
                _session->ToggleRecording();
            }
            ImGui::SetWindowFocus(PatternEditor::ID);
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("Stop Playing and reset to start of current pattern");
            ImGui::EndTooltip();
        }

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        bool isRecording = _session->IsRecording();
        if (isRecording)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255, 0, 0, 155));
        }
        if (ImGui::Selectable(ICON_FAD_RECORD, _session->IsRecording(), ImGuiSelectableFlags_None, iconSize))
        {
            ImGui::SetWindowFocus(PatternEditor::ID);
            _session->ToggleRecording();
        }
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            if (isRecording)
            {
                ImGui::Text("Stop Recording");
            }
            else
            {
                ImGui::Text("Start Recording");
            }
            ImGui::EndTooltip();
        }
        if (isRecording)
        {
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();
}
