#include "librarydialog.h"

#include <algorithm>
#include <boolinq.h>
#include <imgui.h>

LibraryDialog::LibraryDialog() = default;

void LibraryDialog::SetUp(
    IMixer *mixer,
    ILibraryManager *library)
{
    _mixer = mixer;
    _library = library;

    _selectItem.selectedLibrary = nullptr;
    _selectItem.selectedSample = nullptr;
    _selectItem.filter[0] = '\0';
}

bool findStringIC(
    const std::string &strHaystack,
    const std::string &strNeedle)
{
    auto it = std::search(
        strHaystack.begin(), strHaystack.end(),
        strNeedle.begin(), strNeedle.end(),
        [](char ch1, char ch2) { return std::toupper(ch1) == std::toupper(ch2); });

    return (it != strHaystack.end());
}

void LibraryDialog::filterSamples()
{
    _selectItem.filteredItems =
        boolinq::from(_baseItemList)
            .where([&](ILibraryItem *item) { return findStringIC(item->GetName(), _selectItem.filter); })
            .where([&](ILibraryItem *item) { return _selectItem.selectedLibrary == nullptr || item->GetLibrary()->IsParent(_selectItem.selectedLibrary); })
            .orderBy([&](ILibraryItem *item) { return item->GetName(); })
            .toStdSet();
}

void LibraryDialog::ShowSampleDialog(
    bool open,
    std::function<void(ILibraryItem *)> const &func)
{
    _baseItemList = _library->GetSamples();

    ShowDialog("Select sample", open, func);
}

void LibraryDialog::ShowInstrumentDialog(
    bool open,
    std::function<void(ILibraryItem *)> const &func)
{
    _baseItemList = _library->GetInstruments();

    ShowDialog("Select instrument", open, func);
}

void LibraryDialog::ShowDialog(
    const char *title,
    bool open,
    const std::function<void(ILibraryItem *)> &func)
{
    if (open)
    {
        ImGui::OpenPopup(title);
        _selectItem.selectedLibrary = nullptr;
        _selectItem.selectedSample = nullptr;
        _selectItem.filter[0] = '\0';
        filterSamples();
    }

    ImGui::SetNextWindowSize(ImVec2(650 + ImGui::GetStyle().ItemSpacing.x * 2, 600));
    if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_NoResize))
    {
        ImGui::SetNextItemWidth(600 + ImGui::GetStyle().ItemSpacing.x);
        if (ImGui::InputText("##Filter", _selectItem.filter, 64))
        {
            filterSamples();
        }

        if (ImGui::ListBoxHeader("##Libraries", ImVec2(300, -50)))
        {
            bool allSelected = _selectItem.selectedLibrary == nullptr;
            if (ImGui::Selectable("All", &allSelected))
            {
                _selectItem.selectedLibrary = nullptr;
                filterSamples();
            }

            auto banks = boolinq::from(_library->GetTopLevelLibraries())
                             .firstOrDefault([&](ILibrary *l) { return l->GetName() == "banks"; });

            if (banks != nullptr)
            {
                for (auto library : banks->GetChildren())
                {
                    bool selected = _selectItem.selectedLibrary != nullptr && library->GetPath() == _selectItem.selectedLibrary->GetPath();
                    if (ImGui::Selectable(library->GetName().c_str(), &selected))
                    {
                        _selectItem.selectedLibrary = library;
                        filterSamples();
                    }
                }
            }

            ImGui::ListBoxFooter();
        }

        ImGui::SameLine();

        if (ImGui::ListBoxHeader("##Items", ImVec2(300, -50)))
        {
            for (auto sample : _selectItem.filteredItems)
            {
                bool selected = _selectItem.selectedSample != nullptr && sample->GetPath() == _selectItem.selectedSample->GetPath();
                if (ImGui::Selectable(sample->GetName().c_str(), &selected))
                {
                    _mixer->PreviewSample(sample->GetPath());
                    _selectItem.selectedSample = sample;
                }
            }
            ImGui::ListBoxFooter();
        }

        if (ImGui::Button("Ok", ImVec2(120, 0)))
        {
            func(_selectItem.selectedSample);
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
