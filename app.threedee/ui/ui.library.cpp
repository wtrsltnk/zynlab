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
    if (!_state->_showLibrary || _state->_activeTrack < 0)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
    if (ImGui::Begin(LibraryID, &_state->_showLibrary))
    {
        auto &style = ImGui::GetStyle();

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 200 + style.ItemSpacing.x);
        ImGui::SetColumnWidth(1, 200 + style.ItemSpacing.x);

        ImGui::Text("Banks");

        auto count = _state->_mixer->GetBankManager()->GetBankCount();
        auto const &bankNames = _state->_mixer->GetBankManager()->GetBankNames();
        if (ImGui::ListBoxHeader("##Banks", ImVec2(200, -ImGui::GetTextLineHeightWithSpacing())))
        {
            for (int i = 0; i < count; i++)
            {
                bool selected = (_state->_currentBank == i);
                if (ImGui::Selectable(bankNames[static_cast<size_t>(i)], &selected))
                {
                    _state->_currentBank = i;
                    _state->_mixer->GetBankManager()->LoadBank(_state->_currentBank);
                }
            }
            ImGui::ListBoxFooter();
        }

        ImGui::NextColumn();

        ImGui::Text("Instruments");

        if (_state->_currentBank >= 0)
        {
            if (ImGui::ListBoxHeader("##Instruments", ImVec2(200, -ImGui::GetTextLineHeightWithSpacing())))
            {
                for (unsigned int i = 0; i < BANK_SIZE; i++)
                {
                    if (_state->_mixer->GetBankManager()->EmptySlot(i))
                    {
                        continue;
                    }

                    auto instrumentName = _state->_mixer->GetBankManager()->GetName(i);

                    if (ImGui::Selectable(instrumentName.c_str(), false))
                    {
                        auto const &instrument = _state->_mixer->GetTrack(_state->_activeTrack);
                        instrument->Lock();
                        _state->_mixer->GetBankManager()->LoadFromSlot(i, instrument);
                        instrument->Unlock();
                        instrument->ApplyParameters();
                    }
                }
                ImGui::ListBoxFooter();
            }
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}
