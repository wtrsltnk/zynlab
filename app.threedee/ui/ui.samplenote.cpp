#include "ui.samplenote.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.mixer/Mixer.h>
#include <zyn.synth/SampleNoteParams.h>

char const *const SmplSynthEditorID = "SMPL editor";

zyn::ui::SampleNote::SampleNote(AppState *state)
    : _state(state)
{}

zyn::ui::SampleNote::~SampleNote() = default;

bool zyn::ui::SampleNote::Setup() { return true; }

void zyn::ui::SampleNote::Render()
{
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

                auto stereo = parameters->Pstereo == 1;
                if (ImGui::Checkbox("Stereo", &stereo))
                {
                    parameters->Pstereo = stereo ? 1 : 0;
                }
                ImGui::ShowTooltipOnHover("Stereo");

                if (ImGui::BeginTabBar("SMPLNote"))
                {
                    ImGui::EndTabBar();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
