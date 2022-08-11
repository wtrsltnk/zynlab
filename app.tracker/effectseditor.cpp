#include "effectseditor.h"

#include <imgui.h>
#include <sstream>
#include <zyn.fx/EffectMgr.h>
#include <zyn.mixer/Track.h>

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"

EffectsEditor::EffectsEditor()
    : _session(nullptr)
{}

void EffectsEditor::SetUp(
    ApplicationSession *session)
{
    _session = session;
}

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

namespace ImGui
{
    void ShowTooltipOnHover(
        char const *tooltip);
}

void EffectsEditor::Render2d()
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
            for (int e = 0; e < NUM_TRACK_EFX; e++)
            {
                if (e > 0) ImGui::SameLine();
                ImGui::PushID(e);

                int item_current = track->partefx[e]->geteffect();
                ImGui::BeginChild("fx", ImVec2(item_current == 0 ? 120 : 320, 120), true);
                {
                    ImGui::SetNextItemWidth(100);
                    if (ImGui::Combo("##effect", &item_current, EffectNames, IM_ARRAYSIZE(EffectNames)))
                    {
                        track->partefx[e]->changeeffect(item_current);
                    }

                    if (item_current == 0)
                    {
                        ImGui::EndChild();
                        ImGui::PopID();
                        continue;
                    }

                    ImGui::SameLine();

                    int preset_current = track->partefx[e]->getpreset();
                    ImGui::SetNextItemWidth(150);
                    if (ImGui::Combo("preset", &preset_current, reverbPresetNames, IM_ARRAYSIZE(reverbPresetNames)))
                    {
                        track->partefx[e]->changepreset(preset_current);
                    }
                }
                ImGui::EndChild();
                ImGui::PopID();
            }
            ImGui::EndGroup();
        }

        if (selectedTab == 1)
        {
            static ParamIndices selectedParams = ParamIndices::None;

            ImGui::BeginChild("params", ImVec2(260, 0), true);
            if (ImGui::CollapsingHeader("Mixer"))
            {
                if (ImGui::Selectable("Volume", selectedParams == ParamIndices::Mixer_Volume))
                {
                    selectedParams = ParamIndices::Mixer_Volume;
                }
                if (ImGui::Selectable("Panning", selectedParams == ParamIndices::Mixer_Panning))
                {
                    selectedParams = ParamIndices::Mixer_Panning;
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
                        if (ImGui::Selectable(paramName.str().c_str(), selectedParams == paramIndex))
                        {
                            selectedParams = paramIndex;
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
                        RenderEffect(ParamIndices::InsertFX_1 + (100 * i), i, _session->_mixer->GetInsertEffectType(i), selectedParams);
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
                        RenderEffect(ParamIndices::TrackFX_1 + (100 * i), i, track->partefx[i]->geteffect(), selectedParams);
                    }
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();
        }
    }
    ImGui::End();
}

void EffectsEditor::RenderEffect(
    int group,
    int index,
    int effect,
    ParamIndices &selectedParams)
{
    if (ImGui::TreeNode(EffectNames[effect]))
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
