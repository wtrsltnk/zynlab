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
            }

            auto showTrackFx = track->partefx[0]->geteffect() > 0 || track->partefx[1]->geteffect() > 0 || track->partefx[2]->geteffect() > 0;

            if (showTrackFx && ImGui::CollapsingHeader("Track FX"))
            {
                for (int i = 0; i < 3; i++)
                {
                    auto effect = track->partefx[i]->geteffect();
                    if (effect > 0)
                    {
                        if (ImGui::TreeNode(EffectNames[effect]))
                        {
                            if (effect == 1) // Reverb
                            {
                                if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_Volume) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_Volume) + i * 100);
                                }

                                if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_Pan) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_Pan) + i * 100);
                                }

                                if (ImGui::Selectable("Time", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_Time) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_Time) + i * 100);
                                }

                                if (ImGui::Selectable("Initial delay", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_InitialDelay) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_InitialDelay) + i * 100);
                                }

                                if (ImGui::Selectable("Initial delay feedback", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_InitialDelayFeedback) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_InitialDelayFeedback) + i * 100);
                                }

                                if (ImGui::Selectable("LowPass", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_LowPass) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_LowPass) + i * 100);
                                }

                                if (ImGui::Selectable("HighPass", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_HighPass) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Reverb_HighPass) + i * 100);
                                }
                            }

                            if (effect == 2) // Echo
                            {
                                if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_Volume) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_Volume) + i * 100);
                                }

                                if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_Pan) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_Pan) + i * 100);
                                }

                                if (ImGui::Selectable("LeftRightDelay", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_LeftRightDelay) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_LeftRightDelay) + i * 100);
                                }

                                if (ImGui::Selectable("CrossingDelay", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_CrossingDelay) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_CrossingDelay) + i * 100);
                                }

                                if (ImGui::Selectable("Feedback", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_Feedback) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_Feedback) + i * 100);
                                }

                                if (ImGui::Selectable("Damp", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_Damp) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Echo_Damp) + i * 100);
                                }
                            }

                            if (effect == 3) // Chorus
                            {
                                if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Volume) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Volume) + i * 100);
                                }

                                if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Pan) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Pan) + i * 100);
                                }

                                if (ImGui::Selectable("LFOFrequency", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_LFOFrequency) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_LFOFrequency) + i * 100);
                                }

                                if (ImGui::Selectable("LFORandomness", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_LFORandomness) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_LFORandomness) + i * 100);
                                }

                                if (ImGui::Selectable("LeftRightPhaseDifference", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_LeftRightPhaseDifference) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_LeftRightPhaseDifference) + i * 100);
                                }

                                if (ImGui::Selectable("LFODepth", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_LFODepth) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_LFODepth) + i * 100);
                                }

                                if (ImGui::Selectable("Delay", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Delay) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Delay) + i * 100);
                                }

                                if (ImGui::Selectable("Feedback", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Feedback) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Feedback) + i * 100);
                                }

                                if (ImGui::Selectable("Subtract", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Subtract) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Chorus_Subtract) + i * 100);
                                }
                            }

                            if (effect == 4) // Phaser
                            {
                                if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Volume) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Volume) + i * 100);
                                }

                                if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Pan) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Pan) + i * 100);
                                }

                                if (ImGui::Selectable("LFOFrequency", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_LFOFrequency) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_LFOFrequency) + i * 100);
                                }

                                if (ImGui::Selectable("LFORandomness", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_LFORandomness) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_LFORandomness) + i * 100);
                                }

                                if (ImGui::Selectable("LeftRightPhaseDifference", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_LeftRightPhaseDifference) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_LeftRightPhaseDifference) + i * 100);
                                }

                                if (ImGui::Selectable("LFODepth", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_LFODepth) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_LFODepth) + i * 100);
                                }

                                if (ImGui::Selectable("Stages", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Stages) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Stages) + i * 100);
                                }

                                if (ImGui::Selectable("Feedback", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Feedback) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Feedback) + i * 100);
                                }

                                if (ImGui::Selectable("Subtract", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Subtract) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Phaser_Subtract) + i * 100);
                                }
                            }

                            if (effect == 5) // AlienWah
                            {
                                if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Volume) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Volume) + i * 100);
                                }

                                if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Pan) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Pan) + i * 100);
                                }

                                if (ImGui::Selectable("LFOFrequency", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_LFOFrequency) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_LFOFrequency) + i * 100);
                                }

                                if (ImGui::Selectable("LFORandomness", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_LFORandomness) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_LFORandomness) + i * 100);
                                }

                                if (ImGui::Selectable("LeftRightPhaseDifference", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_LeftRightPhaseDifference) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_LeftRightPhaseDifference) + i * 100);
                                }

                                if (ImGui::Selectable("LFODepth", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_LFODepth) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_LFODepth) + i * 100);
                                }

                                if (ImGui::Selectable("Delay", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Delay) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Delay) + i * 100);
                                }

                                if (ImGui::Selectable("Feedback", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Feedback) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Feedback) + i * 100);
                                }

                                if (ImGui::Selectable("Subtract", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Subtract) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Subtract) + i * 100);
                                }

                                if (ImGui::Selectable("Phase", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Phase) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_AlienWah_Phase) + i * 100);
                                }
                            }

                            if (effect == 6) // Distorsion
                            {
                                if (ImGui::Selectable("Volume", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_Volume) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_Volume) + i * 100);
                                }

                                if (ImGui::Selectable("Panning", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_Pan) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_Pan) + i * 100);
                                }

                                if (ImGui::Selectable("Drive", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_Drive) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_Drive) + i * 100);
                                }

                                if (ImGui::Selectable("Level", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_Level) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_Level) + i * 100);
                                }

                                if (ImGui::Selectable("LowPass", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_LowPass) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_LowPass) + i * 100);
                                }

                                if (ImGui::Selectable("HighPass", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_HighPass) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_HighPass) + i * 100);
                                }

                                if (ImGui::Selectable("DryWetMix", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_DryWetMix) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_Distorsion_DryWetMix) + i * 100);
                                }
                            }

                            if (effect == 7) // EQ
                            {
                                if (ImGui::Selectable("Gain", selectedParams == (ParamIndices)(int(ParamIndices::TrackFX_1_EQ_Gain) + i * 100)))
                                {
                                    selectedParams = (ParamIndices)(int(ParamIndices::TrackFX_1_EQ_Gain) + i * 100);
                                }
                            }

                            ImGui::TreePop();
                        }
                    }
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();
        }
    }
    ImGui::End();
}
