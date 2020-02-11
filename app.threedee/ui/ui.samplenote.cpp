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

std::string zyn::ui::SampleNote::NoteToString(unsigned char note)
{
    switch (note)
    {
        case 35: return "Acoustic Bass Drum";
        case 36: return "Bass Drum 1";
        case 37: return "Side Stick";
        case 38: return "Acoustic Snare";
        case 39: return "Hand Clap";
        case 40: return "Electric Snare";
        case 41: return "Low Floor Tom";
        case 42: return "Closed Hi-Hat";
        case 43: return "High Floor Tom";
        case 44: return "Pedal Hi-Hat";
        case 45: return "Low Tom";
        case 46: return "Open Hi-Hat";
        case 47: return "Low-Mid Tom";
        case 48: return "Hi-Mid Tom";
        case 49: return "Crash Cymbal 1";
        case 50: return "High Tom";
        case 51: return "Ride Cymbal 1";
        case 52: return "Chinese Cymbal";
        case 53: return "Ride Bell";
        case 54: return "Tambourine";
        case 55: return "Splash Cymbal";
        case 56: return "Cowbell";
        case 57: return "Crash Cymbal 2";
        case 58: return "Vibraslap";
        case 59: return "Ride Cymbal 2";
        case 60: return "Hi Bongo";
        case 61: return "Low Bongo";
        case 62: return "Mute Hi Conga";
        case 63: return "Open Hi Conga";
        case 64: return "Low Conga";
        case 65: return "High Timbale";
        case 66: return "Low Timbale";
        case 67: return "High Agogo";
        case 68: return "Low Agogo";
        case 69: return "Cabasa";
        case 70: return "Maracas";
        case 71: return "Short Whistle";
        case 72: return "Long Whistle";
        case 73: return "Short Guiro";
        case 74: return "Long Guiro";
        case 75: return "Claves";
        case 76: return "Hi Wood Block";
        case 77: return "Low Wood Block";
        case 78: return "Mute Cuica";
        case 79: return "Open Cuica";
        case 80: return "Mute Triangle";
        case 81: return "Open Triangle";
    }

    return "drum";
}
void zyn::ui::SampleNote::Render()
{
    auto track = _state->_mixer->GetTrack(_state->_currentTrack);
    if (ImGui::BeginChild(SmplSynthEditorID, ImVec2(0,0), true) && track != nullptr)
    {
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
                    ImGui::SetColumnWidth(0, 150);
                    for (unsigned char i = SAMPLE_NOTE_MAX; i >= SAMPLE_NOTE_MIN; i--)
                    {
                        ImGui::PushID(i);
                        ImGui::Text("%s", NoteToString(i).c_str());

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
    }
    ImGui::EndChild();
}
