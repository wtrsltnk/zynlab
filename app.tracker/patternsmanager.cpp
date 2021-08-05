#include "patternsmanager.h"

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"
#include <imgui.h>

PatternsManager::PatternsManager()
    : _session(nullptr)
{
    std::fill(_patternPropertiesNameBuffer, _patternPropertiesNameBuffer + 128, 0);
}

void PatternsManager::SetUp(ApplicationSession *session)
{
    _session = session;

    SelectPattern(0);
}

void PatternsManager::Render2d()
{
    if (_session == nullptr)
    {
        return;
    }

    if (_selectedPattern != _session->_song->currentPattern)
    {
        SelectPattern(_session->_song->currentPattern);
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "Patterns",
        nullptr,
        flags);
    {
        ImGui::BeginChild("PatternsContainer");
        {
            if (ImGui::Button(ICON_FK_PLUS, ImVec2(0, 0)))
            {
                _session->_song->AddPattern();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Add pattern");
                ImGui::EndTooltip();
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_MINUS, ImVec2(0, 0)))
            {
                _session->_song->RemovePattern(_session->_song->currentPattern);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Remove selected pattern");
                ImGui::EndTooltip();
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_ARROW_UP, ImVec2(0, 0)))
            {
                _session->_song->MovePattern(_session->_song->currentPattern, -1);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Move pattern up");
                ImGui::EndTooltip();
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_ARROW_DOWN, ImVec2(0, 0)))
            {
                _session->_song->MovePattern(_session->_song->currentPattern, 1);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Move pattern down");
                ImGui::EndTooltip();
            }

            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_FILE_O, ImVec2(0, 0)))
            {
                _session->_song->DuplicatePattern(_session->_song->currentPattern);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Duplicate");
                ImGui::EndTooltip();
            }

            ImGui::BeginChild("patterns", ImVec2(0, -100));
            {
                for (unsigned int i = 0; i < _session->_song->GetPatternCount(); i++)
                {
                    ImGui::PushID(i);

                    auto pattern = _session->_song->GetPattern(i);
                    char buf[256] = {0};
                    sprintf_s(buf, 256, "%02d : %s", int(i), pattern->Name().c_str());
                    ImGui::Selectable(buf, i == _session->_song->currentPattern);
                    if (ImGui::IsItemClicked())
                    {
                        SelectPattern(i);
                    }

                    ImGui::PopID();
                }
            }
            ImGui::EndChild();

            ImGui::BeginChild("selectedpattern");
            {
                ImGui::Text("Name");
                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                if (ImGui::InputText("##name", _patternPropertiesNameBuffer, 128, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    _session->_song
                        ->GetPattern(_session->_song->currentPattern)
                        ->Rename(_patternPropertiesNameBuffer);
                }

                ImGui::Text("Length");
                int len = _session->_song->GetPattern(_session->_song->currentPattern)->Length();
                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                if (ImGui::InputInt("##length", &len, 4))
                {
                    _session->_song->GetPattern(_session->_song->currentPattern)->Resize(len);
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void PatternsManager::SelectPattern(unsigned int i)
{
    _selectedPattern = _session->_song->currentPattern = i;

    auto pattern = _session->_song->GetPattern(_session->_song->currentPattern);
    auto name = pattern->Name();
    strcpy_s(_patternPropertiesNameBuffer, 128, name.c_str());

    _session->selectedTab = SelectableTabs::PatternEditor;
}
