#include "piano.h"

#include <imgui.h>
#include <zyn.nio/MidiInputManager.h>

Piano::Piano() = default;

void HitKey(
    int octave,
    int key)
{
    MidiEvent ev;
    ev.type = MidiEventTypes::M_NOTE;
    ev.channel = 1;
    ev.num = (octave * 12) + key;
    ev.value = 100;
    MidiInputManager::Instance().PutEvent(ev);
}

void Piano::Render2d()
{
    //show Main Window
    ImGui::ShowDemoWindow();

    ImGui::Begin("Piano", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
    int n = 0;

    // white keys first
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.4f, 1.0f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.4f, 1.0f, 1.0f, 1.0f));
    ImVec2 note(10, 30), btnSize(40, 220);
    for (int i = 0; i < 28; i++)
    {
        ImGui::SetCursorPosX(note.x);
        ImGui::SetCursorPosY(note.y);
        ImGui::PushID(n++);
        if (ImGui::Button("##test", btnSize))
        {
            auto octave = i / 7;
            auto octaveNote = i % 7;
            switch (octaveNote)
            {
                case 0: // A
                {
                    HitKey(octave, 0);
                    break;
                }
                case 1: // B
                {
                    HitKey(octave, 2);
                    break;
                }
                case 2: // C
                {
                    HitKey(octave, 3);
                    break;
                }
                case 3: // D
                {
                    HitKey(octave, 5);
                    break;
                }
                case 4: // E
                {
                    HitKey(octave, 7);
                    break;
                }
                case 5: // F
                {
                    HitKey(octave, 8);
                    break;
                }
                case 6: // G
                {
                    HitKey(octave, 10);
                    break;
                }
            }
        }
        ImGui::SetItemAllowOverlap();
        ImGui::PopID();

        note.x += 42;
    }
    ImGui::PopStyleColor(3);

    //black keys
    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.1f, 0.0f, 0.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.1f, 0.0f, 0.0f, 1.0f));
    ImVec2 note1 = ImVec2(35, 30);
    ImVec2 btnSize1 = ImVec2(30, 140);
    for (int i = 0; i < 28; i++)
    {
        if (i % 7 == 0 || i % 7 == 2 || i % 7 == 3 || i % 7 == 5 || i % 7 == 6)
        {
            ImGui::SetCursorPosX(note1.x);
            ImGui::SetCursorPosY(note1.y);
            ImGui::PushID(n++);
            if (ImGui::Button("##test", btnSize1))
            {
                auto octave = i / 7;
                auto octaveNote = i % 7;
                switch (octaveNote)
                {
                    case 0: // A#
                    {
                        HitKey(octave, 1);
                        break;
                    }
                    case 2: // C#
                    {
                        HitKey(octave, 4);
                        break;
                    }
                    case 3: // D#
                    {
                        HitKey(octave, 6);
                        break;
                    }
                    case 4: // F#
                    {
                        HitKey(octave, 9);
                        break;
                    }
                    case 5: // G#
                    {
                        HitKey(octave, 11);
                        break;
                    }
                }
            }
            ImGui::PopID();
        }
        note1.x += 42;
    }
    ImGui::PopStyleColor(2);

    ImGui::End();
}
