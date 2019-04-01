#include "ui.library.h"

#include <imgui.h>
#include <iostream>
#include <zyn.common/ILibraryManager.h>
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
    if (!_state->_showLibrary || _state->_currentTrack < 0)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
    if (ImGui::Begin(LibraryID, &_state->_showLibrary))
    {
        if (ImGui::BeginTabBar("LibraryTabs"))
        {
            if (ImGui::BeginTabItem("Instruments"))
            {
                InstrumentLibrary();

                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Samples"))
            {
                SampleLibrary();

                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::End();
    ImGui::PopStyleVar();
}

void zyn::ui::Library::InstrumentLibrary()
{
    auto &style = ImGui::GetStyle();

    ImGui::BeginChild("InstrumentLibrary");

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 150 + style.ItemSpacing.x);
    ImGui::SetColumnWidth(1, 150 + style.ItemSpacing.x);

    ImGui::Text("Tags");

    if (ImGui::ListBoxHeader("##Tags", ImVec2(200, -ImGui::GetTextLineHeightWithSpacing())))
    {
        for (auto const &tag : _state->_library->GetInstrumentTags())
        {
            bool selected = (_state->_currentInstrumentTag == tag);
            if (ImGui::Selectable(tag.c_str(), &selected))
            {
                _state->_currentInstrumentTag = tag;
            }
        }
        ImGui::ListBoxFooter();
    }

    ImGui::NextColumn();

    ImGui::Text("Instruments");

    if (_state->_currentInstrumentTag != "")
    {
        if (ImGui::ListBoxHeader("##Instruments", ImVec2(200, -ImGui::GetTextLineHeightWithSpacing())))
        {
            for (auto instrument : _state->_library->GetInstruments())
            {
                if (instrument->GetTags().find(_state->_currentInstrumentTag) == instrument->GetTags().end())
                {
                    continue;
                }

                auto instrumentName = instrument->GetName();
                auto const &track = _state->_mixer->GetTrack(_state->_currentTrack);

                bool selected = (track->loadedInstrument.tag == _state->_currentInstrumentTag && track->loadedInstrument.instrumentName == instrumentName);
                if (ImGui::Selectable(instrumentName.c_str(), selected))
                {
                    track->Lock();
                    _state->_library->LoadAsInstrument(instrument, track);
                    track->Penabled = 1;
                    track->ApplyParameters();
                    track->Unlock();
                    track->loadedInstrument.tag = _state->_currentInstrumentTag;
                    track->loadedInstrument.instrumentName = instrumentName;
                }
            }
            ImGui::ListBoxFooter();
        }
    }

    ImGui::EndChild();
}

void zyn::ui::Library::SampleLibrary()
{
    auto &style = ImGui::GetStyle();

    ImGui::BeginChild("SampleLibrary");

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 150 + style.ItemSpacing.x);
    ImGui::SetColumnWidth(1, 150 + style.ItemSpacing.x);

    ImGui::Text("Tags");

    if (ImGui::ListBoxHeader("##Tags", ImVec2(200, -ImGui::GetTextLineHeightWithSpacing())))
    {
        for (auto const &tag : _state->_library->GetSampleTags())
        {
            bool selected = (_state->_currentSampleTag == tag);
            if (ImGui::Selectable(tag.c_str(), &selected))
            {
                _state->_currentSampleTag = tag;
            }
        }
        ImGui::ListBoxFooter();
    }

    ImGui::NextColumn();

    ImGui::Text("Samples");

    if (_state->_currentSampleTag != "")
    {
        if (ImGui::ListBoxHeader("##Samples", ImVec2(200, -ImGui::GetTextLineHeightWithSpacing())))
        {
            for (auto sample : _state->_library->GetSamples())
            {
                if (sample->GetTags().find(_state->_currentSampleTag) == sample->GetTags().end())
                {
                    continue;
                }

                if (ImGui::Selectable(sample->GetName().c_str()))
                {
                    _state->_mixer->PreviewSample(sample->GetPath());
                }
            }
            ImGui::ListBoxFooter();
        }
    }

    ImGui::EndChild();
}
