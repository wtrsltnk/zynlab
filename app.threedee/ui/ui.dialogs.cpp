#include "ui.dialogs.h"

#include <system.io/system.io.directory.h>
#include <system.io/system.io.directoryinfo.h>
#include <system.io/system.io.fileinfo.h>
#include <system.io/system.io.path.h>

char const *zyn::ui::Dialogs::SAVEFILE_DIALOG_ID = "Save workspace to file";
char const *zyn::ui::Dialogs::OPENFILE_DIALOG_ID = "Open workspace file";

zyn::ui::Dialogs::Dialogs(AppState *state)
    : _state(state)
{}

bool zyn::ui::Dialogs::Setup()
{
    _saveFileDialog.active = false;
    _saveFileDialog.fileNameBuffer[0] = '\0';

    _openFileDialog.active = false;
    _openFileDialog.fileNameBuffer[0] = '\0';

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

    _saveFileDialog.currentFiles = newFolder.GetFiles();
    _saveFileDialog.currentFolders = newFolder.GetDirectories();
}

void zyn::ui::Dialogs::SaveFileDialog(char const *title)
{
    _saveFileDialog.active = true;
    _saveFileDialog.fileNameBuffer[0] = '\0';
    ImGui::OpenPopup(title);
}

zyn::ui::DialogResults zyn::ui::Dialogs::RenderSaveFileDialog()
{
    DialogResults result = DialogResults::NoResult;

    ImGui::SetNextWindowSize(ImVec2(650, 600));
    if (ImGui::BeginPopupModal(SAVEFILE_DIALOG_ID, nullptr, ImGuiWindowFlags_NoResize))
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

void zyn::ui::Dialogs::updateOpenFileDialog()
{
    auto newFolder = System::IO::DirectoryInfo(_openFileDialog.currentPath);
    if (!newFolder.Exists())
    {
        return;
    }

    _openFileDialog.currentFiles = newFolder.GetFiles();
    _openFileDialog.currentFolders = newFolder.GetDirectories();
}

void zyn::ui::Dialogs::OpenFileDialog(char const *title)
{
    _openFileDialog.active = true;
    _openFileDialog.fileNameBuffer[0] = '\0';
    ImGui::OpenPopup(title);
}

zyn::ui::DialogResults zyn::ui::Dialogs::RenderOpenFileDialog()
{
    DialogResults result = DialogResults::NoResult;

    ImGui::SetNextWindowSize(ImVec2(650, 600));
    if (ImGui::BeginPopupModal(OPENFILE_DIALOG_ID, nullptr, ImGuiWindowFlags_NoResize))
    {
        ImGui::BeginGroup(); // Lock X position
        ImGui::Text("Current folder: \t %s", _openFileDialog.currentPath.c_str());
        ImGui::EndGroup();

        ImGui::BeginChild("Open File Dialog", ImVec2(0, -80));

        auto width = ImGui::GetContentRegionAvailWidth();

        if (ImGui::ListBoxHeader("Path", ImVec2(width, -ImGui::GetTextLineHeightWithSpacing())))
        {
            if (ImGui::Selectable(".."))
            {
                auto newDir = System::IO::DirectoryInfo(System::IO::Path::Combine(_openFileDialog.currentPath, ".."));
                _openFileDialog.currentPath = newDir.FullName();
                updateOpenFileDialog();
            }
            for (auto folder : _openFileDialog.currentFolders)
            {
                bool selected = false;
                if (ImGui::Selectable(System::IO::DirectoryInfo(folder).Name().c_str(), &selected))
                {
                    auto newDir = System::IO::DirectoryInfo(System::IO::Path::Combine(_openFileDialog.currentPath, folder));
                    _openFileDialog.currentPath = newDir.FullName();
                    updateOpenFileDialog();
                }
            }
            for (auto file : _openFileDialog.currentFiles)
            {
                bool selected = false;
                if (ImGui::Selectable(System::IO::FileInfo(file).Name().c_str(), &selected))
                {
                    strncpy(_openFileDialog.fileNameBuffer, System::IO::FileInfo(file).Name().c_str(), 256);
                }
            }
            ImGui::ListBoxFooter();
        }

        ImGui::EndChild();

        ImGui::BeginGroup(); // Lock X position
        ImGui::InputText("name", _openFileDialog.fileNameBuffer, 256);
        ImGui::EndGroup();
        if (ImGui::Button("Open", ImVec2(120, 0)))
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

std::string zyn::ui::Dialogs::GetOpenFileName()
{
    auto file = System::IO::FileInfo(System::IO::Path::Combine(_openFileDialog.currentPath, _openFileDialog.fileNameBuffer));
    return file.FullName();
}
