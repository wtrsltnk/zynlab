#include "ui.padnote.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.mixer/Mixer.h>
#include <zyn.synth/PADnoteParams.h>

char const *const PadSynthEditorID = "PAD editor";

zyn::ui::PadNote::PadNote(AppState *state)
    : _state(state), _AmplitudeEnvelope("Amplitude Envelope"), _AmplitudeLfo("Amplitude LFO")
{}

zyn::ui::PadNote::~PadNote() = default;

bool zyn::ui::PadNote::Setup()
{
    return true;
}

void zyn::ui::PadNote::Render()
{
    auto channel = _state->_mixer->GetChannel(_state->_activeChannel);

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
    ImGui::Begin(PadSynthEditorID, &_state->_showPADNoteEditor);
    if (!_state->_showPADNoteEditor || channel == nullptr || _state->_activeChannelInstrument < 0 || _state->_activeChannelInstrument >= NUM_CHANNEL_INSTRUMENTS)
    {
        ImGui::End();
        return;
    }

    auto *parameters = channel->Instruments[_state->_activeChannelInstrument].padpars;

    if (channel->Instruments[_state->_activeChannelInstrument].Ppadenabled == 0)
    {
        ImGui::Text("PAD editor is disabled");
        if (ImGui::Button("Enable PAD synth"))
        {
            channel->Instruments[_state->_activeChannelInstrument].Ppadenabled = 1;
        }
        ImGui::End();
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
    ImGui::End();
    ImGui::PopStyleVar();
}

void zyn::ui::PadNote::PADNoteEditorAmplitude(PADnoteParameters *parameters)
{
    ImGui::Text("Global Amplitude Parameters");

    ImGui::BeginChild("VolSns", ImVec2(250, 50));
    auto vol = static_cast<int>(parameters->PVolume);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderInt("##Vol", &vol, 0, 127, "Vol %d"))
    {
        parameters->PVolume = static_cast<unsigned char>(vol);
    }
    ImGui::ShowTooltipOnHover("Volume");

    auto velocityScale = static_cast<int>(parameters->PAmpVelocityScaleFunction);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderInt("##V.Sns", &velocityScale, 0, 127, "V.Sns %d"))
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

    _AmplitudeEnvelope.Render(parameters->AmpEnvelope);

    ImGui::Separator();

    _AmplitudeLfo.Render(parameters->AmpLfo);
}
