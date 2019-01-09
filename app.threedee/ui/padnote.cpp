#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/PADnoteParams.h>

char const *const PADeditorID = "PAD editor";

void AppThreeDee::PADNoteEditor(Channel *channel, int instrumentIndex)
{
    if (!_showPADNoteEditor || channel == nullptr || instrumentIndex < 0 || instrumentIndex >= NUM_CHANNEL_INSTRUMENTS)
    {
        return;
    }

    auto *parameters = channel->instruments[instrumentIndex].padpars;

    if (channel->instruments[instrumentIndex].Ppadenabled == 0)
    {
        ImGui::Text("PAD editor is disabled");
        if (ImGui::Button("Enable PAD synth"))
        {
            channel->instruments[instrumentIndex].Ppadenabled = 1;
        }
        return;
    }

    if (ImGui::BeginTabBar("PADnoteTab"))
    {
        if (ImGui::BeginTabItem("Envelopes LFOs"))
        {
            ImGui::Text("PADsynth Envelopes and LFOs Parameters of the Instrument");

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
        if (ImGui::BeginTabItem("Harmonic structure"))
        {
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

void AppThreeDee::PADNoteEditorAmplitude(PADnoteParameters *parameters)
{
    ImGui::Text("Global Amplitude Parameters");

    ImGui::BeginChild("VolSns", ImVec2(250, 50));
    auto vol = static_cast<float>(parameters->PVolume);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderFloat("##Vol", &vol, 0, 127, "Vol %.3f"))
    {
        parameters->PVolume = static_cast<unsigned char>(vol);
    }
    ImGui::ShowTooltipOnHover("Volume");

    auto velocityScale = static_cast<float>(parameters->PAmpVelocityScaleFunction);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderFloat("##V.Sns", &velocityScale, 0, 127, "V.Sns %.3f"))
    {
        parameters->PAmpVelocityScaleFunction = static_cast<unsigned char>(velocityScale);
    }
    ImGui::ShowTooltipOnHover("Velocity Sensing Function (rightmost to disable)");
    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::KnobUchar("Panning", &parameters->PPanning, 0, 127, ImVec2(40, 40), "Panning (leftmost is random)"))
    {
    }

    if (ImGui::KnobUchar("P.Str.", &parameters->PPunchStrength, 0, 127, ImVec2(40, 40), "Punch Strength"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("P.t.", &parameters->PPunchTime, 0, 127, ImVec2(40, 40), "Punch time (duration)"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("P.Stc.", &parameters->PPunchStretch, 0, 127, ImVec2(40, 40), "Punch Stretch"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("P.Vel.", &parameters->PPunchVelocitySensing, 0, 127, ImVec2(40, 40), "Punch Velocity Sensing"))
    {
    }

    ImGui::SameLine();

    auto stereo = parameters->PStereo == 1;
    if (ImGui::Checkbox("Stereo", &stereo))
    {
        parameters->PStereo = stereo ? 1 : 0;
    }
    ImGui::ShowTooltipOnHover("Stereo");

    ImGui::Separator();

    Envelope("Amplitude Envelope", parameters->AmpEnvelope);

    ImGui::Separator();

    LFO("Amplitude LFO", parameters->AmpLfo);
}
