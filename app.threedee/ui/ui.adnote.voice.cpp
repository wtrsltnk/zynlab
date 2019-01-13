#include "ui.adnote.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.synth/ADnoteParams.h>

static char const *unison_sizes[] = {
    "OFF",
    "Size 2",
    "Size 3",
    "Size 4",
    "Size 5",
    "Size 6",
    "Size 8",
    "Size 10",
    "Size 12",
    "Size 15",
    "Size 20",
    "Size 25",
    "Size 30",
    "Size 40",
    "Size 50",
};

static char const *detune_types[] = {
    "L35cents",
    "L10cents",
    "E100cents",
    "E1200cents",
};

static char const *modulation_types[] = {
    "OFF",
    "MORPH",
    "RING",
    "PM",
    "FM",
};

void zyn::ui::AdNote::ADNoteVoiceEditor(ADnoteVoiceParam *parameters)
{
    ImGui::Text("ADsynth Voice Parameters of the Instrument");

    auto enabled = parameters->Enabled != 0;
    if (ImGui::Checkbox("Enabled", &enabled))
    {
        parameters->Enabled = enabled ? 1 : 0;
    }
    ImGui::ShowTooltipOnHover("Enable this voice");

    if (ImGui::BeginTabBar("ADNote Voice"))
    {
        if (ImGui::BeginTabItem("Oscillator"))
        {
            ADNoteVoiceEditorOscillator(parameters);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Amplitude"))
        {
            ADNoteVoiceEditorAmplitude(parameters);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Filter"))
        {
            ADNoteVoiceEditorFilter(parameters);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Frequency"))
        {
            ADNoteVoiceEditorFrequency(parameters);

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Modulation"))
        {
            ADNoteVoiceEditorModulation(parameters);

            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

void zyn::ui::AdNote::ADNoteVoiceEditorOscillator(ADnoteVoiceParam *parameters)
{
    ImGui::Text("Voice Oscillator Parameters");

    auto phase = static_cast<int>(64 - parameters->Poscilphase);
    ImGui::PushItemWidth(300);
    if (ImGui::SliderInt("##Phase", &phase, 0, 127, "Phase %d"))
    {
        parameters->PVolume = static_cast<unsigned char>(phase);
    }

    ImGui::Separator();

    ADNoteVoiceEditorOscillatorUnison(parameters);
}

void zyn::ui::AdNote::ADNoteVoiceEditorOscillatorUnison(ADnoteVoiceParam *parameters)
{
    ImGui::Text("Voice Oscillator Unison Parameters");

    auto frequency_spread = static_cast<int>(parameters->Unison_frequency_spread);
    ImGui::PushItemWidth(300);
    if (ImGui::SliderInt("##Frequency Spread", &frequency_spread, 0, 127, "Frequency Spread %d"))
    {
        parameters->Unison_frequency_spread = static_cast<unsigned char>(frequency_spread);
    }
    ImGui::ShowTooltipOnHover("Frequency Spread of the Unison (cents)");

    ImGui::SameLine();

    static char const *current_unison_size_item = nullptr;

    auto unison_size = static_cast<int>(parameters->PFMEnabled);
    current_unison_size_item = unison_sizes[unison_size];
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("Unison", current_unison_size_item))
    {
        for (int n = 0; n < 15; n++)
        {
            bool is_selected = (current_unison_size_item == unison_sizes[n]);
            if (ImGui::Selectable(unison_sizes[n], is_selected))
            {
                current_unison_size_item = unison_sizes[n];
                parameters->PFMEnabled = static_cast<unsigned char>(n);
            }
        }

        ImGui::EndCombo();
    }
    ImGui::ShowTooltipOnHover("Unison size");

    if (ImGui::KnobUchar("Ph.rand", &parameters->Unison_phase_randomness, 0, 127, ImVec2(40, 40), "Phase randomness"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("Stereo", &parameters->Unison_stereo_spread, 0, 127, ImVec2(40, 40), "Stereo Spread"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("Vibrato", &parameters->Unison_vibratto, 0, 127, ImVec2(40, 40), "Vibrato"))
    {
    }

    ImGui::SameLine();

    if (ImGui::KnobUchar("V.speed", &parameters->Unison_vibratto_speed, 0, 127, ImVec2(40, 40), "Vibrato Average Speed"))
    {
    }
}

void zyn::ui::AdNote::ADNoteVoiceEditorAmplitude(ADnoteVoiceParam *parameters)
{
    ImGui::Text("Voice Amplitude Parameters");

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

    ImGui::Separator();

    _AmplitudeLfo.Render(parameters->AmpLfo);
}

void zyn::ui::AdNote::ADNoteVoiceEditorFilter(ADnoteVoiceParam *parameters)
{
    ImGui::Text("Voice Filter Parameters");

    _Filter.Render(parameters->VoiceFilter);

    ImGui::Separator();

    _FilterEnvelope.Render(parameters->FilterEnvelope);

    ImGui::Separator();

    _FilterLfo.Render(parameters->FilterLfo);
}

void zyn::ui::AdNote::ADNoteVoiceEditorFrequency(ADnoteVoiceParam *parameters)
{
    ImGui::Text("Voice Frequency Parameters");

    auto detune = static_cast<int>(parameters->PDetune) - 8192;
    ImGui::PushItemWidth(300);
    if (ImGui::SliderInt("##Detune", &detune, -35, 35, "Detune %d"))
    {
        parameters->PDetune = static_cast<unsigned short int>(detune + 8192);
    }
    ImGui::ShowTooltipOnHover("Fine detune (cents)");

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

    auto octave = static_cast<int>(parameters->PCoarseDetune / 1024);
    if (octave >= 8)
    {
        octave -= 16;
    }
    ImGui::PushItemWidth(300);
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

    _FrequencyEnvelope.Render(parameters->FreqEnvelope);

    ImGui::Separator();

    _FrequencyLfo.Render(parameters->FreqLfo);
}

void zyn::ui::AdNote::ADNoteVoiceEditorModulation(ADnoteVoiceParam *parameters)
{
    ImGui::Text("Voice Modulation Parameters");

    static char const *current_modulation_type_item = nullptr;

    auto modulation_type = static_cast<int>(parameters->PFMEnabled);
    current_modulation_type_item = modulation_types[modulation_type];
    ImGui::PushItemWidth(300);
    if (ImGui::BeginCombo("Modulation type", current_modulation_type_item))
    {
        for (int n = 0; n < 5; n++)
        {
            bool is_selected = (current_modulation_type_item == modulation_types[n]);
            if (ImGui::Selectable(modulation_types[n], is_selected))
            {
                current_modulation_type_item = modulation_types[n];
                parameters->PFMEnabled = static_cast<unsigned char>(n);
            }
        }

        ImGui::EndCombo();
    }
    ImGui::ShowTooltipOnHover("Modulation type");

    if (parameters->PFMEnabled > 0)
    {
        auto vol = static_cast<int>(parameters->PVolume);
        ImGui::PushItemWidth(300);
        if (ImGui::SliderInt("##Vol", &vol, 0, 127, "Vol %d"))
        {
            parameters->PVolume = static_cast<unsigned char>(vol);
        }
        ImGui::ShowTooltipOnHover("Volume");

        auto velocityScale = static_cast<int>(parameters->PAmpVelocityScaleFunction);
        ImGui::PushItemWidth(300);
        if (ImGui::SliderInt("##V.Sns", &velocityScale, 0, 127, "V.Sns %d"))
        {
            parameters->PAmpVelocityScaleFunction = static_cast<unsigned char>(velocityScale);
        }
        ImGui::ShowTooltipOnHover("Velocity Sensing Function (rightmost to disable)");

        auto detune = static_cast<int>(parameters->PDetune) - 8192;
        ImGui::PushItemWidth(300);
        if (ImGui::SliderInt("##Detune", &detune, -35, 35, "Detune %d"))
        {
            parameters->PDetune = static_cast<unsigned short int>(detune + 8192);
        }
        ImGui::ShowTooltipOnHover("Fine detune (cents)");

        ImGui::SameLine();

        static char const *current_detune_types_item = nullptr;

        auto detune_type = static_cast<int>(parameters->PDetuneType);
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
                    parameters->PDetuneType = static_cast<unsigned char>(n);
                }
            }

            ImGui::EndCombo();
        }
        ImGui::ShowTooltipOnHover("Detune type");

        auto FMVolumeDamp = static_cast<int>(parameters->PFMVolumeDamp - 64);
        ImGui::PushItemWidth(300);
        if (ImGui::SliderInt("##F.Damp", &FMVolumeDamp, -64, 63, "F.Damp %d"))
        {
            parameters->PFMVolumeDamp = static_cast<unsigned char>(FMVolumeDamp) + 64;
        }
        ImGui::ShowTooltipOnHover("Modulator Damp at Higher frequency");

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

        _ModulationAmplitudeEnvelope.Render(parameters->FMAmpEnvelope);

        ImGui::Separator();

        _ModulationFrequencyEnvelope.Render(parameters->FMFreqEnvelope);
    }
}
