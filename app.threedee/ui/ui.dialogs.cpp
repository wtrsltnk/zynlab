#include "ui.dialogs.h"

#include "../../zyn.common/filesystemapi.h"
#include "../imgui_addons/imgui_common.h"

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
    return FilesystemApi::PathCombine(_saveFileDialog.currentPath, _saveFileDialog.fileNameBuffer);
}

void zyn::ui::Dialogs::updateSaveFileDialog()
{
    if (!FilesystemApi::DirectoryExists(_saveFileDialog.currentPath))
    {
        return;
    }

    _saveFileDialog.currentFiles = FilesystemApi::DirectoryGetFiles(_saveFileDialog.currentPath, "*");
    _saveFileDialog.currentFolders = FilesystemApi::DirectoryGetDirectories(_saveFileDialog.currentPath, "*");
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
                _saveFileDialog.currentPath = FilesystemApi::PathCombine(_saveFileDialog.currentPath, "..");
                updateSaveFileDialog();
            }
            for (auto folder : _saveFileDialog.currentFolders)
            {
                bool selected = false;
                if (ImGui::Selectable(FilesystemApi::PathGetFileName(folder).c_str(), &selected))
                {
                    _saveFileDialog.currentPath = FilesystemApi::PathCombine(_saveFileDialog.currentPath, folder);
                    updateSaveFileDialog();
                }
            }
            for (auto file : _saveFileDialog.currentFiles)
            {
                bool selected = false;
                if (ImGui::Selectable(FilesystemApi::PathGetFileName(file).c_str(), &selected))
                {
                    strncpy(_saveFileDialog.fileNameBuffer, FilesystemApi::PathGetFileName(file).c_str(), 256);
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
    if (!FilesystemApi::DirectoryExists(_openFileDialog.currentPath))
    {
        return;
    }

    _openFileDialog.currentFiles = FilesystemApi::DirectoryGetFiles(_openFileDialog.currentPath, "*");
    _openFileDialog.currentFolders = FilesystemApi::DirectoryGetDirectories(_openFileDialog.currentPath, "*");
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
                _openFileDialog.currentPath = FilesystemApi::PathCombine(_openFileDialog.currentPath, "..");
                updateOpenFileDialog();
            }
            for (auto folder : _openFileDialog.currentFolders)
            {
                bool selected = false;
                if (ImGui::Selectable(FilesystemApi::PathGetFileName(folder).c_str(), &selected))
                {
                    _openFileDialog.currentPath = FilesystemApi::PathCombine(_openFileDialog.currentPath, folder);
                    updateOpenFileDialog();
                }
            }
            for (auto file : _openFileDialog.currentFiles)
            {
                bool selected = false;
                if (ImGui::Selectable(FilesystemApi::PathGetFileName(file).c_str(), &selected))
                {
                    strncpy(_openFileDialog.fileNameBuffer, FilesystemApi::PathGetFileName(file).c_str(), 256);
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
    return FilesystemApi::PathCombine(_openFileDialog.currentPath, _openFileDialog.fileNameBuffer);
}
