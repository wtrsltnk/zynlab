#include "../app.threedee.h"

#include <zyn.synth/ADnoteParams.h>

void AppThreeDee::ADNoteEditor(class ADnoteParameters *parameters)
{
    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 300);
    ImGui::SetColumnWidth(1, 300);

    ADNoteEditorAmplitude(parameters);

    ImGui::NextColumn();

    ADNoteEditorFilter(parameters);

    ImGui::Columns(1);

    ADNoteEditorFrequency(parameters);
}

void AppThreeDee::ADNoteEditorAmplitude(ADnoteParameters *parameters)
{
    ImGui::BeginChild("Amplitude", ImVec2(0, 300), true);

    ImGui::Text("AMPLITUDE");

    ImGui::BeginChild("##globalvars", ImVec2(270, 80), true);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 190);
    ImGui::SetColumnWidth(1, 80);

    auto vol = static_cast<float>(parameters->GlobalPar.PVolume);
    if (ImGui::SliderFloat("Vol", &vol, 0, 128))
    {
        parameters->GlobalPar.PVolume = static_cast<unsigned char>(vol);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Volume");
        ImGui::EndTooltip();
    }

    auto velocityScale = static_cast<float>(parameters->GlobalPar.PAmpVelocityScaleFunction);
    if (ImGui::SliderFloat("VelocityScale", &velocityScale, 0, 128))
    {
        parameters->GlobalPar.PAmpVelocityScaleFunction = static_cast<unsigned char>(velocityScale);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Velocity Sensing Function (rightmost to disable)");
        ImGui::EndTooltip();
    }

    ImGui::NextColumn();

    auto pan = static_cast<float>(parameters->GlobalPar.PPanning);
    if (MyKnob("Panning", &pan, 0, 128, ImVec2(40, 40)))
    {
        parameters->GlobalPar.PPanning = static_cast<unsigned char>(pan);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Panning (leftmost is random)");
        ImGui::EndTooltip();
    }

    ImGui::EndChild();

    AmplitudeEnvelope(parameters->GlobalPar.AmpEnvelope);
    LFO("Amplitude LFO", parameters->GlobalPar.AmpLfo);

    ImGui::EndChild();
}

void AppThreeDee::ADNoteEditorFilter(ADnoteParameters *parameters)
{
    ImGui::BeginChild("Filter", ImVec2(0, 300), true);

    ImGui::Text("FILTER");

    FilterParameters(parameters->GlobalPar.GlobalFilter);

    FilterEnvelope(parameters->GlobalPar.FilterEnvelope);

    LFO("Filter LFo", parameters->GlobalPar.FilterLfo);

    ImGui::EndChild();
}

void AppThreeDee::ADNoteEditorFrequency(ADnoteParameters *parameters)
{
    ImGui::BeginChild("Frequency", ImVec2(584, 200), true);

    ImGui::Text("FREQUENCY");

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 500);
    auto detune = static_cast<float>(parameters->GlobalPar.PDetune) - 8192;
    if (ImGui::SliderFloat("Detune", &detune, -35, 35))
    {
        parameters->GlobalPar.PDetune = static_cast<unsigned short int>(detune + 8192);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Fine detune (cents)");
        ImGui::EndTooltip();
    }

    auto octave = static_cast<int>(parameters->GlobalPar.PCoarseDetune / 1024);
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
        parameters->GlobalPar.PCoarseDetune = static_cast<unsigned short>(octave * 1024 + parameters->GlobalPar.PCoarseDetune % 1024);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Octave");
        ImGui::EndTooltip();
    }

    ImGui::NextColumn();

    auto bandwidth = static_cast<float>(parameters->GlobalPar.PBandwidth);
    if (MyKnob("relBW", &(bandwidth), 0, 128, ImVec2(30, 30)))
    {
        parameters->GlobalPar.PBandwidth = static_cast<unsigned char>(bandwidth);

        parameters->getBandwidthDetuneMultiplier();
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Bandwidth - how the relative fine detune of the voice are changed");
        ImGui::EndTooltip();
    }

    ImGui::Columns(1);

    ImGui::Separator();

    FrequencyEnvelope(parameters->GlobalPar.FreqEnvelope);

    ImGui::SameLine();

    LFO("Frequency LFo", parameters->GlobalPar.FreqLfo);

    ImGui::EndChild();
}
