#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.fx/EffectPresets.h>

char const *const InsertionFxEditorID = "Insert effect";
char const *const SystemFxEditorID = "System effect";
char const *const InstrumentFxEditorID = "Instrument effect";

int AppThreeDee::effectNameCount = 9;

char const *const AppThreeDee::effectNames[] = {
    "No effect",
    "Reverb",
    "Echo",
    "Chorus",
    "Phaser",
    "AlienWah",
    "Distortion",
    "Equalizer",
    "DynFilter",
};

static char const *const reverbPresetNames[] = {
    "Cathedral 1",
    "Cathedral 2",
    "Cathedral 3",
    "Hall 1",
    "Hall 2",
    "Room 1",
    "Room 2",
    "Basement",
    "Tunnel",
    "Echoed 1",
    "Echoed 2",
    "Very Long 1",
    "Very Long 2",
};

static char const *const echoPresetNames[] = {
    "Echo 1",
    "Echo 2",
    "Echo 3",
    "Simple Echo",
    "Canyon",
    "Panning Echo 1",
    "Panning Echo 2",
    "Panning Echo 3",
    "Feedback Echo",
};

static char const *const chorusPresetNames[] = {
    "Chorus 1",
    "Chorus 2",
    "Chorus 3",
    "Celeste 1",
    "Celeste 2",
    "Flange 1",
    "Flange 2",
    "Flange 3",
    "Flange 4",
    "Flange 5",
};

static char const *const phaserPresetNames[] = {
    "Phaser 1",
    "Phaser 2",
    "Phaser 3",
    "Phaser 4",
    "Phaser 5",
    "Phaser 6",
    "APhaser 1",
    "APhaser 2",
    "APhaser 3",
    "APhaser 4",
    "APhaser 5",
    "APhaser 6",
};

static char const *const alienWahPresetNames[] = {
    "Alien Wah 1",
    "Alien Wah 2",
    "Alien Wah 3",
    "Alien Wah 4",
};

static char const *const distortionPresetNames[] = {
    "Overdrive 1",
    "Overdrive 2",
    "A. Exciter 1",
    "A. Exciter 2",
    "Guitar Amp",
    "Quantisize",
};

static char const *const dynFilterPresetNames[] = {
    "WahWah",
    "AutoWah",
    "Sweep",
    "VocalMorph 1",
    "VocalMorph 2",
};

static int const reverbTypeNameCount = 3;
static char const *const reverbTypeNames[] = {
    "Random",
    "Freeverb",
    "Bandwidth",
};

static int const lfoTypeCount = 2;
static char const *const lfoTypes[] = {
    "SINE",
    "TRI",
};

static const int distortionTypeCount = 14;
static char const *const distortionTypes[] = {
    "Atan",
    "Asym1",
    "Pow",
    "Sine",
    "Qnts",
    "Zigzg",
    "Lmt",
    "LmtU",
    "LmtL",
    "ILmt",
    "Clip",
    "Asym2",
    "Pow2",
    "Sign",
};

static const int eqBandTypeCount = 10;
static char const *const eqBandTypes[] = {
    "Off",
    "Lp1",
    "Hp1",
    "Lp2",
    "Hp2",
    "Bp2",
    "N2",
    "Pk",
    "LSh",
    "HSh",
};

void AppThreeDee::InsertEffectEditor()
{
    if (_currentInsertEffect < 0 || _currentInsertEffect >= NUM_INS_EFX)
    {
        return;
    }

    ImGui::Begin(InsertionFxEditorID);

    EffectEditor(&_mixer->insefx[_currentInsertEffect]);

    ImGui::End();
}

void AppThreeDee::SystemEffectEditor()
{
    if (_currentSystemEffect < 0 || _currentSystemEffect >= NUM_SYS_EFX)
    {
        return;
    }

    ImGui::Begin(SystemFxEditorID);

    EffectEditor(&_mixer->sysefx[_currentSystemEffect]);

    ImGui::End();
}

