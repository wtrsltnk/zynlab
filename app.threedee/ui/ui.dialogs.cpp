#include "ui.dialogs.h"

#include <system.io/system.io.directory.h>
#include <system.io/system.io.directoryinfo.h>
#include <system.io/system.io.fileinfo.h>
#include <system.io/system.io.path.h>

zyn::ui::Dialogs::Dialogs(AppState *state)
    : _state(state)
{}

bool zyn::ui::Dialogs::Setup()
{
    return true;
}

std::string zyn::ui::Dialogs::GetSaveFileName()
{
    auto file = System::IO::FileInfo(System::IO::Path::Combine(_saveFileDialog.currentPath, _saveFileDialog.fileNameBuffer));
    return file.FullName();
}

void zyn::ui::Dialogs::updateSaveFileDialog()
{
    auto newFolder = System::IO::DirectoryInfo(_saveFileDialog.currentPath);
    if (!newFolder.Exists())
    {
        return;
    }

    _saveFileDialog.currentFiles.clear();
    _saveFileDialog.currentFolders.clear();

    _saveFileDialog.currentFiles = newFolder.GetFiles();
    _saveFileDialog.currentFolders = newFolder.GetDirectories();
}

void zyn::ui::Dialogs::SaveFileDialog(std::string const &title)
{
    _saveFileDialog.title = title;
    _saveFileDialog.active = true;
    _saveFileDialog.fileNameBuffer[0] = '\0';
    ImGui::OpenPopup(title.c_str());
}

zyn::ui::DialogResults zyn::ui::Dialogs::RenderSaveFileDialog()
{
    DialogResults result = DialogResults::NoResult;

    ImGui::SetNextWindowSize(ImVec2(650, 600));
    if (ImGui::BeginPopupModal(_saveFileDialog.title.c_str(), nullptr, ImGuiWindowFlags_NoResize))
    {
        ImGui::BeginGroup(); // Lock X position
        ImGui::Text("Current folder: \t %s", _saveFileDialog.currentPath.c_str());
        ImGui::EndGroup();

        ImGui::BeginChild("Save File Dialog", ImVec2(0, -80));

        auto width = ImGui::GetContentRegionAvailWidth();

        if (ImGui::ListBoxHeader("Path", ImVec2(width, -ImGui::GetTextLineHeightWithSpacing())))
        {
            if (ImGui::Selectable(".."))
            {
                auto newDir = System::IO::DirectoryInfo(System::IO::Path::Combine(_saveFileDialog.currentPath, ".."));
                _saveFileDialog.currentPath = newDir.FullName();
                updateSaveFileDialog();
            }
            for (auto folder : _saveFileDialog.currentFolders)
            {
                bool selected = false;
                if (ImGui::Selectable(System::IO::DirectoryInfo(folder).Name().c_str(), &selected))
                {
                    auto newDir = System::IO::DirectoryInfo(System::IO::Path::Combine(_saveFileDialog.currentPath, folder));
                    _saveFileDialog.currentPath = newDir.FullName();
                    updateSaveFileDialog();
                }
            }
            for (auto file : _saveFileDialog.currentFiles)
            {
                bool selected = false;
                if (ImGui::Selectable(System::IO::FileInfo(file).Name().c_str(), &selected))
                {
                    strncpy(_saveFileDialog.fileNameBuffer, System::IO::FileInfo(file).Name().c_str(), 256);
                }
            }
            ImGui::ListBoxFooter();
        }

        ImGui::EndChild();

        ImGui::BeginGroup(); // Lock X position
        ImGui::InputText("name", _saveFileDialog.fileNameBuffer, 256);
        ImGui::EndGroup();
        if (ImGui::Button("Save", ImVec2(120, 0)))
        {
            result = DialogResults::Ok;
            ImGui::CloseCurrentPopup();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", ImVec2(120, 0)))
        {
            result = DialogResults::Cancel;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    return result;
}
