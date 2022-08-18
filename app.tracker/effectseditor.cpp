#include "effectseditor.h"

#include <imgui.h>
#include <imgui_helpers.h>
#include <sstream>
#include <string>
#include <vector>
#include <zyn.fx/Alienwah.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.mixer/Track.h>

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"

EffectsAndAutomationEditor::EffectsAndAutomationEditor()
    : _session(nullptr)
{}

void EffectsAndAutomationEditor::SetUp(
    ApplicationSession *session)
{
    _session = session;
}

std::vector<std::string> EffectNames = {
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

static std::vector<std::string> reverbPresetNames = {
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

static std::vector<std::string> echoPresetNames = {
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

static std::vector<std::string> chorusPresetNames = {
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

static std::vector<std::string> phaserPresetNames = {
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

static std::vector<std::string> alienWahPresetNames = {
    "Alien Wah 1",
    "Alien Wah 2",
    "Alien Wah 3",
    "Alien Wah 4",
};

static std::vector<std::string> distortionPresetNames = {
    "Overdrive 1",
    "Overdrive 2",
    "A. Exciter 1",
    "A. Exciter 2",
    "Guitar Amp",
    "Quantisize",
};

static std::vector<std::string> dynFilterPresetNames = {
    "WahWah",
    "AutoWah",
    "Sweep",
    "VocalMorph 1",
    "VocalMorph 2",
};

static std::vector<std::string> reverbTypeNames = {
    "Random",
    "Freeverb",
    "Bandwidth",
};

static int const lfoTypeCount = 2;
static std::vector<std::string> lfoTypes = {
    "SINE",
    "TRI",
};

static std::vector<std::string> distortionTypes = {
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

static std::vector<std::string> eqBandTypes = {
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

namespace ImGui
{
    void ShowTooltipOnHover(
        char const *tooltip);
}

enum EffectPresets
{
    Volume = 0,
    Panning = 1,
    LFOFrequency = 2,
    LFORandomness = 3,
    LFOFunction = 4,
    LFOStereo = 5,
};

enum ReverbPresets
{
    ReverbTime = 2,
    ReverbInitialDelay = 3,
    ReverbInitialDelayFeedback = 4,
    ReverbUnused1 = 5,
    ReverbUnused2 = 6,
    ReverbLowPassFilter = 7,
    ReverbHighPassFilter = 8,
    ReverbDampening = 9,
    ReverbType = 10,
    ReverbRoomSize = 11,
    ReverbBandwidth = 12,
};

enum EchoPresets
{
    EchoDelay = 2,
    EchoDelayBetweenLR = 3,
    EchoChannelRouting = 4,
    EchoFeedback = 5,
    EchoDampening = 6,
};

enum ChorusPresets
{
    ChorusDepth = 6,
    ChorusDelay = 7,
    ChorusFeedback = 8,
    ChorusChannelRouting = 9,
    ChorusUnused1 = 10,
    ChorusSubtract = 11,
};

enum PhaserPresets
{
    PhaserDepth = 6,
    PhaserFeedback = 7,
    PhaserStages = 8,
    PhaserChannelRouting = 9,
    PhaserSubtract = 10,
    PhaserPhase = 11,
    PhaserHyper = 12,
    PhaserDistortion = 13,
    PhaserAnalog = 14,
};

enum AlienWahPresets
{
    AlienWahDepth = 6,
    AlienWahFeedback = 7,
    AlienWahDelay = 8,
    AlienWahChannelRouting = 9,
    AlienWahPhase = 10,
};

enum DistorsionPresets
{
    DistorsionChannelRouting = 2,
    DistorsionDrive = 3,
    DistorsionLevel = 4,
    DistorsionType = 5,
    DistorsionNegate = 6,
    DistorsionLowPassFilter = 7,
    DistorsionHighPassFilter = 8,
    DistorsionStereo = 9,
    DistorsionPreFiltering = 10,
};

enum EQPresets
{
    EQBandType = 10,
    EQBandFrequency = 11,
    EQBandGain = 12,
    EQBandQ = 13,
    EQBandStages = 14,
};

enum DynFilterPresets
{
    DynFilterDepth = 6,
    DynFilterAmplitudeSense = 7,
    DynFilterAmplitudeSenseInvert = 8,
    DynFilterAmplitudeSmooth = 9,
};

void VolumeAndPanning(EffectManager *effectManager)
{
    auto volume = effectManager->geteffectpar(EffectPresets::Volume);
    if (KnobUchar("Vol", &volume, 0, 127, ImVec2(30, 30), "Effect Volume"))
    {
        effectManager->seteffectpar(EffectPresets::Volume, volume);
    }

    ImGui::SameLine();

    auto pan = effectManager->geteffectpar(EffectPresets::Panning);
    if (KnobUchar("Pan", &pan, 0, 127, ImVec2(30, 30), "Panning"))
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
    if (DropDown("LFO type", lfoType, lfoTypes, "LFO function"))
    {
        effectManager->seteffectpar(EffectPresets::LFOFunction, lfoType);
    }

    auto frequency = effectManager->geteffectpar(EffectPresets::LFOFrequency);
    if (KnobUchar("Freq", &frequency, 0, 127, ImVec2(30, 30), "LFO Frequency"))
    {
        effectManager->seteffectpar(EffectPresets::LFOFrequency, frequency);
    }

    ImGui::SameLine();

    auto randomness = effectManager->geteffectpar(EffectPresets::LFORandomness);
    if (KnobUchar("Rnd", &randomness, 0, 127, ImVec2(30, 30), "LFO Randomness"))
    {
        effectManager->seteffectpar(EffectPresets::LFORandomness, randomness);
    }

    ImGui::SameLine();

    auto lfoStereo = effectManager->geteffectpar(EffectPresets::LFOStereo);
    if (KnobUchar("St.df", &lfoStereo, 0, 127, ImVec2(30, 30), "LFO Left/Right Channel Phase Shift"))
    {
        effectManager->seteffectpar(EffectPresets::LFOStereo, lfoStereo);
    }
}

void EffectReverbEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (DropDown("Preset", preset, reverbPresetNames, nullptr))
    {
        effectManager->changepreset(preset);
    }

    ImGui::SameLine();

    auto type = static_cast<unsigned char>(effectManager->geteffectpar(ReverbPresets::ReverbType));
    ImGui::PushItemWidth(100);
    if (DropDown("Type", type, reverbTypeNames, nullptr))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbType, type);
    }

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto roomSize = effectManager->geteffectpar(ReverbPresets::ReverbRoomSize);
    if (KnobUchar("Size", &roomSize, 0, 127, ImVec2(30, 30), "Room Size"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbRoomSize, roomSize);
    }

    ImGui::SameLine();

    auto time = effectManager->geteffectpar(ReverbPresets::ReverbTime);
    if (KnobUchar("Time", &time, 0, 127, ImVec2(30, 30), "Duration of Effect"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbTime, time);
    }

    ImGui::SameLine();

    auto lpf = effectManager->geteffectpar(ReverbPresets::ReverbLowPassFilter);
    if (KnobUchar("LPF", &lpf, 0, 127, ImVec2(30, 30), "Low Pass Filter"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbLowPassFilter, lpf);
    }

    ImGui::SameLine();

    auto hpf = effectManager->geteffectpar(ReverbPresets::ReverbHighPassFilter);
    if (KnobUchar("HPF", &hpf, 0, 127, ImVec2(30, 30), "High Pass Filter"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbHighPassFilter, hpf);
    }

    ImGui::SameLine();

    auto dampening = effectManager->geteffectpar(ReverbPresets::ReverbDampening);
    if (KnobUchar("Damp.", &dampening, 0, 127, ImVec2(30, 30), "Dampening"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbDampening, dampening);
    }

    ImGui::Separator();

    ImGui::Text("Initial Delay");

    auto initialDelay = effectManager->geteffectpar(ReverbPresets::ReverbInitialDelay);
    if (KnobUchar("Delay", &initialDelay, 0, 127, ImVec2(30, 30), "Initial Delay"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbInitialDelay, initialDelay);
    }

    ImGui::SameLine();

    auto initialDelayFeedback = effectManager->geteffectpar(ReverbPresets::ReverbInitialDelayFeedback);
    if (KnobUchar("Fb.", &initialDelayFeedback, 0, 127, ImVec2(30, 30), "Initial Delay Feedback"))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbInitialDelayFeedback, initialDelayFeedback);
    }

    ImGui::SameLine();
}

void EffectEchoEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (DropDown("Preset", preset, echoPresetNames, nullptr))
    {
        effectManager->changepreset(preset);
    }

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto delay = effectManager->geteffectpar(EchoPresets::EchoDelay);
    if (KnobUchar("Delay", &delay, 0, 127, ImVec2(30, 30), "Delay"))
    {
        effectManager->seteffectpar(EchoPresets::EchoDelay, delay);
    }

    ImGui::SameLine();

    auto lrDelay = effectManager->geteffectpar(EchoPresets::EchoDelayBetweenLR);
    if (KnobUchar("LRdl.", &lrDelay, 0, 127, ImVec2(30, 30), "Delay Between L/R"))
    {
        effectManager->seteffectpar(EchoPresets::EchoDelayBetweenLR, lrDelay);
    }

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(EchoPresets::EchoChannelRouting);
    if (KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(30, 30), "Channel Routing"))
    {
        effectManager->seteffectpar(EchoPresets::EchoChannelRouting, lrCrossover);
    }

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(EchoPresets::EchoFeedback);
    if (KnobUchar("Fb.", &feedback, 0, 127, ImVec2(30, 30), "Feedback"))
    {
        effectManager->seteffectpar(EchoPresets::EchoFeedback, feedback);
    }

    ImGui::SameLine();

    auto dampening = effectManager->geteffectpar(EchoPresets::EchoDampening);
    if (KnobUchar("Damp.", &dampening, 0, 127, ImVec2(30, 30), "Dampening"))
    {
        effectManager->seteffectpar(EchoPresets::EchoDampening, dampening);
    }
}

void EffectChorusEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (DropDown("Preset", preset, chorusPresetNames, "Effect presets"))
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
    if (KnobUchar("Delay", &delay, 0, 127, ImVec2(30, 30), "Delay"))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusDelay, delay);
    }

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(ChorusPresets::ChorusFeedback);
    if (KnobUchar("Fb.", &feedback, 0, 127, ImVec2(30, 30), "Feedback"))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusFeedback, feedback);
    }

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(ChorusPresets::ChorusDepth);
    if (KnobUchar("Depth", &depth, 0, 127, ImVec2(30, 30), "Depth"))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusDepth, depth);
    }

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(ChorusPresets::ChorusChannelRouting);
    if (KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(30, 30), "Channel Routing"))
    {
        effectManager->seteffectpar(ChorusPresets::ChorusChannelRouting, lrCrossover);
    }

    ImGui::Separator();

    LFOEditor(effectManager, "LFO");
}

void EffectPhaserEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (DropDown("Preset", preset, phaserPresetNames, "Effect presets"))
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
    if (KnobUchar("Phase", &phase, 0, 127, ImVec2(30, 30), "Phase"))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserPhase, phase);
    }

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(PhaserPresets::PhaserFeedback);
    if (KnobUchar("Fb.", &feedback, 0, 127, ImVec2(30, 30), "Feedback"))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserFeedback, feedback);
    }

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(PhaserPresets::PhaserDepth);
    if (KnobUchar("Depth", &depth, 0, 127, ImVec2(30, 30), "Depth"))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserDepth, depth);
    }

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(PhaserPresets::PhaserChannelRouting);
    if (KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(30, 30), "Channel Routing"))
    {
        effectManager->seteffectpar(PhaserPresets::PhaserChannelRouting, lrCrossover);
    }

    ImGui::Separator();

    LFOEditor(effectManager, "LFO");
}

void EffectAlienWahEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (DropDown("Preset", preset, alienWahPresetNames, "Effect presets"))
    {
        effectManager->changepreset(preset);
    }

    VolumeAndPanning(effectManager);

    ImGui::SameLine();

    auto phase = effectManager->geteffectpar(AlienWahPresets::AlienWahPhase);
    if (KnobUchar("Phase", &phase, 0, 127, ImVec2(30, 30), "Phase"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahPhase, phase);
    }

    ImGui::SameLine();

    auto feedback = effectManager->geteffectpar(AlienWahPresets::AlienWahFeedback);
    if (KnobUchar("Fb.", &feedback, 0, 127, ImVec2(30, 30), "Feedback"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahFeedback, feedback);
    }

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(AlienWahPresets::AlienWahDepth);
    if (KnobUchar("Depth", &depth, 0, 127, ImVec2(30, 30), "Depth"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahDepth, depth);
    }

    ImGui::SameLine();

    auto lrCrossover = effectManager->geteffectpar(AlienWahPresets::AlienWahChannelRouting);
    if (KnobUchar("L/R", &lrCrossover, 0, 127, ImVec2(30, 30), "Channel Routing"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahChannelRouting, lrCrossover);
    }

    ImGui::SameLine();

    auto delay = effectManager->geteffectpar(AlienWahPresets::AlienWahDelay);
    if (KnobUchar("Delay", &delay, 1, MAX_ALIENWAH_DELAY, ImVec2(30, 30), "Delay"))
    {
        effectManager->seteffectpar(AlienWahPresets::AlienWahDelay, delay);
    }

    ImGui::Separator();

    LFOEditor(effectManager, "LFO");
}

void EffectDistortionEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (DropDown("Preset", preset, distortionPresetNames, "Effect presets"))
    {
        effectManager->changepreset(preset);
    }

    ImGui::SameLine();

    auto type = static_cast<unsigned char>(effectManager->geteffectpar(DistorsionPresets::DistorsionType));
    ImGui::PushItemWidth(100);
    if (DropDown("Type", type, distortionTypes, "Distortion Type"))
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
    if (KnobUchar("L/R", &channelRouting, 0, 127, ImVec2(30, 30), "Channel Routing"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionChannelRouting, channelRouting);
    }

    ImGui::SameLine();

    auto drive = effectManager->geteffectpar(DistorsionPresets::DistorsionDrive);
    if (KnobUchar("Drive", &drive, 0, 127, ImVec2(30, 30), "Input Amplification"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionDrive, drive);
    }

    ImGui::SameLine();

    auto level = effectManager->geteffectpar(DistorsionPresets::DistorsionLevel);
    if (KnobUchar("Level", &level, 0, 127, ImVec2(30, 30), "Output Amplification"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionLevel, level);
    }

    ImGui::SameLine();

    auto lpf = effectManager->geteffectpar(DistorsionPresets::DistorsionLowPassFilter);
    if (KnobUchar("LPF", &lpf, 0, 127, ImVec2(30, 30), "Low Pass Filter"))
    {
        effectManager->seteffectpar(DistorsionPresets::DistorsionLowPassFilter, lpf);
    }

    ImGui::SameLine();

    auto hpf = effectManager->geteffectpar(DistorsionPresets::DistorsionHighPassFilter);
    if (KnobUchar("HPF", &hpf, 0, 127, ImVec2(30, 30), "High Pass Filter"))
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

