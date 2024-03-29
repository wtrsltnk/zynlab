#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/LFOParams.h>

static std::vector<std::string> lfo_types = {
    "SINE",
    "TRI",
    "SQR",
    "R.up",
    "R.dn",
    "E1dn",
    "E2dn",
};

zyn::ui::Lfo::Lfo(char const *label) : _label(label) {}

void zyn::ui::Lfo::Render(LFOParams *params, unsigned char *enabled)
{
    bool lfoEnabled = enabled == nullptr || (*enabled) == 1;

    if (enabled != nullptr)
    {
        if (ImGui::Checkbox("##LfoEnabled", &lfoEnabled))
        {
            (*enabled) = lfoEnabled ? 1 : 0;
        }
        ImGui::ShowTooltipOnHover("Enable this LFO");

        ImGui::SameLine();
    }

    ImGui::Text("%s", _label);

    if (params == nullptr)
    {
        return;
    }

    if (!lfoEnabled)
    {
        return;
    }

    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("##lfotype", params->PLFOtype, lfo_types, "LFO Type"))
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
