#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.dsp/FilterParams.h>

static const int category_count = 3;
static char const *categories[] = {
    "Analog",
    "Formant",
    "StVarF",
};

static const int filter_type_count = 9;
static char const *filter_types[] = {
    "LPF1",
    "HPF1",
    "LPF2",
    "HPF2",
    "BPF2",
    "NF2",
    "PkF2",
    "LSh2",
    "HSh2",
};

static const int stvarf_filter_type_count = 4;
static char const *stvarf_filter_types[] = {
    "1LPDF",
    "1HPF",
    "1BPF",
    "1NF",
};

void AppThreeDee::FilterParameters(FilterParams *parameters)
{
    ImGui::PushItemWidth(100);
    if (ImGui::PresetSelection("##category", parameters->Pcategory, categories, category_count, "The Category of the Filter (Analog/Formantic/etc.)"))
    {
    }

    ImGui::SameLine();

    switch (parameters->Pcategory)
    {
        default:
        {
            ImGui::PushItemWidth(100);
            if (ImGui::PresetSelection("##filter_type", parameters->Ptype, filter_types, filter_type_count, "Filter type"))
            {
            }
            break;
        }
        case 1:
        {
            ImGui::Button("Edit");
            break;
        }
        case 2:
        {
            ImGui::PushItemWidth(100);
            if (ImGui::PresetSelection("##stvarf_filter_type", parameters->Ptype, stvarf_filter_types, stvarf_filter_type_count, "Filter type"))
            {
            }
            break;
        }
    }

    if (ImGui::KnobUchar("C.Freq", &(parameters->Pfreq), 0, 127, ImVec2(40, 40), "Center Frequency of the Filter or the base position in the vowel's sequence"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("Q", &(parameters->Pq), 0, 127, ImVec2(40, 40), "Filter resonance or bandwidth"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("freq.tr.", &(parameters->Pfreqtrack), 0, 127, ImVec2(40, 40), "Filter frequency tracking (left is negative, middle is 0, and right is positive)"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("gain", &(parameters->Pgain), 0, 127, ImVec2(40, 40), "Filter output gain/damp"))
    {
    }
}
