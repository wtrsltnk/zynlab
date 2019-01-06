#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/LFOParams.h>

void AppThreeDee::LFO(char const *label, LFOParams *params)
{
    ImGui::Text("%s", label);

    ImGui::Spacing();
    ImGui::Spacing();

    static char const *items[] = {
        "SINE",
        "TRI",
        "SQR",
        "R.up",
        "R.dn",
        "E1dn",
        "E2dn",
    };
    static char const *current_item = nullptr;

    auto type = static_cast<int>(params->PLFOtype);
    current_item = items[type];
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("##lfotype", current_item))
    {
        for (int n = 0; n < 7; n++)
        {
            bool is_selected = (current_item == items[n]);
            if (ImGui::Selectable(items[n], is_selected))
            {
                current_item = items[n];
                params->PLFOtype = static_cast<unsigned char>(n);
            }
        }

        ImGui::EndCombo();
    }
    ImGui::ShowTooltipOnHover("LFO Type");

    if (ImGui::Knob("Freq.", &(params->Pfreq), 0.0f, 1.0f, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("LFO frequency");

    ImGui::SameLine();

    if (ImGui::KnobUchar("Depth", &(params->Pintensity), 0, 127, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("LFO amount");

    ImGui::SameLine();

    if (ImGui::KnobUchar("Start", &(params->Pstartphase), 0, 127, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("LFO startphase (leftmost is random)");

    ImGui::SameLine();

    if (ImGui::KnobUchar("Delay", &(params->Pdelay), 0, 127, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("LFO delay");

    ImGui::SameLine();

    if (ImGui::KnobUchar("Str.", &(params->Pstretch), 0, 127, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("LFO stretch");

    ImGui::SameLine();

    if (ImGui::KnobUchar("A.R", &(params->Prandomness), 0, 127, ImVec2(30, 30)))
    {
    }
    ImGui::ShowTooltipOnHover("LFO Amplitude Randomness");

    ImGui::SameLine();

    if (ImGui::KnobUchar("F.R", &(params->Pfreqrand), 0, 127, ImVec2(30, 30)))
    {
    }
    ImGui::ShowTooltipOnHover("LFO Frequency Randomness");
}
