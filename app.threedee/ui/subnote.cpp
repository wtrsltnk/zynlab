#include "../app.threedee.h"

#include <zyn.synth/SUBnoteParams.h>

void AppThreeDee::SUBNoteEditor(SUBnoteParameters *parameters)
{
    ImGui::Text("SUBsynth Global Parameters of the Instrument");

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("SUBNote"))
    {
        if (ImGui::BeginTabItem("Amplitude"))
        {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Bandwidth"))
        {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Overtones"))
        {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Filter"))
        {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Frequency"))
        {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }}
