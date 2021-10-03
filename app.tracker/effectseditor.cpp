#include "effectseditor.h"

#include <imgui.h>
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

        if (selectedTab == 0)
        {
            ImGui::BeginGroup();
            auto track = _session->_mixer->GetTrack(_session->_mixer->State.currentTrack);
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
    }
    ImGui::End();
}
