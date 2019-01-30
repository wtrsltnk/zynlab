#include "ui.effect.h"

#include "../imgui_addons/imgui_knob.h"
#include <zyn.fx/Alienwah.h>
#include <zyn.fx/EffectPresets.h>
#include <zyn.mixer/Mixer.h>

char const *const InsertionFxEditorID = "Insert effect";
char const *const SystemFxEditorID = "System effect";
char const *const TrackFxEditorID = "Track effect";

int EffectNameCount = 9;

char const *const EffectNames[] = {
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

zyn::ui::Effect::Effect(AppState *state)
    : _state(state)
{}

bool zyn::ui::Effect::Setup()
{
    return true;
}

void zyn::ui::Effect::Render()
{
    if (_state->_showInsertEffectsEditor)
    {
        if (ImGui::Begin(InsertionFxEditorID, &_state->_showInsertEffectsEditor))
        {
            if (_state->_currentInsertEffect >= 0 && _state->_currentInsertEffect < NUM_INS_EFX)
            {
                EffectEditor(&_state->_mixer->insefx[_state->_currentInsertEffect]);
            }
        }
        ImGui::End();
    }

    if (_state->_showSystemEffectsEditor)
    {
        if (ImGui::Begin(SystemFxEditorID, &_state->_showSystemEffectsEditor))
        {
            if (_state->_currentSystemEffect >= 0 && _state->_currentSystemEffect < NUM_SYS_EFX)
            {
                EffectEditor(&_state->_mixer->sysefx[_state->_currentSystemEffect]);
            }
        }
        ImGui::End();
    }

    if (_state->_showTrackEffectsEditor)
    {
        if (ImGui::Begin(TrackFxEditorID, &_state->_showTrackEffectsEditor))
        {
            auto track = _state->_mixer->GetTrack(_state->_activeTrack);

            if (_state->_currentTrackEffect >= 0 && _state->_currentTrackEffect < NUM_TRACK_EFX)
            {
                EffectEditor(track->partefx[_state->_currentTrackEffect]);
            }
        }
        ImGui::End();
    }
}

void VolumeAndPanning(EffectManager *effectManager)
{
    auto volume = effectManager->geteffectpar(EffectPresets::Volume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40), "Effect Volume"))
    {
        effectManager->seteffectpar(EffectPresets::Volume, volume);
    }

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(EffectPresets::Panning);
    if (ImGui::KnobUchar("Pan", &pan, 0, 127, ImVec2(40, 40), "Panning"))
    {
        effectManager->seteffectpar(EffectPresets::Panning, pan);
    }
}

void LFOEditor(EffectManager *effectManager, char const *label)
{
    ImGui::Text("%s", label);

    ImGui::SameLine();

    auto lfoType = effectManager->geteffectpar(EffectPresets::LFOFunction);
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("LFO type", lfoType, lfoTypes, lfoTypeCount, "LFO function"))
    {
        effectManager->seteffectpar(EffectPresets::LFOFunction, lfoType);
    }

    auto frequency = effectManager->geteffectpar(EffectPresets::LFOFrequency);
    if (ImGui::KnobUchar("Freq", &frequency, 0, 127, ImVec2(40, 40), "LFO Frequency"))
    {
        effectManager->seteffectpar(EffectPresets::LFOFrequency, frequency);
    }

    ImGui::SameLine();

    auto randomness = effectManager->geteffectpar(EffectPresets::LFORandomness);
    if (ImGui::KnobUchar("Rnd", &randomness, 0, 127, ImVec2(40, 40), "LFO Randomness"))
    {
        effectManager->seteffectpar(EffectPresets::LFORandomness, randomness);
    }

    ImGui::SameLine();

    auto lfoStereo = effectManager->geteffectpar(EffectPresets::LFOStereo);
    if (ImGui::KnobUchar("St.df", &lfoStereo, 0, 127, ImVec2(40, 40), "LFO Left/Right Channel Phase Shift"))
    {
        effectManager->seteffectpar(EffectPresets::LFOStereo, lfoStereo);
    }
}

void zyn::ui::Effect::EffectEditor(EffectManager *effectManager)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
    auto effect = static_cast<unsigned char>(effectManager->geteffect());
    ImGui::PushItemWidth(250);
    if (ImGui::DropDown("Effect", effect, EffectNames, EffectNameCount, "Selected Effect"))
    {
        effectManager->changeeffect(effect);
    }

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

