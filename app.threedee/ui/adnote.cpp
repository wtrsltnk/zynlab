#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/ADnoteParams.h>

void AppThreeDee::ADNoteEditor(class ADnoteParameters *parameters)
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

void AppThreeDee::ADNoteEditorAmplitude(ADnoteGlobalParam *parameters)
{
    ImGui::Text("Global Amplitude Parameters");

    auto vol = static_cast<float>(parameters->PVolume);
    if (ImGui::SliderFloat("Vol", &vol, 0, 128))
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
    if (ImGui::SliderFloat("V.Sns", &velocityScale, 0, 128))
    {
        parameters->PAmpVelocityScaleFunction = static_cast<unsigned char>(velocityScale);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Velocity Sensing Function (rightmost to disable)");
        ImGui::EndTooltip();
    }

    auto pan = static_cast<float>(parameters->PPanning);
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        parameters->PPanning = static_cast<unsigned char>(pan);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Panning (leftmost is random)");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    auto punchStrength = static_cast<float>(parameters->PPunchStrength);
    if (ImGui::Knob("P.Str.", &punchStrength, 0, 128, ImVec2(30, 30)))
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

    auto punchTime = static_cast<float>(parameters->PPunchTime);
    if (ImGui::Knob("P.t.", &punchTime, 0, 128, ImVec2(30, 30)))
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

    auto punchStretch = static_cast<float>(parameters->PPunchStretch);
    if (ImGui::Knob("P.Stc.", &punchStretch, 0, 128, ImVec2(30, 30)))
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

    auto punchVelocitySensing = static_cast<float>(parameters->PPunchVelocitySensing);
    if (ImGui::Knob("P.Vel.", &punchVelocitySensing, 0, 128, ImVec2(30, 30)))
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

    ImGui::SameLine();

    auto randomGrouping = parameters->Hrandgrouping == 1;
    if (ImGui::Checkbox("##random_grouping", &randomGrouping))
    {
        parameters->Hrandgrouping = randomGrouping ? 1 : 0;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("How the Harmonic Amplitude is applied to voices that use the same oscillator");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    ImGui::Text("Rnd Grp");

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
    if (ImGui::SliderFloat("Detune", &detune, -35, 35))
    {
        parameters->PDetune = static_cast<unsigned short int>(detune + 8192);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Fine detune (cents)");
        ImGui::EndTooltip();
    }

    auto octave = static_cast<int>(parameters->PCoarseDetune / 1024);
    if (octave >= 8)
    {
        octave -= 16;
    }
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
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Octave");
        ImGui::EndTooltip();
    }

    static char const *detune_types[] = {
        "L35cents",
        "L10cents",
        "E100cents",
        "E1200cents",
    };
    static char const *current_detune_types_item = nullptr;

    auto detune_type = static_cast<int>(parameters->PDetuneType - 1);
    current_detune_types_item = detune_types[detune_type];
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
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Detune type");
        ImGui::EndTooltip();
    }

    auto bandwidth = static_cast<float>(parameters->PBandwidth);
    if (ImGui::Knob("relBW", &(bandwidth), 0, 128, ImVec2(30, 30)))
    {
        parameters->PBandwidth = static_cast<unsigned char>(bandwidth);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Bandwidth - how the relative fine detune of the voice are changed");
        ImGui::EndTooltip();
    }

    ImGui::Separator();

    Envelope("Frequency Envelope", parameters->FreqEnvelope);

    ImGui::Separator();

    LFO("Frequency LFo", parameters->FreqLfo);
}
