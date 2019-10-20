#include "app.tiny.h"
#include <iostream>

#include <imgui.h>
#include <imgui_knob.h>
#include <glm/glm.hpp>
#include <map>
#include <vector>

AppTiny::AppTiny()
{ }

bool AppTiny::OnInit()
{
    glClearColor(0, 0.5f, 1, 1);

    return true;
}

void AppTiny::OnTick(double /*timeInMs*/)
{
}

void AppTiny::OnRender()
{
}

void AppTiny::OnRenderUi()
{
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
}

void AppTiny::OnKeyAction(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{
}

void AppTiny::OnResize(int width, int height)
{
    glViewport(0, 0, width, height);
}