void zyn::ui::Effect::EffectReverbEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Preset", preset, reverbPresetNames, 13))
    {
        effectManager->changepreset(preset);
    }

    ImGui::SameLine();

    auto type = static_cast<unsigned char>(effectManager->geteffectpar(ReverbPresets::ReverbType));
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Type", type, reverbTypeNames, reverbTypeNameCount))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbType, type);
    }

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto roomSize = effectManager->geteffectpar(ReverbPresets::ReverbRoomSize);
    if (ImGui::KnobUchar("Size", &roomSize, 0, 127, ImVec2(40, 40), "Room Size"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbRoomSize, roomSize);
    }

    ImGui::SameLine();

    auto time = effectManager->geteffectpar(ReverbPresets::ReverbTime);
    if (ImGui::KnobUchar("Time", &time, 0, 127, ImVec2(40, 40), "Duration of Effect"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbTime, time);
    }

    ImGui::SameLine();

    auto lpf = effectManager->geteffectpar(ReverbPresets::ReverbLowPassFilter);
    if (ImGui::KnobUchar("LPF", &lpf, 0, 127, ImVec2(40, 40), "Low Pass Filter"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbLowPassFilter, lpf);
    }

    ImGui::SameLine();

    auto hpf = effectManager->geteffectpar(ReverbPresets::ReverbHighPassFilter);
    if (ImGui::KnobUchar("HPF", &hpf, 0, 127, ImVec2(40, 40), "High Pass Filter"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbHighPassFilter, hpf);
    }

    ImGui::SameLine();

    auto dampening = effectManager->geteffectpar(ReverbPresets::ReverbDampening);
    if (ImGui::KnobUchar("Damp.", &dampening, 0, 127, ImVec2(40, 40), "Dampening"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbDampening, dampening);
    }

    ImGui::Separator();

    ImGui::Text("Initial Delay");

    auto initialDelay = effectManager->geteffectpar(ReverbPresets::ReverbInitialDelay);
    if (ImGui::KnobUchar("Delay", &initialDelay, 0, 127, ImVec2(40, 40), "Initial Delay"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbInitialDelay, initialDelay);
    }

    ImGui::SameLine();

    auto initialDelayFeedback = effectManager->geteffectpar(ReverbPresets::ReverbInitialDelayFeedback);
    if (ImGui::KnobUchar("Fb.", &initialDelayFeedback, 0, 127, ImVec2(40, 40), "Initial Delay Feedback"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbInitialDelayFeedback, initialDelayFeedback);
    }

    ImGui::SameLine();
}

void zyn::ui::Effect::EffectEchoEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Preset", preset, echoPresetNames, 9))
    {
        effectManager->changepreset(preset);
    }

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto delay = effectManager->geteffectpar(EchoPresets::EchoDelay);
    if (ImGui::KnobUchar("Delay", &delay, 0, 127, ImVec2(40, 40), "Delay"))
    {
        effectManager->seteffectpar(EchoPresets::EchoDelay, delay);
    }

    ImGui::SameLine();

    auto lrDelay = effectManager->geteffectpar(EchoPresets::EchoDelayBetweenLR);
    if (ImGui::KnobUchar("LRdl.", &lrDelay, 0, 127, ImVec2(40, 40), "Delay Between L/R"))
    {
        effectManager->seteffectpar(EchoPresets::EchoDelayBetweenLR, lrDelay);
    }

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(EchoPresets::EchoChannelRouting);
    if (ImGui::KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(40, 40), "Channel Routing"))
    {
        effectManager->seteffectpar(EchoPresets::EchoChannelRouting, lrCrossover);
    }

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(EchoPresets::EchoFeedback);
    if (ImGui::KnobUchar("Fb.", &feedback, 0, 127, ImVec2(40, 40), "Feedback"))
    {
        effectManager->seteffectpar(EchoPresets::EchoFeedback, feedback);
    }

    ImGui::SameLine();

    auto dampening = effectManager->geteffectpar(EchoPresets::EchoDampening);
    if (ImGui::KnobUchar("Damp.", &dampening, 0, 127, ImVec2(40, 40), "Dampening"))
    {
        effectManager->seteffectpar(EchoPresets::EchoDampening, dampening);
    }
}