void AppThreeDee::InstrumentEffectEditor()
{
    if (_currentInstrumentEffect < 0 || _currentInstrumentEffect >= NUM_CHANNEL_EFX)
    {
        return;
    }

    auto channel = _mixer->GetChannel(_sequencer.ActiveInstrument());

    ImGui::Begin(InstrumentFxEditorID);

    EffectEditor(channel->partefx[_currentInstrumentEffect]);

    ImGui::End();
}

bool PresetSelection(char const *label, int &value, char const *const names[], int nameCount)
{
    bool value_changed = false;

    auto current_effect_item = names[value];
    if (ImGui::BeginCombo(label, current_effect_item))
    {
        for (int n = 0; n < nameCount; n++)
        {
            bool is_selected = (current_effect_item == names[n]);
            if (ImGui::Selectable(names[n], is_selected))
            {
                current_effect_item = names[n];
                value = n;
                value_changed = true;
            }
        }

        ImGui::EndCombo();
    }
    ImGui::ShowTooltipOnHover(label);

    return value_changed;
}

void AppThreeDee::EffectEditor(EffectManager *effectManager)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
    auto effect = static_cast<int>(effectManager->geteffect());
    ImGui::PushItemWidth(250);
    if (PresetSelection("Effect", effect, effectNames, effectNameCount))
    {
        effectManager->changeeffect(effect);
    }
    ImGui::ShowTooltipOnHover("Selected Effect");

    ImGui::Separator();

    switch (effectManager->geteffect())
    {
        case 1:
            EffectReverbEditor(effectManager);
            break;
        case 2:
            EffectEchoEditor(effectManager);
            break;
        case 3:
            EffectChorusEditor(effectManager);
            break;
        case 4:
            EffectPhaserEditor(effectManager);
            break;
        case 5:
            EffectAlienWahEditor(effectManager);
            break;
        case 6:
            EffectDistortionEditor(effectManager);
            break;
        case 7:
            EffectEQEditor(effectManager);
            break;
        case 8:
            EffectDynFilterEditor(effectManager);
            break;
    };
    ImGui::PopStyleVar();
}

void AppThreeDee::EffectReverbEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, reverbPresetNames, 13))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }

    ImGui::SameLine();

    auto type = static_cast<int>(effectManager->geteffectpar(ReverbPresets::ReverbType));
    ImGui::PushItemWidth(100);
    if (PresetSelection("Type", type, reverbTypeNames, reverbTypeNameCount))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbType, type);
    }

    auto volume = effectManager->geteffectpar(ReverbPresets::ReverbVolume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbVolume, volume);
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(ReverbPresets::ReverbPanning);
    if (ImGui::KnobUchar("Pan", &pan, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbPanning, pan);
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto roomSize = effectManager->geteffectpar(ReverbPresets::ReverbRoomSize);
    if (ImGui::KnobUchar("Size", &roomSize, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbRoomSize, roomSize);
    }
    ImGui::ShowTooltipOnHover("Room Size");

    ImGui::SameLine();

    auto time = effectManager->geteffectpar(ReverbPresets::ReverbTime);
    if (ImGui::KnobUchar("Time", &time, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbTime, time);
    }
    ImGui::ShowTooltipOnHover("Duration of Effect");

    ImGui::SameLine();

    auto lpf = effectManager->geteffectpar(ReverbPresets::ReverbLowPassFilter);
    if (ImGui::KnobUchar("LPF", &lpf, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbLowPassFilter, lpf);
    }
    ImGui::ShowTooltipOnHover("Low Pass Filter");

    ImGui::SameLine();

    auto hpf = effectManager->geteffectpar(ReverbPresets::ReverbHighPassFilter);
    if (ImGui::KnobUchar("HPF", &hpf, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbHighPassFilter, hpf);
    }
    ImGui::ShowTooltipOnHover("High Pass Filter");

    ImGui::SameLine();

    auto dampening = effectManager->geteffectpar(ReverbPresets::ReverbDampening);
    if (ImGui::KnobUchar("Damp.", &dampening, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbDampening, dampening);
    }
    ImGui::ShowTooltipOnHover("Dampening");

    ImGui::Separator();

    ImGui::Text("Initial Delay");

    auto initialDelay = effectManager->geteffectpar(ReverbPresets::ReverbInitialDelay);
    if (ImGui::KnobUchar("Delay", &initialDelay, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbInitialDelay, initialDelay);
    }
    ImGui::ShowTooltipOnHover("Initial Delay");

    ImGui::SameLine();

    auto initialDelayFeedback = effectManager->geteffectpar(ReverbPresets::ReverbInitialDelayFeedback);
    if (ImGui::KnobUchar("Fb.", &initialDelayFeedback, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbInitialDelayFeedback, initialDelayFeedback);
    }
    ImGui::ShowTooltipOnHover("Initial Delay Feedback");

    ImGui::SameLine();
}

