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

                if (parameters->PwavData != nullptr)
                {
                    ImGui::Text("%s", parameters->PwavData->filename.c_str());
                }
                else
                {
                    ImGui::Text("No sample selected");
                }

                static std::string selectedTag;
                static ILibraryItem *selectedSample;
                if (ImGui::Button("Change sample"))
                {
                    ImGui::OpenPopup("Select sample");
                    selectedSample = nullptr;
                }

                ImGui::SetNextWindowSize(ImVec2(600, 600));
                if (ImGui::BeginPopupModal("Select sample"))
                {
                    ImGui::BeginGroup(); // Lock X position
                    ImGui::Text("Current: \t %s", selectedSample != nullptr ? selectedSample->GetName().c_str() : "");
                    ImGui::EndGroup();

                    auto &style = ImGui::GetStyle();

                    ImGui::BeginChild("SampleLibrary", ImVec2(0, -40));

                    ImGui::Columns(2);
                    ImGui::SetColumnWidth(0, 250 + style.ItemSpacing.x);
                    ImGui::SetColumnWidth(1, 350 + style.ItemSpacing.x);

                    ImGui::Text("Tags");

                    if (ImGui::ListBoxHeader("##Tags", ImVec2(250, -ImGui::GetTextLineHeightWithSpacing())))
                    {
                        for (auto const &tag : _state->_library->GetSampleTags())
                        {
                            bool selected = (selectedTag == tag);
                            if (ImGui::Selectable(tag.c_str(), &selected))
                            {
                                selectedTag = tag;
                            }
                        }
                        ImGui::ListBoxFooter();
                    }

                    ImGui::NextColumn();

                    ImGui::Text("Samples");

                    if (selectedTag != "")
                    {
                        if (ImGui::ListBoxHeader("##Samples", ImVec2(350, -ImGui::GetTextLineHeightWithSpacing())))
                        {
                            for (auto sample : _state->_library->GetSamples())
                            {
                                if (sample->GetTags().find(selectedTag) == sample->GetTags().end())
                                {
                                    continue;
                                }

                                bool selected = selectedSample != nullptr && sample->GetPath() == selectedSample->GetPath();
                                if (ImGui::Selectable(sample->GetName().c_str(), &selected))
                                {
                                    _state->_mixer->PreviewSample(sample->GetPath());
                                    selectedSample = sample;
                                }
                            }
                            ImGui::ListBoxFooter();
                        }
                    }

                    ImGui::EndChild();

                    if (ImGui::Button("Ok", ImVec2(120, 0)))
                    {
                        if (selectedSample != nullptr)
                        {
                            parameters->PwavData = WavData::Load(selectedSample->GetPath());
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
