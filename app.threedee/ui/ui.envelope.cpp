#include "ui.envelope.h"
#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <imgui.h>
#include <zyn.synth/EnvelopeParams.h>

zyn::ui::Envelope::Envelope(char const *label)
    : _label(label)
{}

void zyn::ui::Envelope::Render(EnvelopeParams *envelope)
{
    ImGui::Text("%s", _label);

    if (envelope == nullptr)
    {
        return;
    }

    if (envelope->Envmode >= 3)
    {
        if (ImGui::KnobUchar("A.val", &(envelope->PA_val), 0, 127, ImVec2(40, 40), "Starting value"))
        {
        }

        ImGui::SameLine();
    }

    if (ImGui::KnobUchar("A.dt", &(envelope->PA_dt), 0, 127, ImVec2(40, 40), "Attack time"))
    {
    }

    ImGui::SameLine();

    if (envelope->Envmode == 4)
    {
        if (ImGui::KnobUchar("D.val", &(envelope->PD_val), 0, 127, ImVec2(40, 40), "Decay value"))
        {
        }

        ImGui::SameLine();
    }

    if (envelope->Envmode != 3 && envelope->Envmode != 5)
    {
        if (ImGui::KnobUchar("D.dt", &(envelope->PD_dt), 0, 127, ImVec2(40, 40), "Decay time"))
        {
        }

        ImGui::SameLine();
    }

    if (envelope->Envmode < 3)
    {
        if (ImGui::KnobUchar("S.val", &(envelope->PS_val), 0, 127, ImVec2(40, 40), "Sustain value"))
        {
        }

        ImGui::SameLine();
    }

    if (ImGui::KnobUchar("R.dt", &(envelope->PR_dt), 0, 127, ImVec2(40, 40), "Release time"))
    {
    }

    ImGui::SameLine();

    if (envelope->Envmode >= 3)
    {
        if (ImGui::KnobUchar("R.val", &(envelope->PR_val), 0, 127, ImVec2(40, 40), "Release value"))
        {
        }

        ImGui::SameLine();
    }

    if (ImGui::KnobUchar("Str.", &(envelope->Penvstretch), 0, 127, ImVec2(40, 40), "Envelope stretch (on lower notes makes the envelope longer)"))
    {
    }

    ImGui::SameLine();

    auto forcedRelease = envelope->Pforcedrelease != 0;
    if (ImGui::Checkbox("frcR.", &forcedRelease))
    {
        envelope->Pforcedrelease = forcedRelease ? 1 : 0;
    }
    ImGui::ShowTooltipOnHover("Forced Release");
}
