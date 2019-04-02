#include "ui.samplenote.h"

#include "../imgui_addons/imgui_knob.h"
#include <algorithm>
#include <cctype>
#include <string>
#include <zyn.mixer/Mixer.h>
#include <zyn.synth/SampleNoteParams.h>

char const *const SmplSynthEditorID = "SMPL editor";

zyn::ui::SampleNote::SampleNote(AppState *state)
    : _state(state)
{}

zyn::ui::SampleNote::~SampleNote() = default;

bool zyn::ui::SampleNote::Setup() { return true; }

bool findStringIC(const std::string &strHaystack, const std::string &strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
    return (it != strHaystack.end());
}

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

                static std::string selectedTag = "";
                static ILibraryItem *selectedSample = nullptr;
                static char filter[64] = {0};
                static std::set<ILibraryItem *> filteredSamples = _state->_library->GetSamples();
                static unsigned char selectingSampleForKey = 0;
                bool b = false;

                ImGui::BeginChild("samples");
                ImGui::Columns(3);
                ImGui::SetColumnWidth(0, 70);
                for (unsigned char i = 87; i >= 75; i--)
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
                        selectedSample = nullptr;
                        selectingSampleForKey = i;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear"))
                    {
                        parameters->PwavData.erase(i);
                    }
                    ImGui::NextColumn();
                    ImGui::PopID();
                }
                ImGui::EndChild();

                if (b) ImGui::OpenPopup("Select sample");
                ImGui::SetNextWindowSize(ImVec2(650, 600));
                if (ImGui::BeginPopupModal("Select sample", nullptr, ImGuiWindowFlags_NoResize))
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
                        bool selected = (selectedTag == "");
                        if (ImGui::Selectable("Show Everything", &selected))
                        {
                            selectedTag = "";
                            filter[0] = 0;
                            filteredSamples = _state->_library->GetSamples();
                        }
                        for (auto const &tag : _state->_library->GetSampleTags())
                        {
                            selected = (selectedTag == tag);
                            if (ImGui::Selectable(tag.c_str(), &selected))
                            {
                                selectedTag = tag;
                                filter[0] = 0;
                                filteredSamples = _state->_library->GetSamples();
                            }
                        }
                        ImGui::ListBoxFooter();
                    }

                    ImGui::NextColumn();

                    ImGui::Text("Samples");

                    if (ImGui::InputText("Filter", filter, 64))
                    {
                        filteredSamples.clear();
                        for (auto sample : _state->_library->GetSamples())
                        {
                            if (!findStringIC(sample->GetName(), filter))
                            {
                                continue;
                            }
                            filteredSamples.insert(sample);
                        }
                    }

                    if (ImGui::ListBoxHeader("##Samples", ImVec2(350, -ImGui::GetTextLineHeightWithSpacing())))
                    {
                        for (auto sample : filteredSamples)
                        {
                            if (selectedTag.size() > 0 && sample->GetTags().find(selectedTag) == sample->GetTags().end())
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

                    ImGui::EndChild();

                    if (ImGui::Button("Ok", ImVec2(120, 0)))
                    {
                        if (selectedSample != nullptr)
                        {
                            parameters->PwavData[selectingSampleForKey] = WavData::Load(selectedSample->GetPath());
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
