#include "syntheditor.h"

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"
#include "imgui_helpers.h"
#include <imgui.h>
#include <imgui_plot.h>
#include <zyn.synth/ADnoteGlobalParam.h>
#include <zyn.synth/ADnoteParams.h>

char const *SynthEditor::ID = "SynthEditor";

unsigned char min = 0, max = 127;

SynthEditor::SynthEditor()
{}

void SynthEditor::SetUp(ApplicationSession *session)
{
    _session = session;

    _libraryDialog.SetUp(session);
}

bool CheckButton(const char *label, bool *p_value, ImVec2 const &size)
{
    bool result = false;
    auto &s = ImGui::GetStyle();

    ImGui::PushStyleColor(ImGuiCol_Button, (*p_value) ? s.Colors[ImGuiCol_ButtonActive] : s.Colors[ImGuiCol_Button]);

    if (ImGui::Button(label, size))
    {
        (*p_value) = !(*p_value);
        result = true;
    }

    ImGui::PopStyleColor();

    return result;
}

void SynthEditor::Render2d()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        ID,
        nullptr,
        flags);
    {
        auto track = _session->_mixer->GetTrack(_session->currentTrack);
        ImGui::BeginChild("btns", ImVec2(121, 0));
        {
            ImGui::Text("Tracks");
            for (unsigned int i = 0; i < _session->_mixer->GetTrackCount(); i++)
            {
                auto t = _session->_mixer->GetTrack(i);
                if (i % 4 != 0)
                {
                    ImGui::SameLine();
                }
                char title[8] = {0};
                sprintf_s(title, 8, "%u", i + 1);
                bool active = (_session->currentTrack == i);
                bool disabled = t->Penabled == 0 && !active;
                if (disabled)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_Separator));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TitleBg));
                }
                if (CheckButton(title, &active, ImVec2(24, 24)))
                {
                    _session->currentTrack = i;
                }
                if (disabled)
                {
                    ImGui::PopStyleColor(2);
                }
            }

            ImGui::Separator();

            bool b;
            bool drumKitChecked = track->Pdrummode == 1;
            if (ImGui::Checkbox("Drummode", &drumKitChecked))
            {
                track->Pdrummode = drumKitChecked ? 1 : 0;
                if (track->Pdrummode == 0)
                {
                    _session->currentTrackInstrument = 0;
                }
            }

            if (drumKitChecked)
            {
                ImGui::Text("Kits");
                for (unsigned int i = 0; i < NUM_TRACK_INSTRUMENTS; i++)
                {
                    if (i % 4 != 0)
                    {
                        ImGui::SameLine();
                    }
                    char title[8] = {0};
                    sprintf_s(title, 8, "%u", i + 1);
                    bool active = (_session->currentTrackInstrument == i);
                    if (CheckButton(title, &active, ImVec2(24, 24)))
                    {
                        _session->currentSynth = ActiveSynths::Add;
                        _session->currentTrackInstrument = i;
                    }
                }
            }

            ImGui::Separator();

            auto &instrument = track->Instruments[_session->currentTrackInstrument];
            bool addChecked = instrument.Padenabled;
            if (ImGui::Checkbox("##ADD_CHECK", &addChecked))
            {
                instrument.Padenabled = addChecked ? 1 : 0;
                _session->currentSynth = ActiveSynths::Add;
            }
            ImGui::SameLine();

            bool voiceActive = (_session->currentSynth == ActiveSynths::Add && _session->currentVoice >= NUM_VOICES);
            if (CheckButton("Add", &voiceActive, ImVec2(-1, 0)))
            {
                _session->currentSynth = ActiveSynths::Add;
                _session->currentVoice = NUM_VOICES;
            }

            ImGui::PushID("VOICES");
            for (unsigned int i = 0; i < NUM_VOICES; i++)
            {
                if (i % 4 != 0)
                {
                    ImGui::SameLine();
                }
                char title[8] = {0};
                sprintf_s(title, 8, "%u", i + 1);
                voiceActive = (_session->currentVoice == i);
                auto voiceEnabled = instrument.adpars->VoicePar[i].Enabled && addChecked;
                if (!voiceEnabled)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetColorU32(ImGuiCol_Separator));
                    ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetColorU32(ImGuiCol_TitleBg));
                }
                if (CheckButton(title, &voiceActive, ImVec2(24, 24)))
                {
                    _session->currentSynth = ActiveSynths::Add;
                    _session->currentVoice = i;
                }
                if (!voiceEnabled)
                {
                    ImGui::PopStyleColor(2);
                }
            }
            ImGui::PopID();

            ImGui::Separator();

            bool subChecked = instrument.Psubenabled == 1;
            if (ImGui::Checkbox("##SUB_CHECK", &subChecked))
            {
                instrument.Psubenabled = subChecked ? 1 : 0;
                _session->currentSynth = ActiveSynths::Sub;
            }
            ImGui::SameLine();
            b = (_session->currentSynth == ActiveSynths::Sub);
            if (CheckButton("Sub", &b, ImVec2(-1, 0)))
            {
                _session->currentSynth = ActiveSynths::Sub;
            }

            ImGui::Separator();

            bool padChecked = instrument.Ppadenabled;
            if (ImGui::Checkbox("##PAD_CHECK", &padChecked))
            {
                instrument.Ppadenabled = padChecked ? 1 : 0;
                _session->currentSynth = ActiveSynths::Pad;
            }
            ImGui::SameLine();
            b = (_session->currentSynth == ActiveSynths::Pad);
            if (CheckButton("Pad", &b, ImVec2(-1, 0)))
            {
                _session->currentSynth = ActiveSynths::Pad;
            }

            ImGui::Separator();

            bool smplChecked = instrument.Psmplenabled;
            if (ImGui::Checkbox("##SMPL_CHECK", &smplChecked))
            {
                instrument.Psmplenabled = smplChecked ? 1 : 0;
                _session->currentSynth = ActiveSynths::Smpl;
            }
            ImGui::SameLine();
            b = (_session->currentSynth == ActiveSynths::Smpl);
            if (CheckButton("Smpl", &b, ImVec2(-1, 0)))
            {
                _session->currentSynth = ActiveSynths::Smpl;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (_session->currentSynth == ActiveSynths::Add)
        {
            ImGui::BeginChild("add-synth");
            {
                if (track->Instruments[_session->currentTrackInstrument].Padenabled == 0)
                {
                    ImGui::Text("ADD editor is disabled");
                    if (ImGui::Button("Enable ADD synth"))
                    {
                        track->Instruments[_session->currentTrackInstrument].Padenabled = 1;
                    }
                }
                else
                {
                    ADnoteParameters *params = track->Instruments[_session->currentTrackInstrument].adpars;

                    RenderAddSynth(params);
                }
            }
            ImGui::EndChild();
        }
        else if (_session->currentSynth == ActiveSynths::Pad)
        {
            ImGui::BeginChild("pad-synth");
            {
                if (track->Instruments[_session->currentTrackInstrument].Ppadenabled == 0)
                {
                    ImGui::Text("PAD editor is disabled");
                    if (ImGui::Button("Enable PAD synth"))
                    {
                        track->Instruments[_session->currentTrackInstrument].Ppadenabled = 1;
                    }
                }
                else
                {
                    PADnoteParameters *params = track->Instruments[_session->currentTrackInstrument].padpars;

                    RenderPadSynth(params);
                }
            }
            ImGui::EndChild();
        }
        else if (_session->currentSynth == ActiveSynths::Sub)
        {
            ImGui::BeginChild("sub-synth");
            {
                if (track->Instruments[_session->currentTrackInstrument].Psubenabled == 0)
                {
                    ImGui::Text("SUB editor is disabled");
                    if (ImGui::Button("Enable SUB synth"))
                    {
                        track->Instruments[_session->currentTrackInstrument].Psubenabled = 1;
                    }
                }
                else
                {
                    SUBnoteParameters *params = track->Instruments[_session->currentTrackInstrument].subpars;

                    RenderSubSynth(params);
                }
            }
            ImGui::EndChild();
        }
        else if (_session->currentSynth == ActiveSynths::Smpl)
        {
            if (ImGui::BeginChild("SmplSynthEditorID", ImVec2(0, 0), false))
            {
                if (track->Instruments[_session->currentTrackInstrument].Psmplenabled == 0)
                {
                    ImGui::Text("SMPL editor is disabled");
                    if (ImGui::Button("Enable SMPL synth"))
                    {
                        track->Instruments[_session->currentTrackInstrument].Psmplenabled = 1;
                    }
                }
                else
                {
                    SampleNoteParameters *params = track->Instruments[_session->currentTrackInstrument].smplpars;

                    RenderSmplSynth(params);
                }
            }
            ImGui::EndChild();
        }
    }

    ImGui::End();
}