void AppThreeDee::EffectEchoEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, echoPresetNames, 9))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }

    auto volume = effectManager->geteffectpar(EchoPresets::EchoVolume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoVolume, volume);
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(EchoPresets::EchoPanning);
    if (ImGui::KnobUchar("Pan", &pan, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoPanning, pan);
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto delay = effectManager->geteffectpar(EchoPresets::EchoDelay);
    if (ImGui::KnobUchar("Delay", &delay, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoDelay, delay);
    }
    ImGui::ShowTooltipOnHover("Delay");

    ImGui::SameLine();

    auto lrDelay = effectManager->geteffectpar(EchoPresets::EchoDelayBetweenLR);
    if (ImGui::KnobUchar("LRdl.", &lrDelay, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoDelayBetweenLR, lrDelay);
    }
    ImGui::ShowTooltipOnHover("Delay Between L/R");

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(EchoPresets::EchoChannelRouting);
    if (ImGui::KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoChannelRouting, lrCrossover);
    }
    ImGui::ShowTooltipOnHover("L/R Channel Routing");

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(EchoPresets::EchoFeedback);
    if (ImGui::KnobUchar("Fb.", &feedback, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoFeedback, feedback);
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();

    auto dampening = effectManager->geteffectpar(EchoPresets::EchoDampening);
    if (ImGui::KnobUchar("Damp.", &dampening, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoDampening, dampening);
    }
    ImGui::ShowTooltipOnHover("Dampening");
}

void AppThreeDee::EffectChorusEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, chorusPresetNames, 10))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
    ImGui::ShowTooltipOnHover("Effect presets");

    ImGui::SameLine();

    auto subtract = effectManager->geteffectpar(ChorusPresets::ChorusSubtract) == 1;
    if (ImGui::Checkbox("Subtract", &subtract))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusSubtract, subtract ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("Inverts output");

    auto volume = effectManager->geteffectpar(ChorusPresets::ChorusVolume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusVolume, volume);
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(ChorusPresets::ChorusPanning);
    if (ImGui::KnobUchar("Pan", &pan, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusPanning, pan);
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto delay = effectManager->geteffectpar(ChorusPresets::ChorusDelay);
    if (ImGui::KnobUchar("Delay", &delay, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusDelay, delay);
    }
    ImGui::ShowTooltipOnHover("Delay");

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(ChorusPresets::ChorusFeedback);
    if (ImGui::KnobUchar("Fb.", &feedback, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusFeedback, feedback);
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(ChorusPresets::ChorusDepth);
    if (ImGui::KnobUchar("Depth", &depth, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusDepth, depth);
    }
    ImGui::ShowTooltipOnHover("Depth");

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(ChorusPresets::ChorusChannelRouting);
    if (ImGui::KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusChannelRouting, lrCrossover);
    }
    ImGui::ShowTooltipOnHover("Channel Routing");

    ImGui::Separator();

    ImGui::Text("Chorus LFO");

    ImGui::SameLine();

    auto lfoType = static_cast<int>(effectManager->geteffectpar(ChorusPresets::ChorusLFOFunction));
    ImGui::PushItemWidth(100);
    if (PresetSelection("LFO type", lfoType, lfoTypes, lfoTypeCount))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusLFOFunction, static_cast<unsigned char>(lfoType));
    }
    ImGui::ShowTooltipOnHover("LFO function");

    auto frequency = effectManager->geteffectpar(ChorusPresets::ChorusLFOFrequency);
    if (ImGui::KnobUchar("Freq", &frequency, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusLFOFrequency, frequency);
    }
    ImGui::ShowTooltipOnHover("LFO Frequency");

    ImGui::SameLine();

    auto randomness = effectManager->geteffectpar(ChorusPresets::ChorusLFORandomness);
    if (ImGui::KnobUchar("Rnd", &randomness, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusLFORandomness, randomness);
    }
    ImGui::ShowTooltipOnHover("LFO Randomness");

    ImGui::SameLine();

    auto lfoStereo = effectManager->geteffectpar(ChorusPresets::ChorusLFOStereo);
    if (ImGui::KnobUchar("St.df", &lfoStereo, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusLFOStereo, lfoStereo);
    }
    ImGui::ShowTooltipOnHover("LFO Left/Right Channel Phase Shift");
}

