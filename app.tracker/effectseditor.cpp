#include "effectseditor.h"

#include <imgui.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.mixer/Track.h>

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

void EffectsEditor::Render2d()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "Effects Editor",
        nullptr,
        flags);
    {
        auto track = _session->_mixer->GetTrack(_session->currentTrack);
        for (int e = 0; e < NUM_TRACK_EFX; e++)
        {
            if (e > 0) ImGui::SameLine();
            ImGui::PushID(e);

            int item_current = track->partefx[e]->geteffect();
            ImGui::BeginChild("fx", ImVec2(item_current == 0 ? 120 : 320, 120), true);

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

            ImGui::EndChild();
            ImGui::PopID();
        }
    }
    ImGui::End();
}
