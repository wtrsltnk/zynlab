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

    ImGui::Spacing();
    ImGui::Spacing();

    auto freq = params->Pfreq;
    if (ImGui::Knob("Freq.", &(freq), 0.0f, 1.0f, ImVec2(40, 40)))
    {
        params->Pfreq = freq;
    }
    ImGui::ShowTooltipOnHover("LFO frequency");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto depth = static_cast<float>(params->Pintensity);
    if (ImGui::Knob("Depth", &(depth), 0, 128, ImVec2(40, 40)))
    {
        params->Pintensity = static_cast<unsigned char>(depth);
    }
    ImGui::ShowTooltipOnHover("LFO amount");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto start = static_cast<float>(params->Pstartphase);
    if (ImGui::Knob("Start", &(start), 0, 128, ImVec2(40, 40)))
    {
        params->Pstartphase = static_cast<unsigned char>(start);
    }
    ImGui::ShowTooltipOnHover("LFO startphase (leftmost is random)");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto delay = static_cast<float>(params->Pdelay);
    if (ImGui::Knob("Delay", &(delay), 0, 128, ImVec2(40, 40)))
    {
        params->Pdelay = static_cast<unsigned char>(delay);
    }
    ImGui::ShowTooltipOnHover("LFO delay");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto stretch = static_cast<float>(params->Pstretch);
    if (ImGui::Knob("Str.", &(stretch), 0, 128, ImVec2(40, 40)))
    {
        params->Pstretch = static_cast<unsigned char>(stretch);
    }
    ImGui::ShowTooltipOnHover("LFO stretch");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto a_r = static_cast<float>(params->Prandomness);
    if (ImGui::Knob("A.R", &(a_r), 0, 128, ImVec2(30, 30)))
    {
        params->Prandomness = static_cast<unsigned char>(a_r);
    }
    ImGui::ShowTooltipOnHover("LFO Amplitude Randomness");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto f_r = static_cast<float>(params->Pfreqrand);
    if (ImGui::Knob("F.R", &(f_r), 0, 128, ImVec2(30, 30)))
    {
        params->Pfreqrand = static_cast<unsigned char>(f_r);
    }
    ImGui::ShowTooltipOnHover("LFO Frequency Randomness");

    ImGui::Spacing();
    ImGui::Spacing();
}