void EffectEQEditor(EffectManager *effectManager)
{
    auto volume = effectManager->geteffectpar(EffectPresets::Volume);
    if (KnobUchar("Vol", &volume, 0, 127, ImVec2(30, 30), "Effect Volume"))
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
            char label[32];
            sprintf(label, "Band %d   ", band);
            if (ImGui::BeginTabItem(label, nullptr, type != 0 ? ImGuiTabItemFlags_UnsavedDocument : ImGuiTabItemFlags_None))
            {
                ImGui::PushItemWidth(100);
                if (DropDown("EQ Type", type, eqBandTypes, "Equalizer type"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandType, type);
                }

                auto frequency = effectManager->geteffectpar(presetStart + EQPresets::EQBandFrequency);
                if (KnobUchar("Freq", &frequency, 0, 127, ImVec2(30, 30), "Frequency"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandFrequency, frequency);
                }

                ImGui::SameLine();

                auto gain = effectManager->geteffectpar(presetStart + EQPresets::EQBandGain);
                if (KnobUchar("Gain", &gain, 0, 127, ImVec2(30, 30), "Gain"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandGain, gain);
                }

                ImGui::SameLine();

                auto q = effectManager->geteffectpar(presetStart + EQPresets::EQBandQ);
                if (KnobUchar("Q", &q, 0, 127, ImVec2(30, 30), "Resonance/Bandwidth"))
                {
                    effectManager->seteffectpar(presetStart + EQPresets::EQBandQ, q);
                }

                ImGui::SameLine();

                auto stages = effectManager->geteffectpar(presetStart + EQPresets::EQBandStages);
                if (KnobUchar("St.", &stages, 0, 127, ImVec2(30, 30), "Additional filter stages"))
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

void EffectDynFilterEditor(EffectManager *effectManager)
{
    auto preset = static_cast<unsigned char>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (DropDown("Preset", preset, dynFilterPresetNames, "Effect presets"))
    {
        effectManager->changepreset(preset);
    }

    VolumeAndPanning(effectManager);

    ImGui::Separator();

    LFOEditor(effectManager, "LFO");

    ImGui::SameLine();

    auto depth = effectManager->geteffectpar(DynFilterPresets::DynFilterDepth);
    if (KnobUchar("Depth", &depth, 0, 127, ImVec2(30, 30), "Depth"))
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
    if (KnobUchar("Sense", &ampSense, 0, 127, ImVec2(30, 30), "How the filter varies according to the input amplitude"))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSense, ampSense);
    }

    ImGui::SameLine();

    auto ampSmooth = effectManager->geteffectpar(DynFilterPresets::DynFilterAmplitudeSmooth);
    if (KnobUchar("Smooth", &ampSmooth, 0, 127, ImVec2(30, 30), "How smooth the input amplitude changes the filter"))
    {
        effectManager->seteffectpar(DynFilterPresets::DynFilterAmplitudeSmooth, ampSmooth);
    }

    ImGui::Separator();

    ImGui::Text("Filter");

    //_Filter.Render(effectManager->_filterpars);
}

void EffectEditor(EffectManager *effectManager)
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
    auto effect = static_cast<unsigned char>(effectManager->geteffect());
    ImGui::PushItemWidth(250);
    if (DropDown("Effect", effect, EffectNames, "Selected Effect"))
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
    }
    ImGui::PopStyleVar();
}

