#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <imgui.h>
#include <zyn.synth/EnvelopeParams.h>

void AppThreeDee::Envelope(char const *label, EnvelopeParams *envelope)
{
    ImGui::Text("%s", label);

    if (envelope->Envmode >= 3)
    {
        auto a_val = static_cast<float>(envelope->PA_val);
        if (ImGui::Knob("A.val", &(a_val), 0, 128, ImVec2(40, 40)))
        {
            envelope->PA_val = static_cast<unsigned char>(a_val);
        }
        ImGui::ShowTooltipOnHover("Starting value");

        ImGui::SameLine();
    }

    auto a_dt = static_cast<float>(envelope->PA_dt);
    if (ImGui::Knob("A.dt", &(a_dt), 0, 128, ImVec2(40, 40)))
    {
        envelope->PA_dt = static_cast<unsigned char>(a_dt);
    }
    ImGui::ShowTooltipOnHover("Attack time");

    ImGui::SameLine();

    if (envelope->Envmode == 4)
    {
        auto d_val = static_cast<float>(envelope->PD_val);
        if (ImGui::Knob("D.val", &(d_val), 0, 128, ImVec2(40, 40)))
        {
            envelope->PD_val = static_cast<unsigned char>(d_val);
        }
        ImGui::ShowTooltipOnHover("Decay value");

        ImGui::SameLine();
    }

    if (envelope->Envmode != 3 && envelope->Envmode != 5)
    {
        auto d_dt = static_cast<float>(envelope->PD_dt);
        if (ImGui::Knob("D.dt", &(d_dt), 0, 128, ImVec2(40, 40)))
        {
            envelope->PD_dt = static_cast<unsigned char>(d_dt);
        }
        ImGui::ShowTooltipOnHover("Decay time");

        ImGui::SameLine();
    }

    if (envelope->Envmode < 3)
    {
        auto s_val = static_cast<float>(envelope->PS_val);
        if (ImGui::Knob("S.val", &(s_val), 0, 128, ImVec2(40, 40)))
        {
            envelope->PS_val = static_cast<unsigned char>(s_val);
        }
        ImGui::ShowTooltipOnHover("Sustain value");

        ImGui::SameLine();
    }

    auto r_dt = static_cast<float>(envelope->PR_dt);
    if (ImGui::Knob("R.dt", &(r_dt), 0, 128, ImVec2(40, 40)))
    {
        envelope->PR_dt = static_cast<unsigned char>(r_dt);
    }
    ImGui::ShowTooltipOnHover("Release time");

    ImGui::SameLine();

    if (envelope->Envmode >= 3)
    {
        auto r_val = static_cast<float>(envelope->PR_val);
        if (ImGui::Knob("R.val", &(r_val), 0, 128, ImVec2(40, 40)))
        {
            envelope->PR_val = static_cast<unsigned char>(r_val);
        }
        ImGui::ShowTooltipOnHover("Release value");

        ImGui::SameLine();
    }

    auto stretch = static_cast<float>(envelope->Penvstretch);
    if (ImGui::Knob("Str.", &(stretch), 0, 128, ImVec2(40, 40)))
    {
        envelope->Penvstretch = static_cast<unsigned char>(stretch);
    }
    ImGui::ShowTooltipOnHover("Envelope stretch (on lower notes makes the envelope longer)");

    ImGui::SameLine();

    auto forcedRelease = envelope->Pforcedrelease != 0;
    if (ImGui::Checkbox("##forcedrelease", &forcedRelease))
    {
        envelope->Pforcedrelease = forcedRelease ? 1 : 0;
    }
    ImGui::ShowTooltipOnHover("Forced Release");

    ImGui::SameLine();

    ImGui::Text("frcR.");
}
