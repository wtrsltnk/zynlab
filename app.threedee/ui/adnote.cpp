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
    if (ImGui::SliderFloat("Vol", &velocityScale, 0, 128))
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
    LFO(parameters->GlobalPar.AmpLfo);

    ImGui::EndChild();
}

void AppThreeDee::ADNoteEditorFilter(ADnoteParameters *parameters)
{
    ImGui::BeginChild("Filter", ImVec2(0, 300), true);

    ImGui::Text("FILTER");

    ImGui::BeginChild("##globalfiltervars", ImVec2(270, 80), true);

    ImGui::EndChild();

    ImGui::EndChild();
}

void AppThreeDee::ADNoteEditorFrequency(ADnoteParameters *parameters)
{
    ImGui::BeginChild("Frequency", ImVec2(0, 200), true);

    ImGui::Text("FREQUENCY");

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

    ImGui::BeginChild("##globalfrequencyvars", ImVec2(570, 80), true);

    ImGui::EndChild();

    ImGui::EndChild();
}
