#include "ui.library.h"

#include <algorithm>
#include <cctype>
#include <imgui.h>
#include <iostream>
#include <string>
#include <zyn.mixer/Mixer.h>

char const *const LibraryID = "Library";

zyn::ui::Library::Library(AppState *state)
    : _state(state)
{}

zyn::ui::Library::~Library() = default;

bool zyn::ui::Library::Setup()
{
    _selectedSample = nullptr;
    _filter[0] = '\0';
    _filteredSamples = _state->_library->GetSamples();

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

bool findStringIC(const std::string &strHaystack, const std::string &strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
    return (it != strHaystack.end());
}

ILibraryItem *zyn::ui::Library::GetSelectedSample()
{
    return _selectedSample;
}

void zyn::ui::Library::RenderSelectSample()
{

    ImGui::BeginGroup(); // Lock X position
    ImGui::Text("Current: \t %s", _selectedSample != nullptr ? _selectedSample->GetName().c_str() : "");
    ImGui::EndGroup();

    auto &style = ImGui::GetStyle();

    ImGui::BeginChild("SampleLibrary", ImVec2(0, -40));

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 250 + style.ItemSpacing.x);
    ImGui::SetColumnWidth(1, 350 + style.ItemSpacing.x);

    ImGui::Text("Libraries");

    for (auto topLevel : _state->_library->GetTopLevelLibraries())
    {
        libraryTree(topLevel);
    }

    ImGui::NextColumn();

    ImGui::Text("Samples");

    if (ImGui::InputText("Filter", _filter, 64))
    {
        _filteredSamples.clear();
        for (auto sample : _state->_library->GetSamples())
        {
            if (!findStringIC(sample->GetName(), _filter))
            {
                continue;
            }
            _filteredSamples.insert(sample);
        }
    }

    if (ImGui::ListBoxHeader("##Samples", ImVec2(350, -ImGui::GetTextLineHeightWithSpacing())))
    {
        for (auto sample : _filteredSamples)
        {
            if (_state->_currentLibrary != nullptr && !sample->GetLibrary()->IsParent(_state->_currentLibrary))
            {
                continue;
            }

            bool selected = _selectedSample != nullptr && sample->GetPath() == _selectedSample->GetPath();
            if (ImGui::Selectable(sample->GetName().c_str(), &selected))
            {
                _state->_mixer->PreviewSample(sample->GetPath());
                _selectedSample = sample;
            }
        }
        ImGui::ListBoxFooter();
    }

    ImGui::EndChild();
}

void zyn::ui::Library::libraryTree(ILibrary *library)
{
    if (library->GetChildren().empty())
    {
        ImGui::TreeNodeEx(library->GetName().c_str(), ImGuiTreeNodeFlags_Leaf);
        if (ImGui::IsItemClicked())
        {
            _state->_currentLibrary = library;
        }
        ImGui::TreePop();
    }
    else
    {
        if (ImGui::TreeNode(library->GetName().c_str()))
        {
            if (ImGui::IsItemClicked())
            {
                _state->_currentLibrary = library;
            }
            for (auto level : library->GetChildren())
            {
                libraryTree(level);
            }
            ImGui::TreePop();
        }
    }
}

void zyn::ui::Library::InstrumentLibrary()
{
    ImGui::BeginChild("InstrumentLibrary");

    auto maxSize = ImGui::GetItemRectSize();

    ImGui::Columns(2);

    ImGui::Text("Libraries");

    for (auto topLevel : _state->_library->GetTopLevelLibraries())
    {
        libraryTree(topLevel);
    }

    ImGui::NextColumn();

    ImGui::Text("Instruments");

    if (ImGui::ListBoxHeader("##Instruments", ImVec2(maxSize.x / 2, -ImGui::GetTextLineHeightWithSpacing())))
    {
        for (auto instrument : _state->_library->GetInstruments())
        {
            if (_state->_currentLibrary != nullptr && !instrument->GetLibrary()->IsParent(_state->_currentLibrary))
            {
                continue;
            }

            auto instrumentName = instrument->GetName();
            auto const &track = _state->_mixer->GetTrack(_state->_currentTrack);

            bool selected = (_state->_currentLibrary != nullptr && track->loadedInstrument.libraryPath == _state->_currentLibrary->GetPath() && track->loadedInstrument.instrumentName == instrumentName);
            if (ImGui::Selectable(instrumentName.c_str(), selected))
            {
                track->Lock();
                _state->_library->LoadAsInstrument(instrument, track);
                track->Penabled = 1;
                track->ApplyParameters();
                track->Unlock();
                if (_state->_currentLibrary != nullptr)
                {
                    track->loadedInstrument.libraryPath = _state->_currentLibrary->GetPath();
                }
                track->loadedInstrument.instrumentName = instrumentName;
            }
        }
        ImGui::ListBoxFooter();
    }

    ImGui::EndChild();
}

void zyn::ui::Library::SampleLibrary()
{
    ImGui::BeginChild("SampleLibrary");

    auto maxSize = ImGui::GetItemRectSize();

    ImGui::Columns(2);

    ImGui::Text("Libraries");

    for (auto topLevel : _state->_library->GetTopLevelLibraries())
    {
        libraryTree(topLevel);
    }

    ImGui::NextColumn();

    ImGui::Text("Samples");

    if (ImGui::ListBoxHeader("##Samples", ImVec2(maxSize.x / 2, -ImGui::GetTextLineHeightWithSpacing())))
    {
        for (auto sample : _state->_library->GetSamples())
        {
            if (_state->_currentLibrary != nullptr && !sample->GetLibrary()->IsParent(_state->_currentLibrary))
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

    ImGui::EndChild();
}
