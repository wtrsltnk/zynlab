#include "instrumentspanel.h"

#include "patterneditor.h"
#include <algorithm>
#include <imgui.h>
#include <zyn.common/globals.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.mixer/Track.h>

InstrumentsPanel::InstrumentsPanel()
    : _session(nullptr)
{
}

void InstrumentsPanel::SetUp(
    ApplicationSession *session)
{
    _session = session;
}

void InstrumentsPanel::Render2d()
{
    if (_session == nullptr)
    {
        return;
    }

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(
        "Instruments",
        nullptr,
        flags);
    {
        if (ImGui::CollapsingHeader("Instrument Tracks", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BeginChild("TracksContainer", ImVec2(0, 300));
            for (unsigned int i = 0; i < _session->_mixer->GetTrackCount(); i++)
            {
                auto track = _session->_mixer->GetTrack(i);
                ImGui::PushID(i);

                bool v = track->Penabled == 1;
                if (ImGui::Checkbox("##enabled", &v))
                {
                    track->Penabled = v ? 1 : 0;
                }
                if (ImGui::IsItemClicked())
                {
                    _session->_mixer->State.currentTrack = i;
                }
                ImGui::SameLine();

                char buf[256] = {0};
                if (track->Pname[0] == 0)
                {
                    sprintf_s(buf, 256, "%02d : <default>", int(i + 1));
                }
                else
                {
                    sprintf_s(buf, 256, "%02d : %s", int(i + 1), track->Pname);
                }
                ImGui::Selectable(buf, i == _session->_mixer->State.currentTrack);
                if (ImGui::IsItemClicked())
                {
                    _session->_mixer->State.currentTrack = i;
                }

                ImGui::PopID();
            }
            ImGui::EndChild();

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
                {
                    if (_session->_mixer->State.currentTrack > 0)
                    {
                        _session->_mixer->State.currentTrack--;
                    }
                }
                if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
                {
                    if (_session->_mixer->State.currentTrack < _session->_mixer->GetTrackCount() - 1)
                    {
                        _session->_mixer->State.currentTrack++;
                    }
                }
            }
        }

        if (ImGui::CollapsingHeader("Instrument Properties"))
        {
            ImGui::BeginChild("InstrumentProperties", ImVec2(0, 200));
            if (_session->_mixer->State.currentTrack < _session->_mixer->GetTrackCount())
            {
                auto track = _session->_mixer->GetTrack(_session->_mixer->State.currentTrack);

                bool v = track->Penabled == 1;
                if (ImGui::Checkbox("Enabled", &v))
                {
                    track->Penabled = v ? 1 : 0;
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(80);

                int midiChannel = track->Prcvchn;
                if (ImGui::SliderInt(
                        "MIDI Channel",
                        &midiChannel,
                        1, NUM_MIDI_CHANNELS))
                {
                    track->Prcvchn = midiChannel;
                }

                ImGui::InputText(
                    "Name",
                    (char *)(track->Pname),
                    IM_ARRAYSIZE(track->Pname));
                ImGui::Text(
                    "By : %s", (track->info.Pauthor));
                ImGui::TextWrapped(
                    "%s", track->info.Pcomments);
            }
            ImGui::EndChild();
        }

        if (ImGui::CollapsingHeader("Instrument Library"))
        {
            // todo : dont do this every frame
            auto libs = _session->_library->GetTopLevelLibraries();
            std::vector<ILibrary *> slibs(libs.begin(), libs.end());
            std::sort(slibs.begin(), slibs.end(), [](ILibrary *a, ILibrary *b) {
                return a->GetName() < b->GetName();
            });
            for (auto topLevel : slibs)
            {
                auto selection = LibraryTree(topLevel);
                if (selection != nullptr)
                {
                    auto const &track = _session->_mixer->GetTrack(_session->_mixer->State.currentTrack);
                    track->Lock();
                    _session->_library->LoadAsInstrument(selection, track);
                    track->Penabled = 1;
                    track->ApplyParameters();
                    track->Unlock();

                    _session->_mixer->PreviewNote(track->Prcvchn, NOTE_C5, 400);

                    ImGui::SetWindowFocus(PatternEditor::ID);
                }
            }
        }
    }
    ImGui::End();
}

ILibraryItem *InstrumentsPanel::LibraryTree(
    ILibrary *library)
{
    ILibraryItem *result = nullptr;

    if (library->GetChildren().empty() && library->GetItems().empty())
    {
        if (ImGui::TreeNodeEx(library->GetName().c_str(), ImGuiTreeNodeFlags_Leaf))
        {
            ImGui::TreePop();
        }
    }
    else
    {
        if (ImGui::TreeNode(library->GetName().c_str()))
        {
            auto &s1 = library->GetChildren();
            std::vector<ILibrary *> children(s1.begin(), s1.end());
            std::sort(children.begin(), children.end(), [](ILibrary const *a, ILibrary const *b) -> bool {
                return a->GetName() < b->GetName();
            });

            for (auto level : children)
            {
                auto tmp = LibraryTree(level);
                if (tmp != nullptr && result == nullptr)
                {
                    result = tmp;
                }
            }

            auto &s2 = library->GetItems();
            std::vector<ILibraryItem *> items(s2.begin(), s2.end());
            std::sort(items.begin(), items.end(), [](ILibraryItem const *a, ILibraryItem const *b) -> bool {
                return a->GetName() < b->GetName();
            });
            for (auto item : items)
            {
                if (ImGui::TreeNodeEx(item->GetName().c_str(), ImGuiTreeNodeFlags_Leaf))
                {
                    ImGui::TreePop();
                }

                if (ImGui::IsItemClicked(0))
                {
                    result = item;
                }
            }
            ImGui::TreePop();
        }
    }

    return result;
}
