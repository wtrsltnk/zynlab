#include "patternsmanager.h"

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"
#include <imgui.h>

PatternsManager::PatternsManager()
    : _session(nullptr)
{}

void PatternsManager::SetUp(ApplicationSession *session)
{
    _session = session;
}

void PatternsManager::Render2d()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "patterns",
        nullptr,
        flags);
    {
        ImGui::BeginChild("PatternsContainer");
        {
            if (ImGui::Button(ICON_FK_PLUS, ImVec2(0, 0)))
            {
                _session->_song->AddPattern();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_MINUS, ImVec2(0, 0)))
            {
                _session->_song->RemovePattern(_session->_song->currentPattern);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_ARROW_UP, ImVec2(0, 0)))
            {
                _session->_song->MovePattern(_session->_song->currentPattern, -1);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_ARROW_DOWN, ImVec2(0, 0)))
            {
                _session->_song->MovePattern(_session->_song->currentPattern, 1);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_FILE_O, ImVec2(0, 0)))
            {
                _session->_song->DuplicatePattern(_session->_song->currentPattern);
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
                        _session->_song->currentPattern = i;
                    }

                    ImGui::PopID();
                }
            }
            ImGui::EndChild();

            ImGui::BeginChild("selectedpattern");
            {
                ImGui::Text("Name");
                auto name = _session->_song->GetPattern(_session->_song->currentPattern)->Name();
                char text[128] = {0};
                strcpy_s(text, 128, name.c_str());
                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                ImGui::InputText("##name", text, 128, ImGuiInputTextFlags_EnterReturnsTrue);

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
