#include "ui.samplenote.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.mixer/Mixer.h>
#include <zyn.synth/SampleNoteParams.h>

char const *const SmplSynthEditorID = "SMPL editor";

zyn::ui::SampleNote::SampleNote(AppState *state)
    : _state(state), _library(state)
{}

zyn::ui::SampleNote::~SampleNote() = default;

bool zyn::ui::SampleNote::Setup() { return true; }

void zyn::ui::SampleNote::Render()
{
    if (!_state->_showSMPLNoteEditor)
    {
        return;
    }

    auto track = _state->_mixer->GetTrack(_state->_currentTrack);
    ImGui::Begin(SmplSynthEditorID, &_state->_showSMPLNoteEditor);
    if (!_state->_showSMPLNoteEditor || track == nullptr || _state->_currentTrackInstrument < 0 || _state->_currentTrackInstrument >= NUM_TRACK_INSTRUMENTS)
    {
        ImGui::End();
        return;
    }

    auto *parameters = track->Instruments[_state->_currentTrackInstrument].smplpars;

    if (track->Instruments[_state->_currentTrackInstrument].Psmplenabled == 0)
    {
        ImGui::Text("SMPL editor is disabled");
        if (ImGui::Button("Enable SMPL synth"))
        {
            track->Instruments[_state->_currentTrackInstrument].Psmplenabled = 1;
        }
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("SMPLnoteTab"))
    {
        if (ImGui::BeginTabItem("Global"))
        {
            if (_state->_currentTrack >= 0)
            {
                ImGui::Text("SMPLsynth Global Parameters of the Instrument");

                static std::string selectedTag = "";
                static unsigned char selectingSampleForKey = 0;
                bool b = false;

                ImGui::BeginChild("samples");
                ImGui::Columns(3);
                ImGui::SetColumnWidth(0, 70);
                for (unsigned char i = SAMPLE_NOTE_MAX; i >= SAMPLE_NOTE_MIN; i--)
                {
                    ImGui::PushID(i);
                    ImGui::Text("%4s%d", NoteNames[(107 - i) % NoteNameCount], (107 - i) / NoteNameCount - 1);

                    ImGui::NextColumn();
                    if (parameters->PwavData.find(i) != parameters->PwavData.end())
                    {
                        ImGui::Text("%s", parameters->PwavData[i]->name.c_str());
                    }
                    else
                    {
                        ImGui::Text("<unused>");
                    }
                    ImGui::NextColumn();

                    if (ImGui::Button("Change"))
                    {
                        b = true;
                        selectingSampleForKey = i;
                    }
                    if (parameters->PwavData.find(i) != parameters->PwavData.end())
                    {
                        ImGui::SameLine();
                        if (ImGui::Button("Clear"))
                        {
                            parameters->PwavData.erase(i);
                        }
                    }
                    ImGui::NextColumn();
                    ImGui::PopID();
                }
                ImGui::EndChild();

                if (b)
                {
                    _library.Setup();
                    ImGui::OpenPopup("Select sample");
                }
                ImGui::SetNextWindowSize(ImVec2(650, 600));
                if (ImGui::BeginPopupModal("Select sample", nullptr, ImGuiWindowFlags_NoResize))
                {
                    _library.RenderSelectSample();

                    if (ImGui::Button("Ok", ImVec2(120, 0)))
                    {
                        if (_library.GetSelectedSample() != nullptr)
                        {
                            parameters->PwavData[selectingSampleForKey] = WavData::Load(_library.GetSelectedSample()->GetPath());
                        }
                        ImGui::CloseCurrentPopup();
                    }

                    ImGui::SameLine();

                    if (ImGui::Button("Cancel", ImVec2(120, 0)))
                    {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