void zyn::ui::Effect::EffectChorusEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Preset", preset, chorusPresetNames, 10, "Effect presets"))
    {
        effectManager->changepreset(preset);
    }

    ImGui::SameLine();

    auto subtract = effectManager->geteffectpar(ChorusPresets::ChorusSubtract) == 1;
    if (ImGui::Checkbox("Subtract", &subtract))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusSubtract, subtract ? 1 : 0);
    }
    ImGui::ShowTooltipOnHover("Inverts output");

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto delay = effectManager->geteffectpar(ChorusPresets::ChorusDelay);
    if (ImGui::KnobUchar("Delay", &delay, 0, 127, ImVec2(40, 40), "Delay"))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusDelay, delay);
    }

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(ChorusPresets::ChorusFeedback);
    if (ImGui::KnobUchar("Fb.", &feedback, 0, 127, ImVec2(40, 40), "Feedback"))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusFeedback, feedback);
    }

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(ChorusPresets::ChorusDepth);
    if (ImGui::KnobUchar("Depth", &depth, 0, 127, ImVec2(40, 40), "Depth"))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusDepth, depth);
    }

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(ChorusPresets::ChorusChannelRouting);
    if (ImGui::KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(40, 40), "Channel Routing"))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusChannelRouting, lrCrossover);
    }

    ImGui::Separator();

    LFOEditor(effectManager, "Chorus LFO");
}

void zyn::ui::Effect::EffectPhaserEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Preset", preset, phaserPresetNames, 12, "Effect presets"))
    {
        effectManager->changepreset(preset);
    }

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

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto phase = effectManager->geteffectpar(PhaserPresets::PhaserPhase);
    if (ImGui::KnobUchar("Phase", &phase, 0, 127, ImVec2(40, 40), "Phase"))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserPhase, phase);
    }

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(PhaserPresets::PhaserFeedback);
    if (ImGui::KnobUchar("Fb.", &feedback, 0, 127, ImVec2(40, 40), "Feedback"))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserFeedback, feedback);
    }

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(PhaserPresets::PhaserDepth);
    if (ImGui::KnobUchar("Depth", &depth, 0, 127, ImVec2(40, 40), "Depth"))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserDepth, depth);
    }

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(PhaserPresets::PhaserChannelRouting);
    if (ImGui::KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(40, 40), "Channel Routing"))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserChannelRouting, lrCrossover);
    }

    ImGui::Separator();

    LFOEditor(effectManager, "Phaser LFO");
}

void zyn::ui::Effect::EffectAlienWahEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Preset", preset, alienWahPresetNames, 4, "Effect presets"))
    {
        effectManager->changepreset(preset);
    }

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto phase = effectManager->geteffectpar(AlienWahPresets::AlienWahPhase);
    if (ImGui::KnobUchar("Phase", &phase, 0, 127, ImVec2(40, 40), "Phase"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahPhase, phase);
    }

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(AlienWahPresets::AlienWahFeedback);
    if (ImGui::KnobUchar("Fb.", &feedback, 0, 127, ImVec2(40, 40), "Feedback"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahFeedback, feedback);
    }

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(AlienWahPresets::AlienWahDepth);
    if (ImGui::KnobUchar("Depth", &depth, 0, 127, ImVec2(40, 40), "Depth"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahDepth, depth);
    }

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(AlienWahPresets::AlienWahChannelRouting);
    if (ImGui::KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(40, 40), "Channel Routing"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahChannelRouting, lrCrossover);
    }

    ImGui::SameLine();

    auto delay = effectManager->geteffectpar(AlienWahPresets::AlienWahDelay);
    if (ImGui::KnobUchar("Delay", &delay, 1, MAX_ALIENWAH_DELAY, ImVec2(40, 40), "Delay"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahDelay, delay);
    }

    ImGui::Separator();

    LFOEditor(effectManager, "Alien Wah LFO");
}