void AppThreeDee::EffectPhaserEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, phaserPresetNames, 12))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
    ImGui::ShowTooltipOnHover("Effect presets");

    auto subtract = effectManager->geteffectpar(PhaserPresets::PhaserSubtract) == 1;
    if (ImGui::Checkbox("Subtract", &subtract))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserSubtract, subtract ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("Inverts output");

    ImGui::SameLine();

    auto hyper = effectManager->geteffectpar(PhaserPresets::PhaserHyper) == 1;
    if (ImGui::Checkbox("Hyp", &hyper))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserHyper, hyper ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("Hyper");

    ImGui::SameLine();

    auto analog = effectManager->geteffectpar(PhaserPresets::PhaserAnalog) == 1;
    if (ImGui::Checkbox("Analog", &analog))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserAnalog, analog ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("Analog");

    auto volume = effectManager->geteffectpar(PhaserPresets::PhaserVolume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserVolume, volume);
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(PhaserPresets::PhaserPanning);
    if (ImGui::KnobUchar("Pan", &pan, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserPanning, pan);
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto phase = effectManager->geteffectpar(PhaserPresets::PhaserPhase);
    if (ImGui::KnobUchar("Phase", &phase, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserPhase, phase);
    }
    ImGui::ShowTooltipOnHover("Phase");

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(PhaserPresets::PhaserFeedback);
    if (ImGui::KnobUchar("Fb.", &feedback, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserFeedback, feedback);
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(PhaserPresets::PhaserDepth);
    if (ImGui::KnobUchar("Depth", &depth, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserDepth, depth);
    }
    ImGui::ShowTooltipOnHover("Depth");

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(PhaserPresets::PhaserChannelRouting);
    if (ImGui::KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserChannelRouting, lrCrossover);
    }
    ImGui::ShowTooltipOnHover("Channel Routing");

    ImGui::Separator();

    ImGui::Text("Phaser LFO");

    ImGui::SameLine();

    auto lfoType = static_cast<int>(effectManager->geteffectpar(PhaserPresets::PhaserLFOFunction));
    ImGui::PushItemWidth(100);
    if (PresetSelection("LFO type", lfoType, lfoTypes, lfoTypeCount))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserLFOFunction, static_cast<unsigned char>(lfoType));
    }
    ImGui::ShowTooltipOnHover("LFO function");

    auto frequency = effectManager->geteffectpar(PhaserPresets::PhaserLFOFrequency);
    if (ImGui::KnobUchar("Freq", &frequency, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserLFOFrequency, frequency);
    }
    ImGui::ShowTooltipOnHover("LFO Frequency");

    ImGui::SameLine();

    auto randomness = effectManager->geteffectpar(PhaserPresets::PhaserLFORandomness);
    if (ImGui::KnobUchar("Rnd", &randomness, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserLFORandomness, randomness);
    }
    ImGui::ShowTooltipOnHover("LFO Randomness");

    ImGui::SameLine();

    auto lfoStereo = effectManager->geteffectpar(PhaserPresets::PhaserLFOStereo);
    if (ImGui::KnobUchar("St.df", &lfoStereo, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserLFOStereo, lfoStereo);
    }
    ImGui::ShowTooltipOnHover("LFO Left/Right Channel Phase Shift");
}

