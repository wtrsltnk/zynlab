#include "../app.threedee.h"

#include "../imgui_addons/imgui_knob.h"

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
    auto effect = static_cast<int>(effectManager->geteffect());
    ImGui::PushItemWidth(250);
    if (PresetSelection("Effect", effect, effectNames, effectNameCount))
    {
        effectManager->changeeffect(effect);
    }

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
}

enum ReverbPresets
{
    ReverbVolume = 0,
    ReverbPanning = 1,
    Time = 2,
    InitialDelay = 3,
    InitialDelayFeedback = 4,
    Unused1 = 5,
    Unused2 = 6,
    LowPassFilter = 7,
    HighPassFilter = 8,
    ReverbDampening = 9,
    Type = 10,
    RoomSize = 11
};

void AppThreeDee::EffectReverbEditor(EffectManager *effectManager)
{
    ImGui::BeginChild("combos", ImVec2(150, 50));
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, reverbPresetNames, 13))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }

    auto type = static_cast<int>(effectManager->geteffectpar(ReverbPresets::Type));
    ImGui::PushItemWidth(100);
    if (PresetSelection("Type", type, reverbTypeNames, reverbTypeNameCount))
    {
        effectManager->seteffectpar(ReverbPresets::Type, static_cast<unsigned char>(type));
    }
    ImGui::EndChild();

    ImGui::SameLine();

    auto roomSize = static_cast<float>(effectManager->geteffectpar(ReverbPresets::RoomSize));
    if (ImGui::Knob("R.S.", &roomSize, 0, 128, ImVec2(30, 30)))
    {
        effectManager->seteffectpar(ReverbPresets::RoomSize, static_cast<unsigned char>(roomSize));
    }
    ImGui::ShowTooltipOnHover("Room Size");

    auto volume = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbVolume));
    if (ImGui::Knob("Vol", &volume, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbVolume, static_cast<unsigned char>(volume));
    }
    ImGui::ShowTooltipOnHover("Effect Volume");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto time = static_cast<float>(effectManager->geteffectpar(ReverbPresets::Time));
    if (ImGui::Knob("Time", &time, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::Time, static_cast<unsigned char>(time));
    }
    ImGui::ShowTooltipOnHover("Duration of Effect");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto initialDelay = static_cast<float>(effectManager->geteffectpar(ReverbPresets::InitialDelay));
    if (ImGui::Knob("I.Del.", &initialDelay, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::InitialDelay, static_cast<unsigned char>(initialDelay));
    }
    ImGui::ShowTooltipOnHover("Initial Delay");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto initialDelayFeedback = static_cast<float>(effectManager->geteffectpar(ReverbPresets::InitialDelayFeedback));
    if (ImGui::Knob("I.Del.Fb.", &initialDelayFeedback, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::InitialDelayFeedback, static_cast<unsigned char>(initialDelayFeedback));
    }
    ImGui::ShowTooltipOnHover("Initial Delay Feedback");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto lpf = static_cast<float>(effectManager->geteffectpar(ReverbPresets::LowPassFilter));
    if (ImGui::Knob("LPF", &lpf, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::LowPassFilter, static_cast<unsigned char>(lpf));
    }
    ImGui::ShowTooltipOnHover("Low Pass Filter");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto hpf = static_cast<float>(effectManager->geteffectpar(ReverbPresets::HighPassFilter));
    if (ImGui::Knob("HPF", &hpf, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::HighPassFilter, static_cast<unsigned char>(hpf));
    }
    ImGui::ShowTooltipOnHover("High Pass Filter");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto dampening = static_cast<float>(effectManager->geteffectpar(ReverbPresets::ReverbDampening));
    if (ImGui::Knob("Damp.", &dampening, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(ReverbPresets::ReverbDampening, static_cast<unsigned char>(dampening));
    }
    ImGui::ShowTooltipOnHover("Dampening");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
}

enum EchoPresets
{
    EchoVolume = 0,
    EchoPanning = 1,
    Delay = 2,
    DelayBetweenLR = 3,
    LRCrossover = 4,
    Feedback = 5,
    EchoDampening = 6,
};

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
    ImGui::Spacing();
    ImGui::SameLine();

    auto pan = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoPanning));
    if (ImGui::Knob("Pan", &pan, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoPanning, static_cast<unsigned char>(pan));
    }
    ImGui::ShowTooltipOnHover("Panning");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto delay = static_cast<float>(effectManager->geteffectpar(EchoPresets::Delay));
    if (ImGui::Knob("Delay", &delay, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::Delay, static_cast<unsigned char>(delay));
    }
    ImGui::ShowTooltipOnHover("Delay");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto lrDelay = static_cast<float>(effectManager->geteffectpar(EchoPresets::DelayBetweenLR));
    if (ImGui::Knob("LRdl.", &lrDelay, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::DelayBetweenLR, static_cast<unsigned char>(lrDelay));
    }
    ImGui::ShowTooltipOnHover("Delay Between L/R");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto lrCrossover = static_cast<float>(effectManager->geteffectpar(EchoPresets::LRCrossover));
    if (ImGui::Knob("LRc.", &lrCrossover, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::LRCrossover, static_cast<unsigned char>(lrCrossover));
    }
    ImGui::ShowTooltipOnHover("L/R Crossover");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto feedback = static_cast<float>(effectManager->geteffectpar(EchoPresets::Feedback));
    if (ImGui::Knob("Fb.", &feedback, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::Feedback, static_cast<unsigned char>(feedback));
    }
    ImGui::ShowTooltipOnHover("Feedback");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();

    auto dampening = static_cast<float>(effectManager->geteffectpar(EchoPresets::EchoDampening));
    if (ImGui::Knob("Damp.", &dampening, 0, 128, ImVec2(40, 40)))
    {
        effectManager->seteffectpar(EchoPresets::EchoDampening, static_cast<unsigned char>(dampening));
    }
    ImGui::ShowTooltipOnHover("Dampening");

    ImGui::SameLine();
    ImGui::Spacing();
    ImGui::SameLine();
}

void AppThreeDee::EffectChorusEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, chorusPresetNames, 10))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}

void AppThreeDee::EffectPhaserEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, phaserPresetNames, 12))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}

void AppThreeDee::EffectAlienWahEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, alienWahPresetNames, 4))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}

void AppThreeDee::EffectDistortionEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    ImGui::PushItemWidth(100);
    if (PresetSelection("Preset", preset, distortionPresetNames, 6))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
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
}
