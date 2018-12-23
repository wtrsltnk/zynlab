#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/PADnoteParams.h>

void AppThreeDee::PADNoteEditor(PADnoteParameters *parameters)
{
    if (!showPADNoteEditor)
    {
        return;
    }

    ImGui::Begin("PAD note editor", &showPADNoteEditor);

    if (ImGui::BeginTabBar("PADnoteTab"))
    {
        if (ImGui::BeginTabItem("Harmonic structure"))
        {
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Envelopes LFOs"))
        {
            ImGui::Text("PADsynth Envelopes and LFOs Parameters of the Instrument");

            ImGui::Spacing();
            ImGui::Spacing();

            if (ImGui::BeginTabBar("PADNote"))
            {
                if (ImGui::BeginTabItem("Amplitude"))
                {
                    PADNoteEditorAmplitude(parameters);

                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void AppThreeDee::PADNoteEditorAmplitude(PADnoteParameters *parameters)
{
    ImGui::Text("Global Amplitude Parameters");

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::BeginChild("VolSns", ImVec2(250, 50));
    auto vol = static_cast<float>(parameters->PVolume);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderFloat("##Vol", &vol, 0, 128, "Vol %.3f"))
    {
        parameters->PVolume = static_cast<unsigned char>(vol);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Volume");
        ImGui::EndTooltip();
    }

    auto velocityScale = static_cast<float>(parameters->PAmpVelocityScaleFunction);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderFloat("##V.Sns", &velocityScale, 0, 128, "V.Sns %.3f"))
    {
        parameters->PAmpVelocityScaleFunction = static_cast<unsigned char>(velocityScale);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Velocity Sensing Function (rightmost to disable)");
        ImGui::EndTooltip();
    }
    ImGui::EndChild();

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto pan = static_cast<float>(parameters->PPanning);
    if (ImGui::Knob("Panning", &pan, 0, 128, ImVec2(40, 40)))
    {
        parameters->PPanning = static_cast<unsigned char>(pan);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Panning (leftmost is random)");
        ImGui::EndTooltip();
    }

    ImGui::Spacing();
    ImGui::Spacing();

    auto punchStrength = static_cast<float>(parameters->PPunchStrength);
    if (ImGui::Knob("P.Str.", &punchStrength, 0, 128, ImVec2(40, 40)))
    {
        parameters->PPunchStrength = static_cast<unsigned char>(punchStrength);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Punch Strength");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto punchTime = static_cast<float>(parameters->PPunchTime);
    if (ImGui::Knob("P.t.", &punchTime, 0, 128, ImVec2(40, 40)))
    {
        parameters->PPunchTime = static_cast<unsigned char>(punchTime);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Punch time (duration)");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto punchStretch = static_cast<float>(parameters->PPunchStretch);
    if (ImGui::Knob("P.Stc.", &punchStretch, 0, 128, ImVec2(40, 40)))
    {
        parameters->PPunchStretch = static_cast<unsigned char>(punchStretch);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Punch Stretch");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto punchVelocitySensing = static_cast<float>(parameters->PPunchVelocitySensing);
    if (ImGui::Knob("P.Vel.", &punchVelocitySensing, 0, 128, ImVec2(40, 40)))
    {
        parameters->PPunchVelocitySensing = static_cast<unsigned char>(punchVelocitySensing);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Punch Velocity Sensing");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto stereo = parameters->PStereo == 1;
    if (ImGui::Checkbox("##stereo", &stereo))
    {
        parameters->PStereo = stereo ? 1 : 0;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Stereo");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    ImGui::Text("Stereo");

    ImGui::Spacing();
    ImGui::Spacing();

    ImGui::Separator();

    Envelope("Amplitude Envelope", parameters->AmpEnvelope);

    ImGui::Separator();

    LFO("Amplitude LFO", parameters->AmpLfo);
}