void AppThreeDee::EffectAlienWahEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, alienWahPresetNames, 4))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
    ImGui::ShowTooltipOnHover("Effect presets");

    auto volume = effectManager->geteffectpar(AlienWahPresets::AlienWahVolume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahVolume, volume);
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(AlienWahPresets::AlienWahPanning);
    if (ImGui::KnobUchar("Pan", &pan, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahPanning, pan);
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto phase = effectManager->geteffectpar(AlienWahPresets::AlienWahPhase);
    if (ImGui::KnobUchar("Phase", &phase, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahPhase, phase);
    }
    ImGui::ShowTooltipOnHover("Phase");

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(AlienWahPresets::AlienWahFeedback);
    if (ImGui::KnobUchar("Fb.", &feedback, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahFeedback, feedback);
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(AlienWahPresets::AlienWahDepth);
    if (ImGui::KnobUchar("Depth", &depth, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahDepth, depth);
    }
    ImGui::ShowTooltipOnHover("Depth");

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(AlienWahPresets::AlienWahChannelRouting);
    if (ImGui::KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahChannelRouting, lrCrossover);
    }
    ImGui::ShowTooltipOnHover("Channel Routing");

    ImGui::SameLine();

    auto delay = effectManager->geteffectpar(AlienWahPresets::AlienWahDelay);
    if (ImGui::KnobUchar("Delay", &delay, 1, MAX_ALIENWAH_DELAY, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahDelay, delay);
    }
    ImGui::ShowTooltipOnHover("Delay");

    ImGui::Separator();

    ImGui::Text("Alien Wah LFO");

    ImGui::SameLine();

    auto lfoType = static_cast<int>(effectManager->geteffectpar(AlienWahPresets::AlienWahLFOFunction));
    ImGui::PushItemWidth(100);
    if (PresetSelection("LFO type", lfoType, lfoTypes, lfoTypeCount))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahLFOFunction, static_cast<unsigned char>(lfoType));
    }
    ImGui::ShowTooltipOnHover("LFO function");

    auto frequency = effectManager->geteffectpar(AlienWahPresets::AlienWahLFOFrequency);
    if (ImGui::KnobUchar("Freq", &frequency, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahLFOFrequency, frequency);
    }
    ImGui::ShowTooltipOnHover("LFO Frequency");

    ImGui::SameLine();

    auto randomness = effectManager->geteffectpar(AlienWahPresets::AlienWahLFORandomness);
    if (ImGui::KnobUchar("Rnd", &randomness, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahLFORandomness, randomness);
    }
    ImGui::ShowTooltipOnHover("LFO Randomness");

    ImGui::SameLine();

    auto lfoStereo = effectManager->geteffectpar(AlienWahPresets::AlienWahLFOStereo);
    if (ImGui::KnobUchar("St.df", &lfoStereo, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahLFOStereo, lfoStereo);
    }
    ImGui::ShowTooltipOnHover("LFO Left/Right Channel Phase Shift");
}

