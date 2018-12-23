#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.dsp/FilterParams.h>

void AppThreeDee::FilterParameters(FilterParams *parameters)
{
    static char const *categories[] = {
        "Analog",
        "Formant",
        "StVarF",
    };
    static char const *current_category_item = nullptr;

    auto category = static_cast<int>(parameters->Pcategory);
    current_category_item = categories[category];
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("##category", current_category_item))
    {
        for (int n = 0; n < 3; n++)
        {
            bool is_selected = (current_category_item == categories[n]);
            if (ImGui::Selectable(categories[n], is_selected))
            {
                current_category_item = categories[n];
                parameters->Pcategory = static_cast<unsigned char>(n);
                parameters->Ptype = 0;
            }
        }

        ImGui::EndCombo();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("The Category of the Filter (Analog/Formantic/etc.)");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

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
    static char const *current_filter_type_item = nullptr;

    static char const *stvarf_filter_types[] = {
        "1LPDF",
        "1HPF",
        "1BPF",
        "1NF",
    };
    static char const *current_stvarf_filter_type_item = nullptr;

    switch (category)
    {
        default:
        {
            auto filter_type = static_cast<int>(parameters->Ptype);
            current_filter_type_item = filter_types[filter_type];
            ImGui::PushItemWidth(100);
            if (ImGui::BeginCombo("##filter_type", current_filter_type_item))
            {
                for (int n = 0; n < 9; n++)
                {
                    bool is_selected = (current_filter_type_item == filter_types[n]);
                    if (ImGui::Selectable(filter_types[n], is_selected))
                    {
                        current_filter_type_item = filter_types[n];
                        parameters->Ptype = static_cast<unsigned char>(n);
                        parameters->changed = true;
                    }
                }

                ImGui::EndCombo();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Filter type");
                ImGui::EndTooltip();
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
            auto filter_type = static_cast<int>(parameters->Ptype);
            current_stvarf_filter_type_item = stvarf_filter_types[filter_type];
            ImGui::PushItemWidth(100);
            if (ImGui::BeginCombo("##stvarf_filter_type", current_stvarf_filter_type_item))
            {
                for (int n = 0; n < 4; n++)
                {
                    bool is_selected = (current_stvarf_filter_type_item == stvarf_filter_types[n]);
                    if (ImGui::Selectable(stvarf_filter_types[n], is_selected))
                    {
                        current_stvarf_filter_type_item = stvarf_filter_types[n];
                        parameters->Ptype = static_cast<unsigned char>(n);
                    }
                }

                ImGui::EndCombo();
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Filter type");
                ImGui::EndTooltip();
            }
            break;
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    auto freq = static_cast<float>(parameters->Pfreq);
    if (ImGui::Knob("C.Freq", &(freq), 0, 128, ImVec2(40, 40)))
    {
        parameters->Pfreq = static_cast<unsigned char>(freq);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Center Frequency of the Filter or the base position in the vowel's sequence");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto q = static_cast<float>(parameters->Pq);
    if (ImGui::Knob("Q", &(q), 0, 128, ImVec2(40, 40)))
    {
        parameters->Pq = static_cast<unsigned char>(q);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Filter resonance or bandwidth");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto freqtrack = static_cast<float>(parameters->Pfreqtrack);
    if (ImGui::Knob("freq.tr.", &(freqtrack), 0, 128, ImVec2(40, 40)))
    {
        parameters->Pfreqtrack = static_cast<unsigned char>(freqtrack);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Filter frequency tracking (left is negative, middle is 0, and right is positive)");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto gain = static_cast<float>(parameters->Pgain);
    if (ImGui::Knob("gain", &(gain), 0, 128, ImVec2(40, 40)))
    {
        parameters->Pgain = static_cast<unsigned char>(gain);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Filter output gain/damp");
        ImGui::EndTooltip();
    }

    ImGui::Spacing();
    ImGui::Spacing();
}
