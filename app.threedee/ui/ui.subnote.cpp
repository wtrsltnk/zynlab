#include "ui.subnote.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.mixer/Mixer.h>
#include <zyn.synth/SUBnoteParams.h>

static const int overtone_position_count = 8;
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

static char const *const mag_types[] = {
    "Linear",
    "-40dB",
    "-60dB",
    "-80dB",
    "-100dB",
};
static unsigned int mag_type_count = 5;

static char const *const start_types[] = {
    "Zero",
    "RND",
    "Max.",
};
static unsigned int start_type_count = 3;

char const *const SubSynthEditorID = "SUB editor";

char const *const SmplSynthEditorID = "SMPL editor";

zyn::ui::SubNote::SubNote(AppState *state)
    : _state(state), _AmplitudeEnvelope("Amplitude Envelope"), _BandwidthEnvelope("Bandwidth Envelope"),
      _FilterEnvelope("Filter Envelope"), _FrequencyEnvelope("Frequency Envelope")
{}

zyn::ui::SubNote::~SubNote() = default;

bool zyn::ui::SubNote::Setup() { return true; }

void zyn::ui::SubNote::Render()
{
    auto track = _state->_mixer->GetTrack(_state->_currentTrack);
    ImGui::Begin(SubSynthEditorID, &_state->_showSUBNoteEditor);
    if (!_state->_showSUBNoteEditor || track == nullptr || _state->_currentTrackInstrument < 0 || _state->_currentTrackInstrument >= NUM_TRACK_INSTRUMENTS)
    {
        ImGui::End();
        return;
    }

    auto *parameters = track->Instruments[_state->_currentTrackInstrument].subpars;

    if (track->Instruments[_state->_currentTrackInstrument].Psubenabled == 0)
    {
        ImGui::Text("SUB editor is disabled");
        if (ImGui::Button("Enable SUB synth"))
        {
            track->Instruments[_state->_currentTrackInstrument].Psubenabled = 1;
        }
        ImGui::End();
        return;
    }

    if (ImGui::BeginTabBar("SUBnoteTab"))
    {
        if (ImGui::BeginTabItem("Global"))
        {
            if (_state->_currentTrack >= 0)
            {
                ImGui::Text("SUBsynth Global Parameters of the Instrument");

                auto stereo = parameters->Pstereo == 1;
                if (ImGui::Checkbox("Stereo", &stereo))
                {
                    parameters->Pstereo = stereo ? 1 : 0;
                }
                ImGui::ShowTooltipOnHover("Stereo");

                ImGui::SameLine();

                auto filterStages = static_cast<int>(parameters->Pnumstages);
                ImGui::PushItemWidth(100);
                if (ImGui::SliderInt("Filter Stages", &filterStages, 1, 5))
                {
                    parameters->Pnumstages = static_cast<unsigned char>(filterStages);
                }
                ImGui::ShowTooltipOnHover("How many times the noise is filtered");

                ImGui::PushItemWidth(100);
                if (ImGui::DropDown("##MagType", parameters->Phmagtype, mag_types, mag_type_count, "Mag type"))
                {
                }

                ImGui::SameLine();

                ImGui::PushItemWidth(100);
                if (ImGui::DropDown("##Start", parameters->Pstart, start_types, start_type_count, "Start"))
                {
                }

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
    ImGui::End();
}

void zyn::ui::SubNote::SUBNoteEditorHarmonicsMagnitude(SUBnoteParameters *parameters)
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

void zyn::ui::SubNote::SUBNoteEditorAmplitude(SUBnoteParameters *parameters)
{
    ImGui::Text("Global Amplitude Parameters");

    ImGui::BeginChild("VolSns", ImVec2(250, ImGui::GetStyle().ItemSpacing.y * 3 + ImGui::GetTextLineHeight() * 2));
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

    ImGui::Separator();

    _AmplitudeEnvelope.Render(parameters->AmpEnvelope);
}

void zyn::ui::SubNote::SUBNoteEditorBandwidth(SUBnoteParameters *parameters)
{
    ImGui::Text("Bandwidth Parameters");

    auto bandwidth = static_cast<int>(parameters->Pbandwidth);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderInt("##Bandwidth", &bandwidth, 0, 127, "Bandwidth %d"))
    {
        parameters->Pbandwidth = static_cast<unsigned char>(bandwidth);
    }
    ImGui::ShowTooltipOnHover("Bandwidth");

    auto bandwidthScale = static_cast<int>(parameters->Pbwscale);
    ImGui::PushItemWidth(250);
    if (ImGui::SliderInt("##BandwidthScale", &bandwidthScale, 0, 127, "Scale %d"))
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

    _BandwidthEnvelope.Render(envelopeEnabled ? parameters->BandWidthEnvelope : nullptr);
}

void zyn::ui::SubNote::SUBNoteEditorOvertones(SUBnoteParameters *parameters)
{
    ImGui::Text("Overtone Parameters");

    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Overtone positions", parameters->POvertoneSpread.type, overtone_positions, overtone_position_count, "Overtone positions"))
    {
    }

    if (ImGui::KnobUchar("Par1", &(parameters->POvertoneSpread.par1), 0, 127, ImVec2(40, 40), "Overtone spread par 1"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("Par2", &(parameters->POvertoneSpread.par2), 0, 127, ImVec2(40, 40), "Overtone spread par 2"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("ForceH", &(parameters->POvertoneSpread.par3), 0, 127, ImVec2(40, 40), "Overtone spread par 3"))
    {
    }
}

void zyn::ui::SubNote::SUBNoteEditorFilter(SUBnoteParameters *parameters)
{
    bool filterEnabled = parameters->PGlobalFilterEnabled == 1;
    if (ImGui::Checkbox("Global Filter Parameters", &filterEnabled))
    {
        parameters->PGlobalFilterEnabled = filterEnabled ? 1 : 0;
    }

    if (filterEnabled)
    {
        _FilterParameters.Render(parameters->GlobalFilter);

        ImGui::Separator();

        _FilterEnvelope.Render(parameters->GlobalFilterEnvelope);
    }
}

void zyn::ui::SubNote::SUBNoteEditorFrequency(SUBnoteParameters *parameters)
{
    ImGui::Text("Global Frequency Parameters");

    auto detune = static_cast<int>(parameters->PDetune) - 8192;
    ImGui::PushItemWidth(300);
    if (ImGui::SliderInt("##Detune", &detune, -35, 35, "Detune %d"))
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

    _FrequencyEnvelope.Render(envelopeEnabled ? parameters->FreqEnvelope : nullptr);
}
