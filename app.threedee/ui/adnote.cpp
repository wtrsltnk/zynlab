#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/ADnoteParams.h>

static char voiceIds[][64]{
    "Voice 1",
    "Voice 2",
    "Voice 3",
    "Voice 4",
    "Voice 5",
    "Voice 6",
    "Voice 7",
    "Voice 8",
};

static char const *detune_types[] = {
    "L35cents",
    "L10cents",
    "E100cents",
    "E1200cents",
};

char const *const ADeditorID = "AD editor";

void AppThreeDee::ADNoteEditor(class ADnoteParameters *parameters)
{
    if (!_showADNoteEditor)
    {
        return;
    }

    ImGui::Begin(ADeditorID, &_showADNoteEditor);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));

    if (ImGui::BeginTabBar("ADnoteTab"))
    {
        if (ImGui::BeginTabItem("Global"))
        {
            if (_sequencer.ActiveInstrument() >= 0)
            {
                ImGui::Text("ADsynth Global Parameters of the Instrument");

                if (ImGui::BeginTabBar("ADNote"))
                {
                    if (ImGui::BeginTabItem("Amplitude"))
                    {
                        ADNoteEditorAmplitude(&parameters->GlobalPar);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Filter"))
                    {
                        ADNoteEditorFilter(&parameters->GlobalPar);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Frequency"))
                    {
                        ADNoteEditorFrequency(&parameters->GlobalPar);

                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
            ImGui::EndTabItem();
        }
        for (int i = 0; i < NUM_VOICES; i++)
        {
            if (_sequencer.ActiveInstrument() >= 0)
            {
                auto parameters = &_mixer->GetChannel(_sequencer.ActiveInstrument())->instruments[0].adpars->VoicePar[i];
                if (ImGui::BeginTabItem(voiceIds[i]))
                {
                    ADNoteVoiceEditor(parameters);

                    ImGui::EndTabItem();
                }
            }
        }
        ImGui::EndTabBar();
    }

    ImGui::PopStyleVar();
    ImGui::End();
}

void AppThreeDee::ADNoteEditorAmplitude(ADnoteGlobalParam *parameters)
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

    ImGui::SameLine();

    auto randomGrouping = parameters->Hrandgrouping == 1;
    if (ImGui::Checkbox("Rnd Grp", &randomGrouping))
    {
        parameters->Hrandgrouping = randomGrouping ? 1 : 0;
    }
    ImGui::ShowTooltipOnHover("How the Harmonic Amplitude is applied to voices that use the same oscillator");

    ImGui::Separator();

    Envelope("Amplitude Envelope", parameters->AmpEnvelope);

    ImGui::Separator();

    LFO("Amplitude LFO", parameters->AmpLfo);
}

void AppThreeDee::ADNoteEditorFilter(ADnoteGlobalParam *parameters)
{
    ImGui::Text("Global Filter Parameters");

    FilterParameters(parameters->GlobalFilter);

    ImGui::Separator();

    Envelope("Filter Envelope", parameters->FilterEnvelope);

    ImGui::Separator();

    LFO("Filter LFo", parameters->FilterLfo);
}

void AppThreeDee::ADNoteEditorFrequency(ADnoteGlobalParam *parameters)
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

    if (ImGui::KnobUchar("relBW", &(parameters->PBandwidth), 0, 127, ImVec2(40, 40), "Bandwidth - how the relative fine detune of the voice are changed"))
    {
    }

    ImGui::SameLine();

    static char const *current_detune_types_item = nullptr;

    auto detune_type = static_cast<int>(parameters->PDetuneType - 1);
    current_detune_types_item = detune_types[detune_type];
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("Detune type", current_detune_types_item))
    {
        for (int n = 0; n < 4; n++)
        {
            bool is_selected = (current_detune_types_item == detune_types[n]);
            if (ImGui::Selectable(detune_types[n], is_selected))
            {
                current_detune_types_item = detune_types[n];
                parameters->PDetuneType = static_cast<unsigned char>(n) + 1;
            }
        }

        ImGui::EndCombo();
    }
    ImGui::ShowTooltipOnHover("Detune type");

    ImGui::Separator();

    Envelope("Frequency Envelope", parameters->FreqEnvelope);

    ImGui::Separator();

    LFO("Frequency LFo", parameters->FreqLfo);
}
