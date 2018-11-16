#include "../app.threedee.h"

#include <imgui.h>
#include <zyn.synth/EnvelopeParams.h>

void AppThreeDee::AmplitudeEnvelope(EnvelopeParams *envelope)
{
    ImGui::BeginChild("Amplitude Envelope", ImVec2(270, 80), true);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 190);
    ImGui::SetColumnWidth(1, 80);

    ImGui::Text("Amplitude Envelope");

    auto a_dt = static_cast<float>(envelope->PA_dt);
    if (MyKnob("A.dt", &(a_dt), 0, 128, ImVec2(30, 30)))
    {
        envelope->PA_dt = static_cast<unsigned char>(a_dt);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Attack time");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto d_dt = static_cast<float>(envelope->PD_dt);
    if (MyKnob("D.dt", &(d_dt), 0, 128, ImVec2(30, 30)))
    {
        envelope->PD_dt = static_cast<unsigned char>(d_dt);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Decay time");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto s_val = static_cast<float>(envelope->PS_val);
    if (MyKnob("S.val", &(s_val), 0, 128, ImVec2(30, 30)))
    {
        envelope->PS_val = static_cast<unsigned char>(s_val);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Sustain value");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto r_dt = static_cast<float>(envelope->PR_dt);
    if (MyKnob("R.dt", &(r_dt), 0, 128, ImVec2(30, 30)))
    {
        envelope->PR_dt = static_cast<unsigned char>(r_dt);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Release time");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto stretch = static_cast<float>(envelope->Penvstretch);
    if (MyKnob("Str.", &(stretch), 0, 128, ImVec2(20, 20)))
    {
        envelope->Penvstretch = static_cast<unsigned char>(stretch);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Envelope stretch (on lower notes makes the envelope longer)");
        ImGui::EndTooltip();
    }

    ImGui::NextColumn();

    auto linear = envelope->Plinearenvelope != 0;
    if (ImGui::Checkbox("##linear", &linear))
    {
        envelope->Plinearenvelope = linear ? 1 : 0;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("The envelope is linear");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    ImGui::Text("Lin.");

    auto forcedRelease = envelope->Pforcedrelease != 0;
    if (ImGui::Checkbox("##forcedrelease", &forcedRelease))
    {
        envelope->Pforcedrelease = forcedRelease ? 1 : 0;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Forced Release");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    ImGui::Text("frcR.");

    ImGui::EndChild();
}

void AppThreeDee::FrequencyEnvelope(EnvelopeParams *envelope) {

    ImGui::BeginChild("Frequency Envelope", ImVec2(270, 80), true);

    ImGui::Columns(2);
    ImGui::SetColumnWidth(0, 190);
    ImGui::SetColumnWidth(1, 80);

    ImGui::Text("Frequency Envelope");

    auto a_val = static_cast<float>(envelope->PA_val);
    if (MyKnob("A.val", &(a_val), 0, 128, ImVec2(30, 30)))
    {
        envelope->PA_val = static_cast<unsigned char>(a_val);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Starting value");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    auto a_dt = static_cast<float>(envelope->PA_dt);
    if (MyKnob("A.dt", &(a_dt), 0, 128, ImVec2(30, 30)))
    {
        envelope->PA_dt = static_cast<unsigned char>(a_dt);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Attack time");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    auto r_dt = static_cast<float>(envelope->PR_dt);
    if (MyKnob("R.dt", &(r_dt), 0, 128, ImVec2(30, 30)))
    {
        envelope->PR_dt = static_cast<unsigned char>(r_dt);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Release time");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    auto r_val = static_cast<float>(envelope->PR_val);
    if (MyKnob("R.val", &(r_val), 0, 128, ImVec2(30, 30)))
    {
        envelope->PR_val = static_cast<unsigned char>(r_val);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Release value");
        ImGui::EndTooltip();
    }

    ImGui::SameLine();

    auto stretch = static_cast<float>(envelope->Penvstretch);
    if (MyKnob("Str.", &(stretch), 0, 128, ImVec2(20, 20)))
    {
        envelope->Penvstretch = static_cast<unsigned char>(stretch);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Envelope stretch (on lower notes makes the envelope longer)");
        ImGui::EndTooltip();
    }

    ImGui::NextColumn();

    auto forcedRelease = envelope->Pforcedrelease != 0;
    if (ImGui::Checkbox("##forcedrelease", &forcedRelease))
    {
        envelope->Pforcedrelease = forcedRelease ? 1 : 0;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Forced Release");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    ImGui::Text("frcR.");

    ImGui::EndChild();
}

void AppThreeDee::FilterEnvelope(EnvelopeParams *envelope) {

    ImGui::BeginChild("Filter Envelope", ImVec2(320, 80), true);

    ImGui::Text("Filter Envelope");

    auto a_val = static_cast<float>(envelope->PA_val);
    if (MyKnob("A.val", &(a_val), 0, 128, ImVec2(30, 30)))
    {
        envelope->PA_val = static_cast<unsigned char>(a_val);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Starting value");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto a_dt = static_cast<float>(envelope->PA_dt);
    if (MyKnob("A.dt", &(a_dt), 0, 128, ImVec2(30, 30)))
    {
        envelope->PA_dt = static_cast<unsigned char>(a_dt);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Attack time");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto d_val = static_cast<float>(envelope->PD_val);
    if (MyKnob("D.val", &(d_val), 0, 128, ImVec2(30, 30)))
    {
        envelope->PD_val = static_cast<unsigned char>(d_val);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Decay value");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto d_dt = static_cast<float>(envelope->PD_dt);
    if (MyKnob("D.dt", &(d_dt), 0, 128, ImVec2(30, 30)))
    {
        envelope->PD_dt = static_cast<unsigned char>(d_dt);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Decay time");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto r_dt = static_cast<float>(envelope->PR_dt);
    if (MyKnob("R.dt", &(r_dt), 0, 128, ImVec2(30, 30)))
    {
        envelope->PR_dt = static_cast<unsigned char>(r_dt);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Release time");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto r_val = static_cast<float>(envelope->PR_val);
    if (MyKnob("R.val", &(r_val), 0, 128, ImVec2(30, 30)))
    {
        envelope->PR_val = static_cast<unsigned char>(r_val);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Release value");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();

    auto stretch = static_cast<float>(envelope->Penvstretch);
    if (MyKnob("Str.", &(stretch), 0, 128, ImVec2(20, 20)))
    {
        envelope->Penvstretch = static_cast<unsigned char>(stretch);
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Envelope stretch (on lower notes makes the envelope longer)");
        ImGui::EndTooltip();
    }

    auto forcedRelease = envelope->Pforcedrelease != 0;
    if (ImGui::Checkbox("##forcedrelease", &forcedRelease))
    {
        envelope->Pforcedrelease = forcedRelease ? 1 : 0;
    }
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("Forced Release");
        ImGui::EndTooltip();
    }
    ImGui::SameLine();
    ImGui::Text("frcR.");

    ImGui::EndChild();
}

void AppThreeDee::BandWidthEnvelope(EnvelopeParams *envelope) {}
