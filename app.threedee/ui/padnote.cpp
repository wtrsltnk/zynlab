#include "../app.threedee.h"

#include <zyn.synth/PADnoteParams.h>

void AppThreeDee::PADNoteEditor(PADnoteParameters *parameters)
{
    ImGui::Text("PADsynth Global Parameters of the Instrument");

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("PADNote"))
    {
        if (ImGui::BeginTabItem("Harmonic structure"))
        {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Envelops LFOs"))
        {
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}
