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
    "EQ",
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
        effectManager->seteffectpar(ReverbPresets::ReverbType, static_cast<unsigned char>(type));
    }

    auto volume = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbVolume));
    if (ImGui::Knob("Vol", &volume, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbVolume, static_cast<unsigned char>(volume));
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto roomSize = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbRoomSize));
    if (ImGui::Knob("Size", &roomSize, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbRoomSize, static_cast<unsigned char>(roomSize));
    }
    ImGui::ShowTooltipOnHover("Room Size");

    ImGui::SameLine();

    auto time = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbTime));
    if (ImGui::Knob("Time", &time, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbTime, static_cast<unsigned char>(time));
    }
    ImGui::ShowTooltipOnHover("Duration of Effect");

    ImGui::SameLine();

    auto lpf = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbLowPassFilter));
    if (ImGui::Knob("LPF", &lpf, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbLowPassFilter, static_cast<unsigned char>(lpf));
    }
    ImGui::ShowTooltipOnHover("Low Pass Filter");

    ImGui::SameLine();

    auto hpf = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbHighPassFilter));
    if (ImGui::Knob("HPF", &hpf, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbHighPassFilter, static_cast<unsigned char>(hpf));
    }
    ImGui::ShowTooltipOnHover("High Pass Filter");

    ImGui::SameLine();

    auto dampening = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbDampening));
    if (ImGui::Knob("Damp.", &dampening, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbDampening, static_cast<unsigned char>(dampening));
    }
    ImGui::ShowTooltipOnHover("Dampening");

    ImGui::Separator();

    ImGui::Text("Initial Delay");

    auto initialDelay = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbInitialDelay));
    if (ImGui::Knob("Delay", &initialDelay, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbInitialDelay, static_cast<unsigned char>(initialDelay));
    }
    ImGui::ShowTooltipOnHover("Initial Delay");

    ImGui::SameLine();

    auto initialDelayFeedback = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbInitialDelayFeedback));
    if (ImGui::Knob("Fb.", &initialDelayFeedback, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbInitialDelayFeedback, static_cast<unsigned char>(initialDelayFeedback));
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

    auto volume = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoVolume));
    if (ImGui::Knob("Vol", &volume, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoVolume, static_cast<unsigned char>(volume));
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto delay = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoDelay));
    if (ImGui::Knob("Delay", &delay, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoDelay, static_cast<unsigned char>(delay));
    }
    ImGui::ShowTooltipOnHover("Delay");

    ImGui::SameLine();

    auto lrDelay = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoDelayBetweenLR));
    if (ImGui::Knob("LRdl.", &lrDelay, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoDelayBetweenLR, static_cast<unsigned char>(lrDelay));
    }
    ImGui::ShowTooltipOnHover("Delay Between L/R");

    ImGui::SameLine();

    auto lrCrossover = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoChannelRouting));
    if (ImGui::Knob("L/R", &lrCrossover, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoChannelRouting, static_cast<unsigned char>(lrCrossover));
    }
    ImGui::ShowTooltipOnHover("L/R Channel Routing");

    ImGui::SameLine();

    auto feedback = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoFeedback));
    if (ImGui::Knob("Fb.", &feedback, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoFeedback, static_cast<unsigned char>(feedback));
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();

    auto dampening = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoDampening));
    if (ImGui::Knob("Damp.", &dampening, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoDampening, static_cast<unsigned char>(dampening));
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

    auto volume = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusVolume));
    if (ImGui::Knob("Vol", &volume, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusVolume, static_cast<unsigned char>(volume));
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto delay = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusDelay));
    if (ImGui::Knob("Delay", &delay, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusDelay, static_cast<unsigned char>(delay));
    }
    ImGui::ShowTooltipOnHover("Delay");

    ImGui::SameLine();

    auto feedback = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusFeedback));
    if (ImGui::Knob("Fb.", &feedback, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusFeedback, static_cast<unsigned char>(feedback));
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();

    auto depth = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusDepth));
    if (ImGui::Knob("Depth", &depth, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusDepth, static_cast<unsigned char>(depth));
    }
    ImGui::ShowTooltipOnHover("Depth");

    ImGui::SameLine();

    auto lrCrossover = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusChannelRouting));
    if (ImGui::Knob("L/R", &lrCrossover, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusChannelRouting, static_cast<unsigned char>(lrCrossover));
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

    auto frequency = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusLFOFrequency));
    if (ImGui::Knob("Freq", &frequency, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusLFOFrequency, static_cast<unsigned char>(frequency));
    }
    ImGui::ShowTooltipOnHover("LFO Frequency");

    ImGui::SameLine();

    auto randomness = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusLFORandomness));
    if (ImGui::Knob("Rnd", &randomness, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusLFORandomness, static_cast<unsigned char>(randomness));
    }
    ImGui::ShowTooltipOnHover("LFO Randomness");

    ImGui::SameLine();

    auto lfoStereo = static_cast<float>(effectManager->geteffectpar(ChorusPresets::ChorusLFOStereo));
    if (ImGui::Knob("St.df", &lfoStereo, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusLFOStereo, static_cast<unsigned char>(lfoStereo));
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

    auto volume = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserVolume));
    if (ImGui::Knob("Vol", &volume, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserVolume, static_cast<unsigned char>(volume));
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto phase = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserPhase));
    if (ImGui::Knob("Phase", &phase, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserPhase, static_cast<unsigned char>(phase));
    }
    ImGui::ShowTooltipOnHover("Phase");

    ImGui::SameLine();

    auto feedback = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserFeedback));
    if (ImGui::Knob("Fb.", &feedback, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserFeedback, static_cast<unsigned char>(feedback));
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();

    auto depth = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserDepth));
    if (ImGui::Knob("Depth", &depth, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserDepth, static_cast<unsigned char>(depth));
    }
    ImGui::ShowTooltipOnHover("Depth");

    ImGui::SameLine();

    auto lrCrossover = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserChannelRouting));
    if (ImGui::Knob("L/R", &lrCrossover, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserChannelRouting, static_cast<unsigned char>(lrCrossover));
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

    auto frequency = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserLFOFrequency));
    if (ImGui::Knob("Freq", &frequency, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserLFOFrequency, static_cast<unsigned char>(frequency));
    }
    ImGui::ShowTooltipOnHover("LFO Frequency");

    ImGui::SameLine();

    auto randomness = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserLFORandomness));
    if (ImGui::Knob("Rnd", &randomness, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserLFORandomness, static_cast<unsigned char>(randomness));
    }
    ImGui::ShowTooltipOnHover("LFO Randomness");

    ImGui::SameLine();

    auto lfoStereo = static_cast<float>(effectManager->geteffectpar(PhaserPresets::PhaserLFOStereo));
    if (ImGui::Knob("St.df", &lfoStereo, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserLFOStereo, static_cast<unsigned char>(lfoStereo));
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

    auto volume = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahVolume));
    if (ImGui::Knob("Vol", &volume, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahVolume, static_cast<unsigned char>(volume));
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto phase = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahPhase));
    if (ImGui::Knob("Phase", &phase, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahPhase, static_cast<unsigned char>(phase));
    }
    ImGui::ShowTooltipOnHover("Phase");

    ImGui::SameLine();

    auto feedback = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahFeedback));
    if (ImGui::Knob("Fb.", &feedback, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahFeedback, static_cast<unsigned char>(feedback));
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();

    auto depth = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahDepth));
    if (ImGui::Knob("Depth", &depth, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahDepth, static_cast<unsigned char>(depth));
    }
    ImGui::ShowTooltipOnHover("Depth");

    ImGui::SameLine();

    auto lrCrossover = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahChannelRouting));
    if (ImGui::Knob("L/R", &lrCrossover, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahChannelRouting, static_cast<unsigned char>(lrCrossover));
    }
    ImGui::ShowTooltipOnHover("Channel Routing");

    ImGui::SameLine();

    auto delay = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahDelay));
    if (ImGui::Knob("Delay", &delay, 1, MAX_ALIENWAH_DELAY, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahDelay, static_cast<unsigned char>(delay));
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

    auto frequency = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahLFOFrequency));
    if (ImGui::Knob("Freq", &frequency, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahLFOFrequency, static_cast<unsigned char>(frequency));
    }
    ImGui::ShowTooltipOnHover("LFO Frequency");

    ImGui::SameLine();

    auto randomness = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahLFORandomness));
    if (ImGui::Knob("Rnd", &randomness, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahLFORandomness, static_cast<unsigned char>(randomness));
    }
    ImGui::ShowTooltipOnHover("LFO Randomness");

    ImGui::SameLine();

    auto lfoStereo = static_cast<float>(effectManager->geteffectpar(AlienWahPresets::AlienWahLFOStereo));
    if (ImGui::Knob("St.df", &lfoStereo, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahLFOStereo, static_cast<unsigned char>(lfoStereo));
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

    auto volume = static_cast<float>(effectManager->geteffectpar(DistorsionPresets::DistorsionVolume));
    if (ImGui::Knob("Vol", &volume, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionVolume, static_cast<unsigned char>(volume));
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(DistorsionPresets::DistorsionPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto channelRouting = static_cast<float>(effectManager->geteffectpar(DistorsionPresets::DistorsionChannelRouting));
    if (ImGui::Knob("L/R", &channelRouting, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionChannelRouting, static_cast<unsigned char>(channelRouting));
    }
    ImGui::ShowTooltipOnHover("Channel Routing");

    ImGui::SameLine();

    auto drive = static_cast<float>(effectManager->geteffectpar(DistorsionPresets::DistorsionDrive));
    if (ImGui::Knob("Drive", &drive, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionDrive, static_cast<unsigned char>(drive));
    }
    ImGui::ShowTooltipOnHover("Input Amplification");

    ImGui::SameLine();

    auto level = static_cast<float>(effectManager->geteffectpar(DistorsionPresets::DistorsionLevel));
    if (ImGui::Knob("Level", &level, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionLevel, static_cast<unsigned char>(level));
    }
    ImGui::ShowTooltipOnHover("Output Amplification");

    ImGui::SameLine();

    auto lpf = static_cast<float>(effectManager->geteffectpar(DistorsionPresets::DistorsionLowPassFilter));
    if (ImGui::Knob("LPF", &lpf, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionLowPassFilter, static_cast<unsigned char>(lpf));
    }
    ImGui::ShowTooltipOnHover("Low Pass Filter");

    ImGui::SameLine();

    auto hpf = static_cast<float>(effectManager->geteffectpar(DistorsionPresets::DistorsionHighPassFilter));
    if (ImGui::Knob("HPF", &hpf, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionHighPassFilter, static_cast<unsigned char>(hpf));
    }
    ImGui::ShowTooltipOnHover("High Pass Filter");
}

void AppThreeDee::EffectEQEditor(EffectManager *effectManager)
{
}

void AppThreeDee::EffectDynFilterEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, dynFilterPresetNames, 5))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }

    auto volume = static_cast<float>(effectManager->geteffectpar(DynFilterPresets::DynFilterVolume));
    if (ImGui::Knob("Vol", &volume, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterVolume, static_cast<unsigned char>(volume));
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(DynFilterPresets::DynFilterPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();

    auto depth = static_cast<float>(effectManager->geteffectpar(DynFilterPresets::DynFilterDepth));
    if (ImGui::Knob("Depth", &depth, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterDepth, static_cast<unsigned char>(depth));
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

    auto ampSense = static_cast<float>(effectManager->geteffectpar(DynFilterPresets::DynFilterAmplitudeSense));
    if (ImGui::Knob("Sense", &ampSense, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSense, static_cast<unsigned char>(ampSense));
    }
    ImGui::ShowTooltipOnHover("How the filter varies according to the input amplitude");

    ImGui::SameLine();

    auto ampSmooth = static_cast<float>(effectManager->geteffectpar(DynFilterPresets::DynFilterAmplitudeSmooth));
    if (ImGui::Knob("Smooth", &ampSmooth, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSmooth, static_cast<unsigned char>(ampSmooth));
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

    auto frequency = static_cast<float>(effectManager->geteffectpar(DynFilterPresets::DynFilterLFOFrequency));
    if (ImGui::Knob("Freq", &frequency, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterLFOFrequency, static_cast<unsigned char>(frequency));
    }
    ImGui::ShowTooltipOnHover("LFO Frequency");

    ImGui::SameLine();

    auto randomness = static_cast<float>(effectManager->geteffectpar(DynFilterPresets::DynFilterLFORandomness));
    if (ImGui::Knob("Rnd", &randomness, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterLFORandomness, static_cast<unsigned char>(randomness));
    }
    ImGui::ShowTooltipOnHover("LFO Randomness");

    ImGui::SameLine();

    auto lfoStereo = static_cast<float>(effectManager->geteffectpar(DynFilterPresets::DynFilterLFOStereo));
    if (ImGui::Knob("St.df", &lfoStereo, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterLFOStereo, static_cast<unsigned char>(lfoStereo));
    }
    ImGui::ShowTooltipOnHover("LFO Left/Right Channel Phase Shift");

    ImGui::Separator();

    ImGui::Text("Filter");

    FilterParameters(effectManager->filterpars);
}
