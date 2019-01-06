#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/LFOParams.h>

static int lfo_type_count = 7;
static char const *lfo_types[] = {
    "SINE",
    "TRI",
    "SQR",
    "R.up",
    "R.dn",
    "E1dn",
    "E2dn",
};

void AppThreeDee::LFO(char const *label, LFOParams *params)
{
    ImGui::Text("%s", label);

    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("##lfotype", params->PLFOtype, lfo_types, lfo_type_count, "LFO Type"))
    {
    }

    if (ImGui::Knob("Freq.", &(params->Pfreq), 0.0f, 1.0f, ImVec2(40, 40), "LFO frequency"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("Depth", &(params->Pintensity), 0, 127, ImVec2(40, 40), "LFO amount"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("Start", &(params->Pstartphase), 0, 127, ImVec2(40, 40), "LFO startphase (leftmost is random)"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("Delay", &(params->Pdelay), 0, 127, ImVec2(40, 40), "LFO delay"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("Str.", &(params->Pstretch), 0, 127, ImVec2(40, 40), "LFO stretch"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("A.R", &(params->Prandomness), 0, 127, ImVec2(30, 30), "LFO Amplitude Randomness"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("F.R", &(params->Pfreqrand), 0, 127, ImVec2(30, 30), "LFO Frequency Randomness"))
    {
    }
}
