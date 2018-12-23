#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/SUBnoteParams.h>

void AppThreeDee::SUBNoteEditor(SUBnoteParameters *parameters)
{
    if (!showSUBNoteEditor)
    {
        return;
    }

    ImGui::Begin("SUB note editor", &showSUBNoteEditor);
    if (ImGui::BeginTabBar("SUBnoteTab"))
    {
        if (ImGui::BeginTabItem("Global"))
        {
            if (_sequencer.ActiveInstrument() >= 0)
            {
                ImGui::Text("SUBsynth Global Parameters of the Instrument");

                ImGui::Spacing();
                ImGui::Spacing();

                if (ImGui::BeginTabBar("SUBNote"))
                {
                    if (ImGui::BeginTabItem("Amplitude"))
                    {
                        SUBNoteEditorAmplitude(parameters);

                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Bandwidth"))
                    {
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Overtones"))
                    {
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Filter"))
                    {
                        ImGui::EndTabItem();
                    }

                    if (ImGui::BeginTabItem("Frequency"))
                    {
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void AppThreeDee::SUBNoteEditorAmplitude(SUBnoteParameters *parameters)
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

    ImGui::Separator();

    Envelope("Amplitude Envelope", &parameters->AmpEnvelope);
}
