#include "librarydialog.h"

#include <imgui.h>

LibraryDialog::LibraryDialog()
{}

void LibraryDialog::ShowDialog(bool open)
{
    if (open)
    {
        ImGui::OpenPopup("Select sample");
    }

    ImGui::SetNextWindowSize(ImVec2(650, 600));
    if (ImGui::BeginPopupModal("Select sample", nullptr, ImGuiWindowFlags_NoResize))
    {
        if (ImGui::Button("Ok", ImVec2(120, 0)))
        {
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