static const int modulation_type_count = 5;
static char const *modulation_types[] = {
    "OFF",
    "MORPH",
    "RING",
    "PM",
    "FM",
};

static const int oscillator_type_count = 2;
static char const *oscillator_types[] = {
    "##Sound",
    "##Noise",
};

static char const *oscillator_type_tooltips[] = {
    "Sound",
    "Whitenoise",
};

static char const *oscillator_type_icons[] = {
    ICON_FAD_MODSINE,
    ICON_FAD_MODRANDOM,
};

void SynthEditor::RenderAbstractSynth(AbstractNoteParameters *params)
{
    if (KnobUchar("Vol", &(params->PVolume), 0, 127, ImVec2(50, 40), "Volume"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("Pan", &params->PPanning, 0, 127, ImVec2(50, 40), "Panning (leftmost is random)"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("V.Sns", &(params->PAmpVelocityScaleFunction), 0, 127, ImVec2(50, 40), "Velocity Sensing Function (rightmost to disable)"))
    {
    }

    ImGui::SameLine();

    auto stereo = params->PStereo == 1;
    if (ImGui::Checkbox("Stereo", &stereo))
    {
        params->PStereo = stereo ? 1 : 0;
    }
    ShowTooltipOnHover("Stereo");
}

void SynthEditor::RenderAddSynth(
    ADnoteParameters *addparams)
{
    if (_session->currentVoice >= NUM_VOICES)
    {
        RenderAbstractSynth(addparams);

        if (ImGui::CollapsingHeader("Amplitude"))
        {
            ImGui::PushID("GLOBAL AMPLITUDDE");

            if (KnobUchar("P.Str.", &addparams->PPunchStrength, 0, 127, ImVec2(50, 40), "Punch Strength"))
            {
            }

            ImGui::SameLine();

            if (KnobUchar("P.t.", &addparams->PPunchTime, 0, 127, ImVec2(50, 40), "Punch time (duration)"))
            {
            }

            ImGui::SameLine();

            if (KnobUchar("P.Stc.", &addparams->PPunchStretch, 0, 127, ImVec2(50, 40), "Punch Stretch"))
            {
            }

            ImGui::SameLine();

            if (KnobUchar("P.Vel.", &addparams->PPunchVelocitySensing, 0, 127, ImVec2(50, 40), "Punch Velocity Sensing"))
            {
            }

            ImGui::SameLine();

            auto randomGrouping = addparams->Hrandgrouping == 1;
            if (ImGui::Checkbox("Rnd Grp", &randomGrouping))
            {
                addparams->Hrandgrouping = randomGrouping ? 1 : 0;
            }
            ShowTooltipOnHover("How the Harmonic Amplitude is applied to voices that use the same oscillator");

            ImGui::Separator();

            RenderEnvelope("Amplitude Envelope", addparams->AmpEnvelope, nullptr);

            ImGui::Separator();

            RenderLfo("Amplitude LFO", addparams->AmpLfo, nullptr);

            ImGui::PopID();
        }
        if (ImGui::CollapsingHeader("Filter"))
        {
            ImGui::PushID("GLOBAL FILTER");

            RenderFilter(addparams->GlobalFilter);

            ImGui::Separator();

            RenderEnvelope("Filter Envelope", addparams->FilterEnvelope, nullptr);

            ImGui::Separator();

            RenderLfo("Filter LFO", addparams->FilterLfo, nullptr);

            ImGui::PopID();
        }
        if (ImGui::CollapsingHeader("Frequency"))
        {
            ImGui::PushID("GLOBAL FREQUENCY");

            RenderDetune(addparams->PDetuneType, addparams->PCoarseDetune, addparams->PDetune);

            ImGui::Separator();

            RenderEnvelope("Frequency Envelope", addparams->FreqEnvelope, nullptr);

            ImGui::Separator();

            RenderLfo("Frequency LFO", addparams->FreqLfo, nullptr);

            ImGui::Separator();

            ImGui::PopID();
        }
    }
    else
    {
        auto voiceparams = &addparams->VoicePar[_session->currentVoice];

        auto enabled = voiceparams->Enabled != 0;
        if (ImGui::Checkbox("Enabled Voice", &enabled))
        {
            voiceparams->Enabled = enabled ? 1 : 0;
        }
        ShowTooltipOnHover("Enable this voice");

        std::vector<float> smps(SystemSettings::Instance().oscilsize);
        voiceparams->OscilSmp->get(smps.data(), -1.0);
        ImGui::PlotLines("##oscillator", smps.data(), static_cast<int>(SystemSettings::Instance().oscilsize), 0, nullptr, -1.1f, 1.1f, ImVec2(90, 60));

        ImGui::SameLine();

        if (KnobUchar("Vol", &(voiceparams->PVolume), 0, 127, ImVec2(50, 40), "Volume"))
        {
        }

        ImGui::SameLine();

        auto volumeminus = voiceparams->PVolumeminus == 1;
        if (ImGui::Checkbox("Minus", &volumeminus))
        {
            voiceparams->PVolumeminus = volumeminus ? 1 : 0;
        }

        ImGui::SameLine();

        if (KnobUchar("Pan", &voiceparams->PPanning, 0, 127, ImVec2(50, 40), "Panning (leftmost is random)"))
        {
        }

        ImGui::SameLine();

        if (KnobUchar("V.Sns", &(voiceparams->PAmpVelocityScaleFunction), 0, 127, ImVec2(50, 40), "Velocity Sensing Function (rightmost to disable)"))
        {
        }

        if (ImGui::CollapsingHeader("Amplitude"))
        {
            ImGui::PushID("Voice Amplitude");

            RenderEnvelope("Amplitude Envelope", voiceparams->AmpEnvelope, &(voiceparams->PAmpEnvelopeEnabled));

            ImGui::Separator();

            RenderLfo("Amplitude LFO", voiceparams->AmpLfo, &(voiceparams->PAmpLfoEnabled));

            ImGui::PopID();
        }
        if (ImGui::CollapsingHeader("Filter"))
        {
            ImGui::PushID("VOICE FILTER");

            auto filterbypass = voiceparams->Pfilterbypass == 1;
            if (ImGui::Checkbox("Bypass Global F.", &filterbypass))
            {
                voiceparams->Pfilterbypass = filterbypass ? 1 : 0;
            }

            ImGui::Separator();

            RenderFilter(voiceparams->VoiceFilter);

            ImGui::Separator();

            RenderEnvelope("Filter Envelope", voiceparams->FilterEnvelope, &(voiceparams->PFilterEnvelopeEnabled));

            ImGui::Separator();

            RenderLfo("Filter LFO", voiceparams->FilterLfo, &(voiceparams->PFilterLfoEnabled));

            ImGui::PopID();
        }
        if (ImGui::CollapsingHeader("Frequency"))
        {
            ImGui::PushID("VOICE FREQUENCY");

            RenderDetune(voiceparams->PDetuneType, voiceparams->PCoarseDetune, voiceparams->PDetune);

            ImGui::Separator();

            auto freq440hz = voiceparams->Pfixedfreq == 1;
            if (ImGui::Checkbox("440Hz", &freq440hz))
            {
                voiceparams->Pfixedfreq = freq440hz ? 1 : 0;
            }
            ShowTooltipOnHover("Set the voice base frequency to 440Hz");

            if (freq440hz)
            {
                ImGui::SameLine();

                auto fixedfreqET = static_cast<int>(voiceparams->PfixedfreqET);
                ImGui::PushItemWidth(100);
                if (ImGui::SliderInt("##EqT", &fixedfreqET, 0, 127, "Eq.T. %d"))
                {
                    voiceparams->PfixedfreqET = static_cast<unsigned short int>(fixedfreqET);
                }
                ShowTooltipOnHover("How the frequency varies acording to the keyboard (leftmost for fixed frequency)");
            }

            ImGui::Separator();

            RenderEnvelope("Frequency Envelope", voiceparams->FreqEnvelope, &(voiceparams->PFreqEnvelopeEnabled));

            ImGui::Separator();

            RenderLfo("Frequency LFO", voiceparams->FreqLfo, &(voiceparams->PFreqLfoEnabled));

            ImGui::PopID();
        }
        if (ImGui::CollapsingHeader("Oscillator"))
        {
            ImGui::PushID("VOICE OSCILLATOR");

            ImGui::BeginGroup();
            for (int i = 0; i < oscillator_type_count; i++)
            {
                int filter_type = voiceparams->Type;
                if (ImGui::RadioButton(oscillator_types[i], &(filter_type), i))
                {
                    voiceparams->Type = i;
                }
                ShowTooltipOnHover(oscillator_type_tooltips[i]);

                ImGui::SameLine();

                ImGui::Text("%s", oscillator_type_icons[i]);
                ShowTooltipOnHover(oscillator_type_tooltips[i]);

                ImGui::SameLine();
            }
            ImGui::EndGroup();

            if (voiceparams->Type == 0) // Sound
            {
                ImGui::SameLine();

                ImGui::Button("Change");

                ImGui::SameLine();

                if (KnobUchar("Phase", &(voiceparams->PFMoscilphase), 0, 127, ImVec2(50, 40), "Velocity Sensing Function"))
                {
                }

                ImGui::PlotLines("##oscillator", smps.data(), static_cast<int>(SystemSettings::Instance().oscilsize), 0, nullptr, -1.1f, 1.1f, ImVec2(400, 200));
            }

            ImGui::PopID();
        }
        if (ImGui::CollapsingHeader("Modulator"))
        {
            ImGui::PushID("VOICE MODULATOR AMPLITUDE");

            static char const *current_modulation_type_item = nullptr;

            auto modulation_type = static_cast<int>(voiceparams->PFMEnabled);
            current_modulation_type_item = modulation_types[modulation_type];
            ImGui::PushItemWidth(300);
            if (ImGui::BeginCombo("Modulation type", current_modulation_type_item))
            {
                for (int n = 0; n < modulation_type_count; n++)
                {
                    bool is_selected = (current_modulation_type_item == modulation_types[n]);
                    if (ImGui::Selectable(modulation_types[n], is_selected))
                    {
                        current_modulation_type_item = modulation_types[n];
                        voiceparams->PFMEnabled = static_cast<unsigned char>(n);
                    }
                }

                ImGui::EndCombo();
            }
            ShowTooltipOnHover("Modulation type");

            if (voiceparams->PFMEnabled)
            {
                if (KnobUchar("Vol", &(voiceparams->PFMVolume), 0, 127, ImVec2(50, 40), "Volume"))
                {
                }

                ImGui::SameLine();

                if (KnobUchar("V.Sns", &(voiceparams->PFMVelocityScaleFunction), 0, 127, ImVec2(50, 40), "Velocity Sensing Function"))
                {
                }

                ImGui::SameLine();

                if (KnobUchar("F.Damp", &(voiceparams->PFMVolumeDamp), 0, 127, ImVec2(50, 40), "Modulator Damp at Higher frequency"))
                {
                }

                ImGui::Separator();

                RenderDetune(voiceparams->PFMDetuneType, voiceparams->PFMCoarseDetune, voiceparams->PFMDetune);

                ImGui::Separator();

                RenderEnvelope("Modulator Amplitude Envelope", voiceparams->FMAmpEnvelope, &(voiceparams->PFMAmpEnvelopeEnabled));

                ImGui::PopID();

                ImGui::Separator();

                ImGui::PushID("VOICE MODULATOR FREQUENCY");

                RenderEnvelope("Modulator Frequency Envelope", voiceparams->FMFreqEnvelope, &(voiceparams->PFMFreqEnvelopeEnabled));

                ImGui::Separator();

                ImGui::Text("Modulator Oscillator");

                ImGui::Button("Change");

                ImGui::SameLine();

                if (KnobUchar("Phase", &(voiceparams->PFMoscilphase), 0, 127, ImVec2(50, 40), "Velocity Sensing Function"))
                {
                }

                std::vector<float> fmsmps(SystemSettings::Instance().oscilsize);
                voiceparams->FMSmp->get(fmsmps.data(), -1.0);
                ImGui::PlotLines("##oscillator", smps.data(), static_cast<int>(SystemSettings::Instance().oscilsize), 0, nullptr, -1.1f, 1.1f, ImVec2(400, 200));

                ImGui::Separator();
            }

            ImGui::PopID();
        }
    }
}

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

static unsigned int mag_type_count = 5;
static char const *const mag_types[] = {
    "Linear",
    "-40dB",
    "-60dB",
    "-80dB",
    "-100dB",
};

static unsigned int start_type_count = 3;
static char const *const start_types[] = {
    "Zero",
    "RND",
    "Max.",
};

void SynthEditor::RenderSubSynth(
    SUBnoteParameters *params)
{
    RenderAbstractSynth(params);

    ImGui::PushItemWidth(100);
    if (DropDown("##MagType", params->Phmagtype, mag_types, mag_type_count, "Mag type"))
    {
    }
    ShowTooltipOnHover("How the magnitudes are computed (0=linear,1=-60dB,2=-60dB)");

    ImGui::SameLine();

    ImGui::PushItemWidth(100);
    if (DropDown("##Start", params->Pstart, start_types, start_type_count, "Start"))
    {
    }
    ShowTooltipOnHover("How the harmonics start(\"0\"=0,\"1\"=random,\"2\"=1)");

    if (ImGui::CollapsingHeader("Amplitude"))
    {
        RenderEnvelope("Amplitude Envelope", params->AmpEnvelope, nullptr);
    }

    if (ImGui::CollapsingHeader("Filter"))
    {
        ImGui::PushID("GLOBAL FILTER");

        auto filterStages = static_cast<int>(params->Pnumstages);
        ImGui::PushItemWidth(100);
        if (ImGui::SliderInt("Filter Stages", &filterStages, 1, 5))
        {
            params->Pnumstages = static_cast<unsigned char>(filterStages);
        }
        ShowTooltipOnHover("How many times the noise is filtered");

        bool filterEnabled = params->PGlobalFilterEnabled == 1;
        if (ImGui::Checkbox("Enable Filter", &filterEnabled))
        {
            params->PGlobalFilterEnabled = filterEnabled ? 1 : 0;
        }

        if (filterEnabled)
        {
            ImGui::Separator();

            RenderFilter(params->GlobalFilter);

            ImGui::Separator();

            RenderEnvelope("Filter Envelope", params->GlobalFilterEnvelope, nullptr);
        }
        ImGui::PopID();
    }

    if (ImGui::CollapsingHeader("Frequency"))
    {
        RenderDetune(params->PDetuneType, params->PCoarseDetune, params->PDetune);

        ImGui::Separator();

        auto freq440hz = params->Pfixedfreq == 1;
        if (ImGui::Checkbox("440Hz", &freq440hz))
        {
            params->Pfixedfreq = freq440hz ? 1 : 0;
        }
        ShowTooltipOnHover("Set the voice base frequency to 440Hz");

        if (freq440hz)
        {
            ImGui::SameLine();

            auto fixedfreqET = static_cast<int>(params->PfixedfreqET);
            ImGui::PushItemWidth(100);
            if (ImGui::SliderInt("##EqT", &fixedfreqET, 0, 127, "Eq.T. %d"))
            {
                params->PfixedfreqET = static_cast<unsigned short int>(fixedfreqET);
            }
            ShowTooltipOnHover("How the frequency varies acording to the keyboard (leftmost for fixed frequency)");
        }

        ImGui::Separator();

        RenderEnvelope("Frequency Envelope", params->FreqEnvelope, &(params->PFreqEnvelopeEnabled));
    }

    if (ImGui::CollapsingHeader("Bandwidth"))
    {
        auto bandwidth = static_cast<int>(params->Pbandwidth);
        ImGui::PushItemWidth(250);
        if (ImGui::SliderInt("##Bandwidth", &bandwidth, 0, 127, "Bandwidth %d"))
        {
            params->Pbandwidth = static_cast<unsigned char>(bandwidth);
        }
        ShowTooltipOnHover("Bandwidth");

        auto bandwidthScale = static_cast<int>(params->Pbwscale);
        ImGui::PushItemWidth(250);
        if (ImGui::SliderInt("##BandwidthScale", &bandwidthScale, 0, 127, "Scale %d"))
        {
            params->Pbwscale = static_cast<unsigned char>(bandwidthScale);
        }
        ShowTooltipOnHover("Bandwidth Scale");

        ImGui::Separator();

        RenderEnvelope("Bandwidth Envelope", params->BandWidthEnvelope, &(params->PFreqEnvelopeEnabled));
    }

    if (ImGui::CollapsingHeader("Overtones"))
    {
        ImGui::Text("Overtone Parameters");

        ImGui::PushItemWidth(100);
        if (DropDown("Overtone positions", params->POvertoneSpread.type, overtone_positions, overtone_position_count, "Overtone positions"))
        {
        }

        if (KnobUchar("Par1", &(params->POvertoneSpread.par1), 0, 127, ImVec2(50, 40), "Overtone spread par 1"))
        {
        }

        ImGui::SameLine();

        if (KnobUchar("Par2", &(params->POvertoneSpread.par2), 0, 127, ImVec2(50, 40), "Overtone spread par 2"))
        {
        }

        ImGui::SameLine();

        if (KnobUchar("ForceH", &(params->POvertoneSpread.par3), 0, 127, ImVec2(50, 40), "Overtone spread par 3"))
        {
        }
    }

    if (ImGui::CollapsingHeader("Harmonics"))
    {
        ImGui::BeginChild("Harmonics", ImVec2(), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar);
        for (int i = 0; i < MAX_SUB_HARMONICS; i++)
        {
            ImGui::PushID(i);
            if (i != 0)
            {
                ImGui::SameLine();
            }
            int v = static_cast<int>(params->Phmag[i]);
            if (ImGui::VSliderInt("##harmonic", ImVec2(10, 100), &v, 0, 127, ""))
            {
                params->Phmag[i] = static_cast<unsigned char>(v);
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
            int v = static_cast<int>(params->Phrelbw[i]);
            if (ImGui::VSliderInt("##subharmonic", ImVec2(10, 100), &v, 0, 127, ""))
            {
                params->Phrelbw[i] = static_cast<unsigned char>(v);
            }
            ImGui::PopID();
        }
        ImGui::EndChild();
    }
}

void SynthEditor::RenderPadSynth(
    PADnoteParameters *params)
{
    RenderAbstractSynth(params);

    if (ImGui::CollapsingHeader("Amplitude"))
    {
        if (KnobUchar("P.Str.", &params->PPunchStrength, 0, 127, ImVec2(50, 40), "Punch Strength"))
        {
        }

        ImGui::SameLine();

        if (KnobUchar("P.t.", &params->PPunchTime, 0, 127, ImVec2(50, 40), "Punch time (duration)"))
        {
        }

        ImGui::SameLine();

        if (KnobUchar("P.Stc.", &params->PPunchStretch, 0, 127, ImVec2(50, 40), "Punch Stretch"))
        {
        }

        ImGui::SameLine();

        if (KnobUchar("P.Vel.", &params->PPunchVelocitySensing, 0, 127, ImVec2(50, 40), "Punch Velocity Sensing"))
        {
        }

        ImGui::Separator();

        RenderEnvelope("Amplitude Envelope", params->AmpEnvelope, nullptr);

        ImGui::Separator();

        RenderLfo("Amplitude LFO", params->AmpLfo, nullptr);
    }

    if (ImGui::CollapsingHeader("Filter"))
    {
        ImGui::PushID("GLOBAL FILTER");

        RenderFilter(params->GlobalFilter);

        ImGui::Separator();

        RenderEnvelope("Filter Envelope", params->FilterEnvelope, nullptr);

        ImGui::Separator();

        RenderLfo("Filter LFO", params->FilterLfo, nullptr);

        ImGui::PopID();
    }
    if (ImGui::CollapsingHeader("Frequency"))
    {
        ImGui::PushID("GLOBAL FREQUENCY");

        RenderDetune(params->PDetuneType, params->PCoarseDetune, params->PDetune);

        ImGui::Separator();

        auto freq440hz = params->Pfixedfreq == 1;
        if (ImGui::Checkbox("440Hz", &freq440hz))
        {
            params->Pfixedfreq = freq440hz ? 1 : 0;
        }
        ShowTooltipOnHover("Set the voice base frequency to 440Hz");

        if (freq440hz)
        {
            ImGui::SameLine();

            auto fixedfreqET = static_cast<int>(params->PfixedfreqET);
            ImGui::PushItemWidth(100);
            if (ImGui::SliderInt("##EqT", &fixedfreqET, 0, 127, "Eq.T. %d"))
            {
                params->PfixedfreqET = static_cast<unsigned short int>(fixedfreqET);
            }
            ShowTooltipOnHover("How the frequency varies acording to the keyboard (leftmost for fixed frequency)");
        }

        ImGui::Separator();

        RenderEnvelope("Frequency Envelope", params->FreqEnvelope, nullptr);

        ImGui::Separator();

        RenderLfo("Frequency LFO", params->FreqLfo, nullptr);

        ImGui::Separator();

        ImGui::PopID();
    }
}

std::string NoteToString(unsigned char note)
{
    switch (note)
    {
        case 35:
            return "Acoustic Bass Drum [35]";
        case 36:
            return "Bass Drum 1 [36]";
        case 37:
            return "Side Stick [37]";
        case 38:
            return "Acoustic Snare [38]";
        case 39:
            return "Hand Clap [39]";
        case 40:
            return "Electric Snare [40]";
        case 41:
            return "Low Floor Tom [41]";
        case 42:
            return "Closed Hi-Hat [42]";
        case 43:
            return "High Floor Tom [43]";
        case 44:
            return "Pedal Hi-Hat [44]";
        case 45:
            return "Low Tom [45]";
        case 46:
            return "Open Hi-Hat [46]";
        case 47:
            return "Low-Mid Tom [47]";
        case 48:
            return "Hi-Mid Tom [48]";
        case 49:
            return "Crash Cymbal 1 [49]";
        case 50:
            return "High Tom [50]";
        case 51:
            return "Ride Cymbal 1 [51]";
        case 52:
            return "Chinese Cymbal [52]";
        case 53:
            return "Ride Bell [53]";
        case 54:
            return "Tambourine [54]";
        case 55:
            return "Splash Cymbal [55]";
        case 56:
            return "Cowbell [56]";
        case 57:
            return "Crash Cymbal 2 [57]";
        case 58:
            return "Vibraslap [58]";
        case 59:
            return "Ride Cymbal 2 [59]";
        case 60:
            return "Hi Bongo [60]";
        case 61:
            return "Low Bongo [61]";
        case 62:
            return "Mute Hi Conga [62]";
        case 63:
            return "Open Hi Conga [63]";
        case 64:
            return "Low Conga [64]";
        case 65:
            return "High Timbale [65]";
        case 66:
            return "Low Timbale [66]";
        case 67:
            return "High Agogo [67]";
        case 68:
            return "Low Agogo [68]";
        case 69:
            return "Cabasa [69]";
        case 70:
            return "Maracas [70]";
        case 71:
            return "Short Whistle [71]";
        case 72:
            return "Long Whistle [72]";
        case 73:
            return "Short Guiro [73]";
        case 74:
            return "Long Guiro [74]";
        case 75:
            return "Claves [57]";
        case 76:
            return "Hi Wood Block [76]";
        case 77:
            return "Low Wood Block [77]";
        case 78:
            return "Mute Cuica [78]";
        case 79:
            return "Open Cuica [79]";
        case 80:
            return "Mute Triangle [80]";
        case 81:
            return "Open Triangle [81]";
    }

    return "drum";
}

void SynthEditor::RenderSmplSynth(
    SampleNoteParameters *params)
{
    RenderAbstractSynth(params);

    bool selectSample = false;
    static unsigned char selectingSampleForKey = 0;
    static int selectedNote = SAMPLE_NOTE_MIN;

    ImGui::BeginChild("samples", ImVec2(0, -200));
    {
        if (ImGui::BeginTabBar("samplestabs"))
        {
            if (ImGui::BeginTabItem("midi-notes"))
            {
                ImGui::Columns(2);
                ImGui::SetColumnWidth(0, 200);
                for (unsigned char i = SAMPLE_NOTE_MIN; i < SAMPLE_NOTE_MAX; i++)
                {
                    ImGui::PushID(i);

                    char label[32];
                    sprintf(label, "%s", NoteToString(i).c_str());
                    if (ImGui::Selectable(label, selectedNote == i, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        selectedNote = i;
                    }

                    ImGui::NextColumn();
                    if (params->PwavData.find(i) != params->PwavData.end())
                    {
                        ImGui::Text("%s", params->PwavData[i]->name.c_str());
                    }
                    else
                    {
                        ImGui::Text("<unused>");
                    }
                    ImGui::NextColumn();
                    ImGui::PopID();
                }
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("qwerty-keyboard"))
            {
                ImGui::BeginChild("realkeyboard", ImVec2(0, 0));
                {
                }
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
    }
    ImGui::EndChild();

    ImGui::BeginChild("sample", ImVec2(0, 190));
    {
        if (ImGui::Button("Change"))
        {
            selectSample = true;
            selectingSampleForKey = selectedNote;
        }
        auto selectedWav = params->PwavData.find(selectedNote);
        if (selectedWav != params->PwavData.end())
        {
            ImGui::SameLine();
            if (ImGui::Button("Clear"))
            {
                params->PwavData.erase(selectedNote);
            }

            ImGui::SameLine();
            if (ImGui::Button("Play"))
            {
                _session->_mixer->PreviewSample((*selectedWav).second->path);
            }

            ImGui::PlotConfig conf;
            conf.values.ys = (*selectedWav).second->PwavData;
            conf.values.count = (*selectedWav).second->samplesPerChannel * (*selectedWav).second->channels;
            conf.scale.min = -1;
            conf.scale.max = 1;
            conf.tooltip.show = true;
            conf.tooltip.format = "x=%.2f, y=%.2f";
            conf.grid_x.show = false;
            conf.grid_y.show = false;
            conf.frame_size = ImVec2(ImGui::GetContentRegionAvailWidth(), 160);
            conf.line_thickness = 2.f;

            ImGui::Plot("plot", conf);
        }
    }
    ImGui::EndChild();

    _libraryDialog.ShowDialog(selectSample, [params](ILibraryItem *sample) {
        if (sample != nullptr)
        {
            params->PwavData[selectingSampleForKey] = WavData::Load(sample->GetPath());
        }
    });
}

static int lfo_type_count = 7;
static char const *lfo_types[] = {
    "SINE",
    "TRI",
    "SQR",
    "R.up",
    "R.dn",
    "E1dn",
    "E2dn",
};

static int detune_types_count = 4;
static char const *detune_types[] = {
    "L35cents",
    "L10cents",
    "E100cents",
    "E1200cents",
};

void SynthEditor::RenderLfo(
    char const *label,
    LFOParams *params,
    unsigned char *enabled)
{
    if (params == nullptr)
    {
        return;
    }

    bool lfoEnabled = enabled == nullptr || (*enabled) == 1;

    if (enabled != nullptr)
    {
        if (ImGui::Checkbox(label, &lfoEnabled))
        {
            (*enabled) = lfoEnabled ? 1 : 0;
        }
        ShowTooltipOnHover("Enable this LFO");
    }
    else
    {
        ImGui::Text("%s", label);
    }

    if (!lfoEnabled)
    {
        return;
    }

    ImGui::PushItemWidth(100);
    if (DropDown("##lfotype", params->PLFOtype, lfo_types, lfo_type_count, "LFO Type"))
    {
    }

    if (Knob("Freq.", &(params->Pfreq), 0.0f, 1.0f, ImVec2(50, 40), "LFO frequency"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("Depth", &(params->Pintensity), 0, 127, ImVec2(50, 40), "LFO amount"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("Start", &(params->Pstartphase), 0, 127, ImVec2(50, 40), "LFO startphase (leftmost is random)"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("Delay", &(params->Pdelay), 0, 127, ImVec2(50, 40), "LFO delay"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("Str.", &(params->Pstretch), 0, 127, ImVec2(50, 40), "LFO stretch"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("A.R", &(params->Prandomness), 0, 127, ImVec2(30, 30), "LFO Amplitude Randomness"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("F.R", &(params->Pfreqrand), 0, 127, ImVec2(30, 30), "LFO Frequency Randomness"))
    {
    }
}

int getpointx(
    EnvelopeParams *env,
    int n,
    int w)
{
    int lx = w - 20;
    int npoints = env->Penvpoints;

    float sum = 0;
    for (int i = 1; i < npoints; i++)
        sum += env->getdt(i) + 1;

    float sumbefore = 0; //the sum of all points before the computed point
    for (int i = 1; i <= n; i++)
        sumbefore += env->getdt(i) + 1;

    return ((int)(sumbefore / (float)sum * lx));
}

int getpointy(
    EnvelopeParams *env,
    int n,
    int h)
{
    int ly = h - 20;

    return ((int)((1.0 - env->Penvval[n] / 127.0) * ly));
}

void SynthEditor::RenderEnvelope(
    char const *label,
    EnvelopeParams *params,
    unsigned char *enabled)
{
    if (params == nullptr)
    {
        return;
    }

    auto plotSize = ImVec2(200, 80);

    bool envelopeEnabled = enabled == nullptr || (*enabled) == 1;

    if (enabled != nullptr)
    {
        if (ImGui::Checkbox(label, &envelopeEnabled))
        {
            (*enabled) = envelopeEnabled ? 1 : 0;
        }
        ShowTooltipOnHover("Enable this Envelope");
    }
    else
    {
        ImGui::Text("%s", label);
    }

    if (!envelopeEnabled)
    {
        return;
    }

    ImGui::BeginChild("EnvelopePlot", plotSize);
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        static ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
        const ImU32 col = ImColor(colf);
        static ImVec4 bgf = ImVec4(0.4f, 0.5f, 0.8f, 0.3f);
        const ImU32 bg = ImColor(bgf);
        auto th = 1.0f;
        const ImVec2 p = ImGui::GetCursorScreenPos();

        float x = p.x + 10, y = p.y + 10;

        draw_list->AddRectFilled(p, p + plotSize, bg);
        EnvelopeParams::ConvertToFree(params);

        ImVec2 start = {
            x + getpointx(params, 0, plotSize.x),
            y + getpointy(params, 0, plotSize.y),
        };
        for (int i = 1; i < params->Penvpoints; i++)
        {
            auto xx = getpointx(params, i, plotSize.x);
            auto yy = getpointy(params, i, plotSize.y);
            ImVec2 end = {
                x + xx,
                y + yy,
            };

            draw_list->AddLine(start, end, col, th);
            start = end;
        }
    }
    ImGui::EndChild();

    if (params->Envmode >= 3)
    {
        if (KnobUchar("A.val", &(params->PA_val), 0, 127, ImVec2(50, 40), "Starting value"))
        {
        }

        ImGui::SameLine();
    }

    if (KnobUchar("A.dt", &(params->PA_dt), 0, 127, ImVec2(50, 40), "Attack time"))
    {
    }

    ImGui::SameLine();

    if (params->Envmode == 4)
    {
        if (KnobUchar("D.val", &(params->PD_val), 0, 127, ImVec2(50, 40), "Decay value"))
        {
        }

        ImGui::SameLine();
    }

    if (params->Envmode != 3 && params->Envmode != 5)
    {
        if (KnobUchar("D.dt", &(params->PD_dt), 0, 127, ImVec2(50, 40), "Decay time"))
        {
        }

        ImGui::SameLine();
    }

    if (params->Envmode < 3)
    {
        if (KnobUchar("S.val", &(params->PS_val), 0, 127, ImVec2(50, 40), "Sustain value"))
        {
        }

        ImGui::SameLine();
    }

    if (KnobUchar("R.dt", &(params->PR_dt), 0, 127, ImVec2(50, 40), "Release time"))
    {
    }

    ImGui::SameLine();

    if (params->Envmode >= 3)
    {
        if (KnobUchar("R.val", &(params->PR_val), 0, 127, ImVec2(50, 40), "Release value"))
        {
        }

        ImGui::SameLine();
    }

    if (KnobUchar("Str.", &(params->Penvstretch), 0, 127, ImVec2(50, 40), "Envelope stretch (on lower notes makes the envelope longer)"))
    {
    }

    ImGui::SameLine();

    auto forcedRelease = params->Pforcedrelease != 0;
    if (ImGui::Checkbox("frcR.", &forcedRelease))
    {
        params->Pforcedrelease = forcedRelease ? 1 : 0;
    }
    ShowTooltipOnHover("Forced Release");
}

static const int category_count = 3;
static char const *categories[] = {
    "Analog",
    "Formant",
    "StVarF",
};

static const int analog_filter_type_count = 9;
static char const *analog_filter_types[] = {
    "##LPF1",
    "##HPF1",
    "##LPF2",
    "##HPF2",
    "##BPF2",
    "##NF2",
    "##PkF2",
    "##LSh2",
    "##HSh2",
};
static char const *analog_filter_type_tooltips[] = {
    "1 pole Lowpass Filter",
    "1 pole Highpass Filter",
    "2 pole Lowpass Filter",
    "2 pole Highpass Filter",
    "2 pole Bandpass filter",
    "2 pole Bandstop Filter",
    "2 pole Peak Filter",
    "2 pole Lowshelf Filter",
    "2 pole Highshelf Filter",
};
static char const *analog_filter_type_icons[] = {
    ICON_FAD_FILTER_LOWPASS,
    ICON_FAD_FILTER_HIGHPASS,
    ICON_FAD_FILTER_REZ_LOWPASS,
    ICON_FAD_FILTER_REZ_HIGHPASS,
    ICON_FAD_FILTER_BANDPASS,
    ICON_FAD_FILTER_NOTCH,
    ICON_FAD_FILTER_BANDPASS,
    ICON_FAD_FILTER_LOWPASS,
    ICON_FAD_FILTER_HIGHPASS,
};

static const int stvarf_filter_type_count = 4;
static char const *stvarf_filter_types[] = {
    "##1LPDF",
    "##1HPF",
    "##1BPF",
    "##1NF",
};

static char const *stvarf_filter_type_tooltips[] = {
    "1 pole Lowpass Filter",
    "1 pole Highpass Filter",
    "1 pole Bandpass Filter",
    "1 pole Notch Filter",
};

static char const *stvarf_filter_type_icons[] = {
    ICON_FAD_FILTER_LOWPASS,
    ICON_FAD_FILTER_HIGHPASS,
    ICON_FAD_FILTER_BANDPASS,
    ICON_FAD_FILTER_NOTCH,
};

void SynthEditor::RenderFilter(FilterParams *filter)
{
    ImGui::BeginGroup();
    for (int i = 0; i < category_count; i++)
    {
        int category = filter->Pcategory;
        if (ImGui::RadioButton(categories[i], &(category), i))
        {
            filter->Pcategory = i;
        }
        ImGui::SameLine();
    }
    ImGui::EndGroup();

    ImGui::SameLine();

    ImGui::PushItemWidth(100);
    int stages = filter->Pstages + 1;
    if (ImGui::InputInt("Stages", &stages))
    {
        stages = stages - 1;
        if (stages > 4)
        {
            stages = 4;
        }
        if (stages < 0)
        {
            stages = 0;
        }
        filter->Pstages = stages;
    }
    ShowTooltipOnHover("Filter stages (in order to increase dB/oct. value and the order of the filter)");

    switch (filter->Pcategory)
    {
        default:
        {
            ImGui::BeginGroup();
            for (int i = 0; i < analog_filter_type_count; i++)
            {
                int filter_type = filter->Ptype;
                if (ImGui::RadioButton(analog_filter_types[i], &(filter_type), i))
                {
                    filter->Ptype = i;
                }
                ShowTooltipOnHover(analog_filter_type_tooltips[i]);

                ImGui::SameLine();

                ImGui::Text("%s", analog_filter_type_icons[i]);
                ShowTooltipOnHover(analog_filter_type_tooltips[i]);

                ImGui::SameLine();
            }
            ImGui::EndGroup();
            break;
        }
        case 1:
        {
            if (ImGui::Button("Edit"))
            {
            }
            break;
        }
        case 2:
        {
            ImGui::BeginGroup();
            for (int i = 0; i < stvarf_filter_type_count; i++)
            {
                int filter_type = filter->Ptype;
                if (ImGui::RadioButton(stvarf_filter_types[i], &(filter_type), i))
                {
                    filter->Ptype = i;
                }
                ShowTooltipOnHover(stvarf_filter_type_tooltips[i]);

                ImGui::SameLine();

                ImGui::Text("%s", stvarf_filter_type_icons[i]);
                ShowTooltipOnHover(stvarf_filter_type_tooltips[i]);

                ImGui::SameLine();
            }
            ImGui::EndGroup();
            break;
        }
    }

    if (KnobUchar("C.Freq", &(filter->Pfreq), 0, 127, ImVec2(50, 40), "Center Frequency of the Filter or the base position in the vowel's sequence"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("Q", &(filter->Pq), 0, 127, ImVec2(50, 40), "Filter resonance or bandwidth"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("freq.tr.", &(filter->Pfreqtrack), 0, 127, ImVec2(50, 40), "Filter frequency tracking (left is negative, middle is 0, and right is positive)"))
    {
    }

    ImGui::SameLine();

    if (KnobUchar("gain", &(filter->Pgain), 0, 127, ImVec2(50, 40), "Filter output gain/damp"))
    {
    }
}

void SynthEditor::RenderDetune(unsigned char &detuneType, unsigned short int &coarseDetune, unsigned short int &detune)
{
    ImGui::Text("Detune");

    static char const *current_detune_types_item = nullptr;
    auto detune_type = static_cast<int>(detuneType);
    current_detune_types_item = detune_types[detune_type];
    ImGui::PushItemWidth(114);
    if (ImGui::BeginCombo("type", current_detune_types_item))
    {
        for (int n = 0; n < detune_types_count; n++)
        {
            bool is_selected = (current_detune_types_item == detune_types[n]);
            if (ImGui::Selectable(detune_types[n], is_selected))
            {
                current_detune_types_item = detune_types[n];
                detuneType = static_cast<unsigned char>(n);
            }
        }

        ImGui::EndCombo();
    }
    ShowTooltipOnHover("Detune type");

    ImGui::SameLine(200);

    int octave = coarseDetune / 1024;
    if (octave >= 8)
    {
        octave -= 16;
    }
    ImGui::PushItemWidth(100);
    if (ImGui::InputInt("Coarse detune", &octave))
    {
        if (octave < 0)
        {
            octave += 16;
        }

        coarseDetune = octave * 1024 + coarseDetune % 1024;
    }
    ShowTooltipOnHover("Octave");

    auto d = static_cast<int>(detune) - 8192;
    ImGui::PushItemWidth(300);
    if (ImGui::SliderInt("Fine detune", &d, -8192, 8191, "%d"))
    {
        detune = static_cast<unsigned short int>(d + 8192);
    }
    ShowTooltipOnHover("Fine detune (cents)");
}