void zyn::ui::Effect::EffectDistortionEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Preset", preset, distortionPresetNames, 6, "Effect presets"))
    {
        effectManager->changepreset(preset);
    }

    ImGui::SameLine();

    auto type = static_cast<unsigned char>(effectManager->geteffectpar(DistorsionPresets::DistorsionType));
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Type", type, distortionTypes, distortionTypeCount, "Distortion Type"))
    {
        effectManager->changepreset(type);
    }

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

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto channelRouting = effectManager->geteffectpar(DistorsionPresets::DistorsionChannelRouting);
    if (ImGui::KnobUchar("L/R", &channelRouting, 0, 127, ImVec2(40, 40), "Channel Routing"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionChannelRouting, channelRouting);
    }

    ImGui::SameLine();

    auto drive = effectManager->geteffectpar(DistorsionPresets::DistorsionDrive);
    if (ImGui::KnobUchar("Drive", &drive, 0, 127, ImVec2(40, 40), "Input Amplification"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionDrive, drive);
    }

    ImGui::SameLine();

    auto level = effectManager->geteffectpar(DistorsionPresets::DistorsionLevel);
    if (ImGui::KnobUchar("Level", &level, 0, 127, ImVec2(40, 40), "Output Amplification"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionLevel, level);
    }

    ImGui::SameLine();

    auto lpf = effectManager->geteffectpar(DistorsionPresets::DistorsionLowPassFilter);
    if (ImGui::KnobUchar("LPF", &lpf, 0, 127, ImVec2(40, 40), "Low Pass Filter"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionLowPassFilter, lpf);
    }

    ImGui::SameLine();

    auto hpf = effectManager->geteffectpar(DistorsionPresets::DistorsionHighPassFilter);
    if (ImGui::KnobUchar("HPF", &hpf, 0, 127, ImVec2(40, 40), "High Pass Filter"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionHighPassFilter, hpf);
    }
}

float getEQPlotValue(EffectManager *effectManager, int x, int maxX, int maxY)
{
    int const maxdB = 30;

    auto pos = static_cast<float>(x) / static_cast<float>(maxX);

    if (pos > 1.0f) pos = 1.0f;

    auto dbresp = effectManager->getEQfreqresponse(20.0f * pow(1000.0f, pos));

    return ((dbresp / maxdB + 1.0f) * maxY / 2.0f);
}

void zyn::ui::Effect::EffectEQEditor(EffectManager *effectManager)
{
    auto volume = effectManager->geteffectpar(EffectPresets::Volume);
    if (ImGui::KnobUchar("Vol", &volume, 0, 127, ImVec2(40, 40), "Effect Volume"))
    {
        effectManager->seteffectpar(EffectPresets::Volume, volume);
    }

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
            auto type = static_cast<unsigned char>(effectManager->geteffectpar(presetStart + EQPresets::EQBandType));
            if (ImGui::BeginTabItem(eqBandTypes[type]))
            {
                ImGui::PushItemWidth(100);
                if (ImGui::DropDown("EQ Type", type, eqBandTypes, eqBandTypeCount, "Equalizer type"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandType, type);
                }

                auto frequency = effectManager->geteffectpar(presetStart + EQPresets::EQBandFrequency);
                if (ImGui::KnobUchar("Freq", &frequency, 0, 127, ImVec2(40, 40), "Frequency"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandFrequency, frequency);
                }

                ImGui::SameLine();

                auto gain = effectManager->geteffectpar(presetStart + EQPresets::EQBandGain);
                if (ImGui::KnobUchar("Gain", &gain, 0, 127, ImVec2(40, 40), "Gain"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandGain, gain);
                }

                ImGui::SameLine();

                auto q = effectManager->geteffectpar(presetStart + EQPresets::EQBandQ);
                if (ImGui::KnobUchar("Q", &q, 0, 127, ImVec2(40, 40), "Resonance/Bandwidth"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandQ, q);
                }

                ImGui::SameLine();

                auto stages = effectManager->geteffectpar(presetStart + EQPresets::EQBandStages);
                if (ImGui::KnobUchar("St.", &stages, 0, 127, ImVec2(40, 40), "Additional filter stages"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandStages, stages);
                }

                ImGui::EndTabItem();
            }
            ImGui::PopID();
        }
        ImGui::EndTabBar();
    }
}

void zyn::ui::Effect::EffectDynFilterEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (ImGui::DropDown("Preset", preset, dynFilterPresetNames, 5, "Effect presets"))
    {
        effectManager->changepreset(preset);
    }

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(DynFilterPresets::DynFilterDepth);
    if (ImGui::KnobUchar("Depth", &depth, 0, 127, ImVec2(40, 40), "Depth"))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterDepth, depth);
    }

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
    if (ImGui::KnobUchar("Sense", &ampSense, 0, 127, ImVec2(40, 40), "How the filter varies according to the input amplitude"))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSense, ampSense);
    }

    ImGui::SameLine();

    auto ampSmooth = effectManager->geteffectpar(DynFilterPresets::DynFilterAmplitudeSmooth);
    if (ImGui::KnobUchar("Smooth", &ampSmooth, 0, 127, ImVec2(40, 40), "How smooth the input amplitude changes the filter"))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSmooth, ampSmooth);
    }

    ImGui::Separator();

    ImGui::Separator();

    ImGui::Text("Filter");

    _Filter.Render(effectManager->_filterpars);
}