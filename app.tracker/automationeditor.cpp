#include "automationeditor.h"

#include <imgui.h>

AutomationEditor::AutomationEditor()
    : _session(nullptr)
{}

void AutomationEditor::SetUp(
    ApplicationSession *session)
{
    _session = session;
}

void AutomationEditor::Render2d()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "Automation",
        nullptr,
        flags);
    {
    }
    ImGui::End();
}