void AppThreeDee::EffectDistortionEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, distortionPresetNames, 6))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
    ImGui::ShowTooltipOnHover("Effect presets");

    ImGui::SameLine();

    auto type = static_cast<int>(effectManager->geteffectpar(DistorsionPresets::DistorsionType));
    ImGui::PushItemWidth(100);
    if (PresetSelection("Type", type, distortionTypes, distortionTypeCount))
    {
        effectManager->changepreset(static_cast<unsigned char>(type));
    }
    ImGui::ShowTooltipOnHover("Distortion Type");

    auto negate = effectManager->geteffectpar(DistorsionPresets::DistorsionNegate) == 1;
    if (ImGui::Checkbox("Negate", &negate))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionNegate, negate ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("When the input is negated");

    ImGui::SameLine();

    auto preFiltering = effectManager->geteffectpar(DistorsionPresets::DistorsionPreFiltering) == 1;
    if (ImGui::Checkbox("PreFiltering", &preFiltering))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionPreFiltering, preFiltering ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("Applies the filters(before or after) the distorsion");

    ImGui::SameLine();

    auto stereo = effectManager->geteffectpar(DistorsionPresets::DistorsionStereo) == 1;
    if (ImGui::Checkbox("Stereo", &stereo))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionStereo, stereo ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("Stereo (0=mono, 1=stereo)");

    auto volume = effectManager->geteffectpar(DistorsionPresets::DistorsionVolume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionVolume, volume);
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(DistorsionPresets::DistorsionPanning);
    if (ImGui::KnobUchar("Pan", &pan, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionPanning, pan);
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto channelRouting = effectManager->geteffectpar(DistorsionPresets::DistorsionChannelRouting);
    if (ImGui::KnobUchar("L/R", &channelRouting, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionChannelRouting, channelRouting);
    }
    ImGui::ShowTooltipOnHover("Channel Routing");

    ImGui::SameLine();

    auto drive = effectManager->geteffectpar(DistorsionPresets::DistorsionDrive);
    if (ImGui::KnobUchar("Drive", &drive, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionDrive, drive);
    }
    ImGui::ShowTooltipOnHover("Input Amplification");

    ImGui::SameLine();

    auto level = effectManager->geteffectpar(DistorsionPresets::DistorsionLevel);
    if (ImGui::KnobUchar("Level", &level, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionLevel, level);
    }
    ImGui::ShowTooltipOnHover("Output Amplification");

    ImGui::SameLine();

    auto lpf = effectManager->geteffectpar(DistorsionPresets::DistorsionLowPassFilter);
    if (ImGui::KnobUchar("LPF", &lpf, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionLowPassFilter, lpf);
    }
    ImGui::ShowTooltipOnHover("Low Pass Filter");

    ImGui::SameLine();

    auto hpf = effectManager->geteffectpar(DistorsionPresets::DistorsionHighPassFilter);
    if (ImGui::KnobUchar("HPF", &hpf, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionHighPassFilter, hpf);
    }
    ImGui::ShowTooltipOnHover("High Pass Filter");
}

float getEQPlotValue(EffectManager *effectManager, int x, int maxX, int maxY)
{
    int const maxdB = 30;

    auto pos = static_cast<float>(x) / static_cast<float>(maxX);

    if (pos > 1.0f) pos = 1.0f;

    auto dbresp = effectManager->getEQfreqresponse(20.0f * pow(1000.0f, pos));

    return ((dbresp / maxdB + 1.0f) * maxY / 2.0f);
}

void AppThreeDee::EffectEQEditor(EffectManager *effectManager)
{
    auto volume = effectManager->geteffectpar(EQPresets::EQVolume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EQPresets::EQVolume, volume);
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    int lx = 200;
    int ly = 50;
    std::vector<float> values;
    for (int i = 0; i < lx; i++)
    {
        values.push_back(getEQPlotValue(effectManager, i, lx, ly));
    }
    ImGui::PlotLines("EQ", &(values[0]), static_cast<int>(values.size()), 0, nullptr, 0, ly, ImVec2(lx, ly));

    if (ImGui::BeginTabBar("EQ bands"))
    {
        for (int band = 0; band < MAX_EQ_BANDS; band++)
        {
            auto presetStart = (band * 5);
            ImGui::PushID(band);
            auto type = static_cast<int>(effectManager->geteffectpar(presetStart + EQPresets::EQBandType));
            if (ImGui::BeginTabItem(eqBandTypes[type]))
            {
                ImGui::PushItemWidth(100);
                if (PresetSelection("EQ Type", type, eqBandTypes, eqBandTypeCount))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandType, static_cast<unsigned char>(type));
                }

                auto frequency = effectManager->geteffectpar(presetStart + EQPresets::EQBandFrequency);
                if (ImGui::KnobUchar("Freq", &frequency, 0, 127, ImVec2(40, 40)))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandFrequency, frequency);
                }
                ImGui::ShowTooltipOnHover("Frequency");

                ImGui::SameLine();

                auto gain = effectManager->geteffectpar(presetStart + EQPresets::EQBandGain);
                if (ImGui::KnobUchar("Gain", &gain, 0, 127, ImVec2(40, 40)))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandGain, gain);
                }
                ImGui::ShowTooltipOnHover("Gain");

                ImGui::SameLine();

                auto q = effectManager->geteffectpar(presetStart + EQPresets::EQBandQ);
                if (ImGui::KnobUchar("Q", &q, 0, 127, ImVec2(40, 40)))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandQ, q);
                }
                ImGui::ShowTooltipOnHover("Resonance/Bandwidth");

                ImGui::SameLine();

                auto stages = effectManager->geteffectpar(presetStart + EQPresets::EQBandStages);
                if (ImGui::KnobUchar("St.", &stages, 0, 127, ImVec2(40, 40)))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandStages, stages);
                }
                ImGui::ShowTooltipOnHover("Additional filter stages");

                ImGui::EndTabItem();
            }
            ImGui::PopID();
        }
        ImGui::EndTabBar();
    }
}

