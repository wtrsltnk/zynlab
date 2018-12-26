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
    ImGui::PushItemWidth(100);
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

void AppThreeDee::EffectReverbEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    if (PresetSelection("Preset", preset, reverbPresetNames, 13))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}

void AppThreeDee::EffectEchoEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    if (PresetSelection("Preset", preset, echoPresetNames, 9))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}

void AppThreeDee::EffectChorusEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    if (PresetSelection("Preset", preset, chorusPresetNames, 10))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}

void AppThreeDee::EffectPhaserEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    if (PresetSelection("Preset", preset, phaserPresetNames, 12))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}

void AppThreeDee::EffectAlienWahEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
    if (PresetSelection("Preset", preset, alienWahPresetNames, 4))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}

void AppThreeDee::EffectDistortionEditor(EffectManager *effectManager)
{
    auto preset = static_cast<int>(effectManager->getpreset());
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
    if (PresetSelection("Preset", preset, dynFilterPresetNames, 5))
    {
        effectManager->changepreset(static_cast<unsigned char>(preset));
    }
}
