#include "ui.library.h"

#include <algorithm>
#include <cctype>
#include <imgui.h>
#include <iostream>
#include <string>
#include <system.io/system.io.directory.h>
#include <system.io/system.io.directoryinfo.h>
#include <system.io/system.io.fileinfo.h>
#include <system.io/system.io.path.h>
#include <zyn.mixer/Mixer.h>

char const *const LibraryID = "Library";

zyn::ui::Library::Library(AppState *state)
    : _state(state)
{}

zyn::ui::Library::~Library() = default;

bool zyn::ui::Library::Setup()
{
    _selectSample.selectedLibrary = nullptr;
    _selectSample.selectedSample = nullptr;
    _selectSample.filter[0] = '\0';
    _selectSample.filteredSamples = _state->_library->GetSamples();

    return true;
}

void zyn::ui::Library::Render()
{
    if (_state->_currentTrack < 0)
    {
        return;
    }

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));
    if (ImGui::BeginChild(LibraryID, ImVec2(LIBRARY_WIDTH, 0)))
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
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::SameLine();
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
    return _selectSample.selectedSample;
}

void zyn::ui::Library::filterSamples()
{
    _selectSample.filteredSamples.clear();
    for (auto sample : _state->_library->GetSamples())
    {
        if (!findStringIC(sample->GetName(), _selectSample.filter))
        {
            continue;
        }
        if (_selectSample.selectedLibrary != nullptr && !sample->GetLibrary()->IsParent(_selectSample.selectedLibrary))
        {
            continue;
        }

        _selectSample.filteredSamples.insert(sample);
    }
}

void zyn::ui::Library::RenderSelectSample()
{
    ImGui::BeginGroup(); // Lock X position
    ImGui::Text("Current: \t %s", _selectSample.selectedSample != nullptr ? _selectSample.selectedSample->GetName().c_str() : "");
    ImGui::EndGroup();

    ImGui::BeginChild("SampleLibrary", ImVec2(0, -40));

    ImGui::Columns(2);

    auto width = ImGui::GetContentRegionAvailWidth();
    ImGui::Text("Libraries");

    ImGui::BeginChild("##TopLevelLibraries", ImVec2(width, -ImGui::GetTextLineHeightWithSpacing()));
    for (auto topLevel : _state->_library->GetTopLevelLibraries())
    {
        auto newSelection = libraryTree(topLevel);
        if (newSelection != nullptr)
        {
            _selectSample.selectedLibrary = newSelection;
            filterSamples();
        }
    }
    ImGui::EndChild();

    ImGui::NextColumn();

    width = ImGui::GetContentRegionAvailWidth();
    ImGui::Text("Samples");

    if (ImGui::InputText("##Filter", _selectSample.filter, 64))
    {
        filterSamples();
    }

    if (ImGui::ListBoxHeader("##Samples", ImVec2(width, -ImGui::GetTextLineHeightWithSpacing())))
    {
        for (auto sample : _selectSample.filteredSamples)
        {
            bool selected = _selectSample.selectedSample != nullptr && sample->GetPath() == _selectSample.selectedSample->GetPath();
            if (ImGui::Selectable(sample->GetName().c_str(), &selected))
            {
                _state->_mixer->PreviewSample(sample->GetPath());
                _selectSample.selectedSample = sample;
            }
        }
        ImGui::ListBoxFooter();
    }

    ImGui::EndChild();
}

ILibrary *zyn::ui::Library::libraryTree(ILibrary *library)
{
    ILibrary *result = nullptr;

    if (library->GetChildren().empty())
    {
        if (ImGui::TreeNodeEx(library->GetName().c_str(), ImGuiTreeNodeFlags_Leaf))
        {
            if (ImGui::IsItemClicked())
            {
                result = library;
            }
            ImGui::TreePop();
        }
    }
    else
    {
        if (ImGui::TreeNode(library->GetName().c_str()))
        {
            if (ImGui::IsItemClicked())
            {
                result = library;
            }

            for (auto level : library->GetChildren())
            {
                auto tmp = libraryTree(level);
                if (tmp != nullptr && result == nullptr)
                {
                    result = tmp;
                }
            }
            ImGui::TreePop();
        }
    }

    return result;
}

void zyn::ui::Library::InstrumentLibrary()
{
    ImGui::BeginChild("InstrumentLibrary");

    auto maxSize = ImGui::GetItemRectSize();

    ImGui::Columns(2);

    ImGui::Text("Libraries");

    for (auto topLevel : _state->_library->GetTopLevelLibraries())
    {
        auto newSelection = libraryTree(topLevel);
        if (newSelection != nullptr)
        {
            _state->_currentLibrary = newSelection;
        }
    }

    ImGui::NextColumn();

    ImGui::Text("Instruments");

    if (ImGui::ListBoxHeader("##Instruments", ImVec2((maxSize.x - 5) / 2, -ImGui::GetTextLineHeightWithSpacing())))
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

    if (ImGui::ListBoxHeader("##Samples", ImVec2((maxSize.x - 5) / 2, -ImGui::GetTextLineHeightWithSpacing())))
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