void AppThreeDee::EffectDynFilterEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, dynFilterPresetNames, 5))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }

    auto volume = effectManager->geteffectpar(DynFilterPresets::DynFilterVolume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterVolume, volume);
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(DynFilterPresets::DynFilterPanning);
    if (ImGui::KnobUchar("Pan", &pan, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterPanning, pan);
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(DynFilterPresets::DynFilterDepth);
    if (ImGui::KnobUchar("Depth", &depth, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterDepth, depth);
    }
    ImGui::ShowTooltipOnHover("Depth");

    ImGui::Separator();

    ImGui::Text("Input Amplitude");

    ImGui::SameLine();

    auto ampSenseInvert = effectManager->geteffectpar(DynFilterPresets::DynFilterAmplitudeSenseInvert) == 1;
    if (ImGui::Checkbox("Invert", &ampSenseInvert))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSenseInvert, ampSenseInvert ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("Lower filter freq when the input amplitude rises");

    auto ampSense = effectManager->geteffectpar(DynFilterPresets::DynFilterAmplitudeSense);
    if (ImGui::KnobUchar("Sense", &ampSense, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSense, ampSense);
    }
    ImGui::ShowTooltipOnHover("How the filter varies according to the input amplitude");

    ImGui::SameLine();

    auto ampSmooth = effectManager->geteffectpar(DynFilterPresets::DynFilterAmplitudeSmooth);
    if (ImGui::KnobUchar("Smooth", &ampSmooth, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSmooth, ampSmooth);
    }
    ImGui::ShowTooltipOnHover("How smooth the input amplitude changes the filter");

    ImGui::Separator();

    ImGui::Text("Dynamic Filter LFO");

    ImGui::SameLine();

    auto lfoType = static_cast<int>(effectManager->geteffectpar(DynFilterPresets::DynFilterLFOFunction));
    ImGui::PushItemWidth(100);
    if (PresetSelection("LFO type", lfoType, lfoTypes, lfoTypeCount))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterLFOFunction, static_cast<unsigned char>(lfoType));
    }
    ImGui::ShowTooltipOnHover("LFO function");

    auto frequency = effectManager->geteffectpar(DynFilterPresets::DynFilterLFOFrequency);
    if (ImGui::KnobUchar("Freq", &frequency, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterLFOFrequency, frequency);
    }
    ImGui::ShowTooltipOnHover("LFO Frequency");

    ImGui::SameLine();

    auto randomness = effectManager->geteffectpar(DynFilterPresets::DynFilterLFORandomness);
    if (ImGui::KnobUchar("Rnd", &randomness, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterLFORandomness, randomness);
    }
    ImGui::ShowTooltipOnHover("LFO Randomness");

    ImGui::SameLine();

    auto lfoStereo = effectManager->geteffectpar(DynFilterPresets::DynFilterLFOStereo);
    if (ImGui::KnobUchar("St.df", &lfoStereo, 0, 127, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterLFOStereo, lfoStereo);
    }
    ImGui::ShowTooltipOnHover("LFO Left/Right Channel Phase Shift");

    ImGui::Separator();

    ImGui::Text("Filter");

    FilterParameters(effectManager->filterpars);
}
