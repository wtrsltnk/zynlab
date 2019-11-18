#include "app.tiny.h"
#include <iostream>

#include <glm/glm.hpp>
#include <imgui.h>
#include <imgui_knob.h>
#include <map>
#include <vector>
#include <zyn.nio/EngineManager.h>
#include <zyn.nio/RtMidi.h>

AppTiny::AppTiny()
{}

bool AppTiny::OnInit()
{
    glClearColor(0, 0.5f, 1, 1);

    midiout = new RtMidiOut(RtMidi::WINDOWS_MM);

    _selectedPort = -1;
    RefreshMidiChannels();

    return true;
}

void AppTiny::RefreshMidiChannels()
{
    _portNames.clear();

    _selectedPort = -1;
    for (unsigned int i = 0; i < midiout->getPortCount(); i++)
    {
        _portNames.push_back(midiout->getPortName(i));
    }
}

void AppTiny::OnTick(double /*timeInMs*/)
{
}

void AppTiny::OnRender()
{
}

void AppTiny::OnRenderUi()
{
    ImGui::Begin("Midi", nullptr, ImVec2(400, 400));

    if (ImGui::BeginChild("MIDI ports"))
    {
        if (ImGui::Button("Refresh"))
        {
            RefreshMidiChannels();
        }

        for (int i = 0; i < _portNames.size(); i++)
        {
            if (ImGui::RadioButton(_portNames[i].c_str(), &_selectedPort, i))
            {
                midiout->closePort();
                midiout->openPort(i);
            }
        }
        std::vector<unsigned char> message(3);

        if (_selectedPort >= 0)
        {
            ImGui::Button("C4");
            if (ImGui::IsItemHovered())
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
            }
            if (ImGui::IsItemActive())
            {
                if (ImGui::IsMouseClicked(0))
                {
                    // Note On: 144, 64, 90
                    message[0] = 144;
                    message[1] = 64;
                    message[2] = 90;
                    midiout->sendMessage(&message);
                }
            }
            if (ImGui::IsMouseReleased(0))
            {
                // Note Off: 128, 64, 40
                message[0] = 128;
                message[1] = 64;
                message[2] = 40;
                midiout->sendMessage(&message);
            }
        }
    }
    ImGui::EndChild();

    ImGui::End();

    ImGui::Begin("Test");
    {
        if (ImGui::BeginChild("VCF", ImVec2(150, 110)))
        {
            ImGui::Text("VCF");

            static unsigned char peak = 0, cutoff = 0;
            if (ImGui::KnobUchar("Peak", &peak, 0, 127, ImVec2(40, 60), "VCF Peak"))
            {
            }

            ImGui::SameLine();

            if (ImGui::KnobUchar("Cutoff", &cutoff, 0, 127, ImVec2(40, 60), "VCF Cutoff"))
            {
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("LFO", ImVec2(150, 110)))
        {
            ImGui::Text("LFO");

            static unsigned char rate = 0, lfoint = 0;
            if (ImGui::KnobUchar("Rate", &rate, 0, 127, ImVec2(40, 60), "LFO Rate"))
            {
            }

            ImGui::SameLine();

            if (ImGui::KnobUchar("Int", &lfoint, 0, 127, ImVec2(40, 60), "LFO Int"))
            {
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (ImGui::BeginChild("VCO", ImVec2(200, 110)))
        {
            ImGui::Text("VCO");

            static unsigned char pitch1 = 0, pitch2 = 0, pitch3 = 0;
            if (ImGui::KnobUchar("Pitch 1", &pitch1, 0, 127, ImVec2(40, 60), "Pitch for VCO 1"))
            {
            }

            ImGui::SameLine();

            if (ImGui::KnobUchar("Pitch 2", &pitch2, 0, 127, ImVec2(40, 60), "Pitch for VCO 2"))
            {
            }

            ImGui::SameLine();

            if (ImGui::KnobUchar("Pitch 3", &pitch3, 0, 127, ImVec2(40, 60), "Pitch for VCO 3"))
            {
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void AppTiny::OnDestroy()
{
    midiout->closePort();
    delete midiout;
    midiout = nullptr;
}

void AppTiny::OnKeyAction(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{
}

void AppTiny::OnResize(int width, int height)
{
    glViewport(0, 0, width, height);
}
