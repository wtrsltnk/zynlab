#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/SUBnoteParams.h>

static char const *overtone_positions[] = {
    "Harmonic",
    "ShiftU",
    "ShiftL",
    "PowerU",
    "PowerL",
    "Sine",
    "Power",
    "Shift",
};

char const *const SUBeditorID = "SUB editor";

void AppThreeDee::SUBNoteEditor(SUBnoteParameters *parameters)
{
    if (!_showSUBNoteEditor)
    {
        return;
    }

    ImGui::Begin(SUBeditorID, &_showSUBNoteEditor);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));

    if (ImGui::BeginTabBar("SUBnoteTab"))
    {
        if (ImGui::BeginTabItem("Global"))
        {
            if (_sequencer.ActiveInstrument() >= 0)
            {
                ImGui::Text("SUBsynth Global Parameters of the Instrument");

                if (ImGui::BeginTabBar("SUBNote"))
                {
                    if (ImGui::BeginTabItem("Amplitude"))
                    {
                        SUBNoteEditorAmplitude(parameters);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Bandwidth"))
                    {
                        SUBNoteEditorBandwidth(parameters);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Overtones"))
                    {
                        SUBNoteEditorOvertones(parameters);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Filter"))
                    {
                        SUBNoteEditorFilter(parameters);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Frequency"))
                    {
                        SUBNoteEditorFrequency(parameters);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Harmonics"))
                    {
                        SUBNoteEditorHarmonicsMagnitude(parameters);

                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

void AppThreeDee::SUBNoteEditorHarmonicsMagnitude(SUBnoteParameters *parameters)
{
    ImGui::BeginChild("Harmonics", ImVec2(), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
    for (int i = 0; i < MAX_SUB_HARMONICS; i++)
    {
        ImGui::PushID(i);
        if (i != 0)
        {
            ImGui::SameLine();
        }
        int v = static_cast<int>(parameters->Phmag[i]);
        if (ImGui::VSliderInt("##harmonic", ImVec2(10, 100), &v, 0, 127, ""))
        {
            parameters->Phmag[i] = static_cast<unsigned char>(v);
        }
        ImGui::PopID();
    }

    for (int i = 0; i < MAX_SUB_HARMONICS; i++)
    {
        ImGui::PushID(i);
        if (i != 0)
        {
            ImGui::SameLine();
        }
        int v = static_cast<int>(parameters->Phrelbw[i]);
        if (ImGui::VSliderInt("##subharmonic", ImVec2(10, 100), &v, 0, 127, ""))
        {
            parameters->Phrelbw[i] = static_cast<unsigned char>(v);
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
}

void AppThreeDee::SUBNoteEditorAmplitude(SUBnoteParameters *parameters)
{
    ImGui::Text("Global Amplitude Parameters");

    ImGui::BeginChild("VolSns", ImVec2(250, 50));
    auto vol = static_cast<float>(parameters->PVolume);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderFloat("##Vol", &vol, 0, 128, "Vol %.3f"))
    {
        parameters->PVolume = static_cast<unsigned char>(vol);
    }
    ImGui::ShowTooltipOnHover("Volume");

    auto velocityScale = static_cast<float>(parameters->PAmpVelocityScaleFunction);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderFloat("##V.Sns", &velocityScale, 0, 128, "V.Sns %.3f"))
    {
        parameters->PAmpVelocityScaleFunction = static_cast<unsigned char>(velocityScale);
    }
    ImGui::ShowTooltipOnHover("Velocity Sensing Function (rightmost to disable)");
    ImGui::EndChild();

    ImGui::SameLine();

    if (ImGui::KnobUchar("Panning", &parameters->PPanning, 0, 127, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("Panning (leftmost is random)");

    ImGui::Separator();

    Envelope("Amplitude Envelope", &parameters->AmpEnvelope);
}

void AppThreeDee::SUBNoteEditorBandwidth(SUBnoteParameters *parameters)
{
    ImGui::Text("Bandwidth Parameters");

    auto bandwidth = static_cast<float>(parameters->Pbandwidth);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderFloat("##Bandwidth", &bandwidth, 0, 128, "Bandwidth %.3f"))
    {
        parameters->Pbandwidth = static_cast<unsigned char>(bandwidth);
    }
    ImGui::ShowTooltipOnHover("Bandwidth");

    auto bandwidthScale = static_cast<float>(parameters->Pbwscale);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderFloat("##BandwidthScale", &bandwidthScale, 0, 128, "Scale %.3f"))
    {
        parameters->Pbwscale = static_cast<unsigned char>(bandwidthScale);
    }
    ImGui::ShowTooltipOnHover("Bandwidth Scale");

    ImGui::Separator();

    bool envelopeEnabled = parameters->PFreqEnvelopeEnabled == 1;
    if (ImGui::Checkbox("##subnotefilterenabled", &envelopeEnabled))
    {
        parameters->PFreqEnvelopeEnabled = envelopeEnabled ? 1 : 0;
    }

    ImGui::SameLine();

    if (!envelopeEnabled)
    {
        ImGui::Text("Bandwidth Envelope");
    }
    else
    {
        Envelope("Bandwidth Envelope", parameters->BandWidthEnvelope);
    }
}

void AppThreeDee::SUBNoteEditorOvertones(SUBnoteParameters *parameters)
{
    ImGui::Text("Overtone Parameters");

    static char const *current_overtone_positions_item = nullptr;

    auto overtone_position = static_cast<int>(parameters->POvertoneSpread.type);
    current_overtone_positions_item = overtone_positions[overtone_position];
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("Overtone positions", current_overtone_positions_item))
    {
        for (int n = 0; n < 8; n++)
        {
            bool is_selected = (current_overtone_positions_item == overtone_positions[n]);
            if (ImGui::Selectable(overtone_positions[n], is_selected))
            {
                current_overtone_positions_item = overtone_positions[n];
                parameters->POvertoneSpread.type = static_cast<unsigned char>(n);
            }
        }

        ImGui::EndCombo();
    }
    ImGui::ShowTooltipOnHover("Overtone positions");

    if (ImGui::KnobUchar("Par1", &(parameters->POvertoneSpread.par1), 0, 127, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("Overtone spread par 1");

    ImGui::SameLine();

    if (ImGui::KnobUchar("Par2", &(parameters->POvertoneSpread.par2), 0, 127, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("Overtone spread par 2");

    ImGui::SameLine();

    if (ImGui::KnobUchar("ForceH", &(parameters->POvertoneSpread.par3), 0, 127, ImVec2(40, 40)))
    {
    }
    ImGui::ShowTooltipOnHover("Overtone spread par 3");
}

void AppThreeDee::SUBNoteEditorFilter(SUBnoteParameters *parameters)
{
    bool filterEnabled = parameters->PGlobalFilterEnabled == 1;
    if (ImGui::Checkbox("##subnotefilterenabled", &filterEnabled))
    {
        parameters->PGlobalFilterEnabled = filterEnabled ? 1 : 0;
    }

    ImGui::SameLine();

    ImGui::Text("Global Filter Parameters");

    if (filterEnabled)
    {
        FilterParameters(parameters->GlobalFilter);

        ImGui::Separator();

        Envelope("Filter Envelope", parameters->GlobalFilterEnvelope);
    }
}

void AppThreeDee::SUBNoteEditorFrequency(SUBnoteParameters *parameters)
{
    ImGui::Text("Global Frequency Parameters");

    auto detune = static_cast<float>(parameters->PDetune) - 8192;
    ImGui::PushItemWidth(300);
    if (ImGui::SliderFloat("##Detune", &detune, -35, 35, "Detune %.3f"))
    {
        parameters->PDetune = static_cast<unsigned short int>(detune + 8192);
    }
    ImGui::ShowTooltipOnHover("Fine detune (cents)");

    ImGui::SameLine();

    auto octave = static_cast<int>(parameters->PCoarseDetune / 1024);
    if (octave >= 8)
    {
        octave -= 16;
    }
    ImGui::PushItemWidth(100);
    if (ImGui::InputInt("Octave", &octave))
    {
        if (octave < -8)
        {
            octave = -8;
        }
        else if (octave > 7)
        {
            octave = 7;
        }

        if (octave < 0)
        {
            octave += 16;
        }
        parameters->PCoarseDetune = static_cast<unsigned short>(octave * 1024 + parameters->PCoarseDetune % 1024);
    }
    ImGui::ShowTooltipOnHover("Octave");

    ImGui::Separator();

    bool envelopeEnabled = parameters->PFreqEnvelopeEnabled == 1;
    if (ImGui::Checkbox("##subnotefilterenabled", &envelopeEnabled))
    {
        parameters->PFreqEnvelopeEnabled = envelopeEnabled ? 1 : 0;
    }

    ImGui::SameLine();

    if (!envelopeEnabled)
    {
        ImGui::Text("Frequency Envelope");
    }
    else
    {
        Envelope("Frequency Envelope", parameters->FreqEnvelope);
    }
}
