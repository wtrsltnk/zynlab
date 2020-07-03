#include "librarydialog.h"

#include <algorithm>
#include <imgui.h>

LibraryDialog::LibraryDialog()
{}

void LibraryDialog::SetUp(
    ApplicationSession *session)
{
    _session = session;

    _selectSample.selectedLibrary = nullptr;
    _selectSample.selectedSample = nullptr;
    _selectSample.filter[0] = '\0';
    _selectSample.filteredSamples = _session->_library->GetSamples();
}

bool findStringIC(const std::string &strHaystack, const std::string &strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });
    return (it != strHaystack.end());
}

void LibraryDialog::filterSamples()
{
    _selectSample.filteredSamples.clear();
    for (auto sample : _session->_library->GetSamples())
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

void LibraryDialog::ShowDialog(bool open, std::function<void(ILibraryItem *)> const &func)
{
    if (open)
    {
        ImGui::OpenPopup("Select sample");
    }

    ImGui::SetNextWindowSize(ImVec2(650, 600));
    if (ImGui::BeginPopupModal("Select sample", nullptr, ImGuiWindowFlags_NoResize))
    {
        ImGui::SetNextItemWidth(600);
        if (ImGui::InputText("##Filter", _selectSample.filter, 64))
        {
            filterSamples();
        }

        if (ImGui::ListBoxHeader("##Samples", ImVec2(600, -40)))
        {
            for (auto sample : _selectSample.filteredSamples)
            {
                bool selected = _selectSample.selectedSample != nullptr && sample->GetPath() == _selectSample.selectedSample->GetPath();
                if (ImGui::Selectable(sample->GetName().c_str(), &selected))
                {
                    _session->_mixer->PreviewSample(sample->GetPath());
                    _selectSample.selectedSample = sample;
                }
            }
            ImGui::ListBoxFooter();
        }

        if (ImGui::Button("Ok", ImVec2(120, 0)))
        {
            func(_selectSample.selectedSample);
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