void EffectsAndAutomationEditor::Render2d()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "Effects and Automation",
        nullptr,
        flags);
    {
        static int selectedTab = 0;
        ImGui::BeginGroup();
        if (ImGui::Selectable(ICON_FAD_PRESET_AB, selectedTab == 0, ImGuiSelectableFlags_None, ImVec2(20, 0))) selectedTab = 0;
        ImGui::ShowTooltipOnHover("Track FX editor");
        if (ImGui::Selectable(ICON_FAD_AUTOMATION_4P, selectedTab == 1, ImGuiSelectableFlags_None, ImVec2(20, 0))) selectedTab = 1;
        ImGui::ShowTooltipOnHover("Track automation editor");
        ImGui::EndGroup();

        ImGui::SameLine(40);

        auto track = _session->_mixer->GetTrack(_session->_mixer->State.currentTrack);
        if (selectedTab == 0)
        {
            ImGui::BeginGroup();
            auto effectWidth = (ImGui::GetWindowContentRegionWidth() / 3) - 20;

            for (int e = 0; e < NUM_TRACK_EFX; e++)
            {
                if (e > 0) ImGui::SameLine();
                ImGui::PushID(e);

                unsigned char item_current = track->partefx[e]->geteffect();
                ImGui::BeginChild("fx", ImVec2(item_current == 0 ? 120.0f : effectWidth, 0.0f), true);
                {
                    EffectEditor(track->partefx[e]);
                }
                ImGui::EndChild();
                ImGui::PopID();
            }
            ImGui::EndGroup();
        }

        if (selectedTab == 1)
        {
            static ParamIndices selectedParam = ParamIndices::None;

            ImGui::BeginChild("params", ImVec2(260, 0), true);
            if (ImGui::CollapsingHeader("Mixer"))
            {
                if (ImGui::Selectable("Volume", selectedParam == ParamIndices::Mixer_Volume))
                {
                    selectedParam = ParamIndices::Mixer_Volume;
                }
                if (ImGui::Selectable("Panning", selectedParam == ParamIndices::Mixer_Panning))
                {
                    selectedParam = ParamIndices::Mixer_Panning;
                }
            }

            auto showSystemFx = _session->_mixer->GetSystemEffectType(0) > 0     // system effect 1
                                || _session->_mixer->GetSystemEffectType(1) > 0  // system effect 2
                                || _session->_mixer->GetSystemEffectType(2) > 0  // system effect 3
                                || _session->_mixer->GetSystemEffectType(3) > 0; // system effect 4

            if (showSystemFx && ImGui::CollapsingHeader("System FX"))
            {
                for (int i = 0; i < NUM_SYS_EFX; i++)
                {
                    auto effect = _session->_mixer->GetSystemEffectType(i);
                    if (effect > 0)
                    {
                        auto paramIndex = (ParamIndices)(int(ParamIndices::SystemFX_1_Volume) + i);
                        std::stringstream paramName;
                        paramName << EffectNames[effect] << " volume";
                        if (ImGui::Selectable(paramName.str().c_str(), selectedParam == paramIndex))
                        {
                            selectedParam = paramIndex;
                        }
                    }
                }
            }

            auto showInsertFx = (_session->_mixer->GetTrackIndexForInsertEffect(0) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(0) > 0)     // insert effect 1
                                || (_session->_mixer->GetTrackIndexForInsertEffect(1) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(1) > 0)  // insert effect 2
                                || (_session->_mixer->GetTrackIndexForInsertEffect(2) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(2) > 0)  // insert effect 3
                                || (_session->_mixer->GetTrackIndexForInsertEffect(3) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(3) > 0)  // insert effect 4
                                || (_session->_mixer->GetTrackIndexForInsertEffect(4) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(4) > 0)  // insert effect 5
                                || (_session->_mixer->GetTrackIndexForInsertEffect(5) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(5) > 0)  // insert effect 6
                                || (_session->_mixer->GetTrackIndexForInsertEffect(6) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(6) > 0)  // insert effect 7
                                || (_session->_mixer->GetTrackIndexForInsertEffect(7) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(7) > 0); // insert effect 8

            if (showInsertFx && ImGui::CollapsingHeader("Insert FX"))
            {
                for (int i = 0; i < NUM_INS_EFX; i++)
                {
                    if ((_session->_mixer->GetTrackIndexForInsertEffect(i) == _session->_mixer->State.currentTrack && _session->_mixer->GetInsertEffectType(i) > 0))
                    {
                        RenderEffect(ParamIndices::InsertFX_1 + (100 * i), i, _session->_mixer->GetInsertEffectType(i), selectedParam);
                    }
                }
            }

            auto showTrackFx = track->partefx[0]->geteffect() > 0 || track->partefx[1]->geteffect() > 0 || track->partefx[2]->geteffect() > 0;

            if (showTrackFx && ImGui::CollapsingHeader("Track FX"))
            {
                for (int i = 0; i < NUM_TRACK_EFX; i++)
                {
                    if (track->partefx[i]->geteffect() > 0)
                    {
                        RenderEffect(ParamIndices::TrackFX_1 + (100 * i), i, track->partefx[i]->geteffect(), selectedParam);
                    }
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();

            RenderAutomatedParam(selectedParam);
        }
    }
    ImGui::End();
}

void EffectsAndAutomationEditor::RenderAutomatedParam(
    int selectedParam)
{
    auto song = _session->_song;
    auto pattern = song->GetPattern(song->currentPattern);
    auto params = pattern->AutomatedTrackParameters(_session->_mixer->State.currentTrack);
    auto param = params.find(selectedParam);

    if (param == params.end())
    {
        return;
    }

    auto keyFrames = param->second._keyFrames;

    float arr[5] = {0, 0.3f, 0.4f, 0.1f, 0.9f};
    ImGui::BeginChild("paramCurve");

    ImGui::EndChild();
}

void EffectsAndAutomationEditor::RenderEffect(
    int group,
    int index,
    int effect,
    ParamIndices &selectedParams)
{
    if (ImGui::TreeNode(EffectNames[effect].c_str()))
    {
        if (effect == 1) // Reverb
        {
            if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(group + (index * 100) + Reverb_Volume)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Reverb_Volume);
            }

            if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(group + (index * 100) + Reverb_Pan)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Reverb_Pan);
            }

            if (ImGui::Selectable("Time", selectedParams == (ParamIndices)(group + (index * 100) + Reverb_Time)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Reverb_Time);
            }

            if (ImGui::Selectable("Initial delay", selectedParams == (ParamIndices)(group + (index * 100) + Reverb_InitialDelay)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Reverb_InitialDelay);
            }

            if (ImGui::Selectable("Initial delay feedback", selectedParams == (ParamIndices)(group + (index * 100) + Reverb_InitialDelayFeedback)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Reverb_InitialDelayFeedback);
            }

            if (ImGui::Selectable("LowPass", selectedParams == (ParamIndices)(group + (index * 100) + Reverb_LowPass)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Reverb_LowPass);
            }

            if (ImGui::Selectable("HighPass", selectedParams == (ParamIndices)(group + (index * 100) + Reverb_HighPass)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Reverb_HighPass);
            }
        }

        if (effect == 2) // Echo
        {
            if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(group + (index * 100) + Echo_Volume)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Echo_Volume);
            }

            if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(group + (index * 100) + Echo_Pan)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Echo_Pan);
            }

            if (ImGui::Selectable("LeftRightDelay", selectedParams == (ParamIndices)(group + (index * 100) + Echo_LeftRightDelay)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Echo_LeftRightDelay);
            }

            if (ImGui::Selectable("CrossingDelay", selectedParams == (ParamIndices)(group + (index * 100) + Echo_CrossingDelay)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Echo_CrossingDelay);
            }

            if (ImGui::Selectable("Feedback", selectedParams == (ParamIndices)(group + (index * 100) + Echo_Feedback)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Echo_Feedback);
            }

            if (ImGui::Selectable("Damp", selectedParams == (ParamIndices)(group + (index * 100) + Echo_Damp)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Echo_Damp);
            }
        }

        if (effect == 3) // Chorus
        {
            if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_Volume)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_Volume);
            }

            if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_Pan)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_Pan);
            }

            if (ImGui::Selectable("LFOFrequency", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_LFOFrequency)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_LFOFrequency);
            }

            if (ImGui::Selectable("LFORandomness", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_LFORandomness)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_LFORandomness);
            }

            if (ImGui::Selectable("LeftRightPhaseDifference", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_LeftRightPhaseDifference)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_LeftRightPhaseDifference);
            }

            if (ImGui::Selectable("LFODepth", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_LFODepth)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_LFODepth);
            }

            if (ImGui::Selectable("Delay", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_Delay)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_Delay);
            }

            if (ImGui::Selectable("Feedback", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_Feedback)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_Feedback);
            }

            if (ImGui::Selectable("Subtract", selectedParams == (ParamIndices)(group + (index * 100) + Chorus_Subtract)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Chorus_Subtract);
            }
        }

        if (effect == 4) // Phaser
        {
            if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_Volume)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_Volume);
            }

            if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_Pan)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_Pan);
            }

            if (ImGui::Selectable("LFOFrequency", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_LFOFrequency)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_LFOFrequency);
            }

            if (ImGui::Selectable("LFORandomness", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_LFORandomness)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_LFORandomness);
            }

            if (ImGui::Selectable("LeftRightPhaseDifference", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_LeftRightPhaseDifference)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_LeftRightPhaseDifference);
            }

            if (ImGui::Selectable("LFODepth", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_LFODepth)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_LFODepth);
            }

            if (ImGui::Selectable("Stages", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_Stages)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_Stages);
            }

            if (ImGui::Selectable("Feedback", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_Feedback)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_Feedback);
            }

            if (ImGui::Selectable("Subtract", selectedParams == (ParamIndices)(group + (index * 100) + Phaser_Subtract)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Phaser_Subtract);
            }
        }

        if (effect == 5) // AlienWah
        {
            if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_Volume)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_Volume);
            }

            if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_Pan)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_Pan);
            }

            if (ImGui::Selectable("LFOFrequency", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_LFOFrequency)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_LFOFrequency);
            }

            if (ImGui::Selectable("LFORandomness", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_LFORandomness)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_LFORandomness);
            }

            if (ImGui::Selectable("LeftRightPhaseDifference", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_LeftRightPhaseDifference)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_LeftRightPhaseDifference);
            }

            if (ImGui::Selectable("LFODepth", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_LFODepth)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_LFODepth);
            }

            if (ImGui::Selectable("Delay", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_Delay)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_Delay);
            }

            if (ImGui::Selectable("Feedback", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_Feedback)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_Feedback);
            }

            if (ImGui::Selectable("Subtract", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_Subtract)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_Subtract);
            }

            if (ImGui::Selectable("Phase", selectedParams == (ParamIndices)(group + (index * 100) + AlienWah_Phase)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + AlienWah_Phase);
            }
        }

        if (effect == 6) // Distorsion
        {
            if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(group + (index * 100) + Distorsion_Volume)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Distorsion_Volume);
            }

            if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(group + (index * 100) + Distorsion_Pan)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Distorsion_Pan);
            }

            if (ImGui::Selectable("Drive", selectedParams == (ParamIndices)(group + (index * 100) + Distorsion_Drive)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Distorsion_Drive);
            }

            if (ImGui::Selectable("Level", selectedParams == (ParamIndices)(group + (index * 100) + Distorsion_Level)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Distorsion_Level);
            }

            if (ImGui::Selectable("LowPass", selectedParams == (ParamIndices)(group + (index * 100) + Distorsion_LowPass)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Distorsion_LowPass);
            }

            if (ImGui::Selectable("HighPass", selectedParams == (ParamIndices)(group + (index * 100) + Distorsion_HighPass)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Distorsion_HighPass);
            }

            if (ImGui::Selectable("DryWetMix", selectedParams == (ParamIndices)(group + (index * 100) + Distorsion_DryWetMix)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + Distorsion_DryWetMix);
            }
        }

        if (effect == 7) // EQ
        {
            if (ImGui::Selectable("Gain", selectedParams == (ParamIndices)(group + (index * 100) + EQ_Gain)))
            {
                selectedParams = (ParamIndices)(group + (index * 100) + EQ_Gain);
            }
        }

        ImGui::TreePop();
    }
}
