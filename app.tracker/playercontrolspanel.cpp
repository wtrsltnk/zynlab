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
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "PlayerControls",
        nullptr,
        flags | ImGuiWindowFlags_NoTitleBar);
    {
        bool playing = _session->_playState != PlayStates::Stopped;
        if (!playing)
        {
            if (ImGui::Button(ICON_FAD_PLAY, ImVec2(0, 0)))
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
        }
        else
        {
            if (ImGui::Button(ICON_FAD_PAUSE, ImVec2(0, 0)))
            {
                _session->StopPlaying();
                ImGui::SetWindowFocus(PatternEditor::ID);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Pause Playing");
                ImGui::EndTooltip();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FAD_STOP, ImVec2(0, 0)))
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

        bool isRecording = _session->IsRecording();
        if (isRecording)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255, 0, 0, 155));
        }
        if (ImGui::Button(ICON_FAD_RECORD, ImVec2(0, 0)))
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
