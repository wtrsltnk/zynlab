#include "envelopeeditor.h"

#include "imgui_helpers.h"

EnvelopeEditor::EnvelopeEditor()
{
}

void EnvelopeEditor::Setup(
    EnvelopeParams *params)
{
    _params = params;
}

void EnvelopeEditor::Render2d()
{
    if (_params == nullptr)
    {
        return;
    }

    auto plotSize = ImVec2(200, 80);

    ImGui::BeginChild("Envelope", ImVec2(0, plotSize.y + 100));
    {
//        bool envelopeEnabled = enabled == nullptr || (*enabled) == 1;

//        if (enabled != nullptr)
//        {
//            if (ImGui::Checkbox("##EnvelopeEnabled", &envelopeEnabled))
//            {
//                (*enabled) = envelopeEnabled ? 1 : 0;
//            }

//            ImGui::SameLine();
//        }

        ImGui::Text("Envelope");

        ImGui::BeginChild("EnvelopePlot", plotSize);
        {
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            static ImVec4 colf = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
            const ImU32 col = ImColor(colf);
            static ImVec4 bgf = ImVec4(0.4f, 0.5f, 0.8f, 0.3f);
            const ImU32 bg = ImColor(bgf);
            auto th = 1.0f;
            const ImVec2 p = ImGui::GetCursorScreenPos();

            float x = p.x + 10, y = p.y + 10;

            draw_list->AddRectFilled(p, p + plotSize, bg);
            EnvelopeParams::ConvertToFree(_params);

            ImVec2 start = {
                x + getpointx(_params, 0, plotSize.x),
                y + getpointy(_params, 0, plotSize.y),
            };
            for (int i = 1; i < _params->Penvpoints; i++)
            {
                auto xx = getpointx(_params, i, plotSize.x);
                auto yy = getpointy(_params, i, plotSize.y);
                ImVec2 end = {
                    x + xx,
                    y + yy,
                };

                draw_list->AddLine(start, end, col, th);
                start = end;
            }
        }
        ImGui::EndChild();

//        if (enabled != nullptr)
//        {
//            if (ImGui::Checkbox("##EnvelopeEnabled", &envelopeEnabled))
//            {
//                (*enabled) = envelopeEnabled ? 1 : 0;
//            }
//            ShowTooltipOnHover("Enable this Envelope");

//            ImGui::SameLine();
//        }

        if (_params == nullptr)
        {
            return;
        }

//        if (!envelopeEnabled)
//        {
//            return;
//        }

        if (_params->Envmode >= 3)
        {
            if (KnobUchar("A.val", &(_params->PA_val), 0, 127, ImVec2(50, 40), "Starting value"))
            {
            }

            ImGui::SameLine();
        }

        if (KnobUchar("A.dt", &(_params->PA_dt), 0, 127, ImVec2(50, 40), "Attack time"))
        {
        }

        ImGui::SameLine();

        if (_params->Envmode == 4)
        {
            if (KnobUchar("D.val", &(_params->PD_val), 0, 127, ImVec2(50, 40), "Decay value"))
            {
            }

            ImGui::SameLine();
        }

        if (_params->Envmode != 3 && _params->Envmode != 5)
        {
            if (KnobUchar("D.dt", &(_params->PD_dt), 0, 127, ImVec2(50, 40), "Decay time"))
            {
            }

            ImGui::SameLine();
        }

        if (_params->Envmode < 3)
        {
            if (KnobUchar("S.val", &(_params->PS_val), 0, 127, ImVec2(50, 40), "Sustain value"))
            {
            }

            ImGui::SameLine();
        }

        if (KnobUchar("R.dt", &(_params->PR_dt), 0, 127, ImVec2(50, 40), "Release time"))
        {
        }

        ImGui::SameLine();

        if (_params->Envmode >= 3)
        {
            if (KnobUchar("R.val", &(_params->PR_val), 0, 127, ImVec2(50, 40), "Release value"))
            {
            }

            ImGui::SameLine();
        }

        if (KnobUchar("Str.", &(_params->Penvstretch), 0, 127, ImVec2(50, 40), "Envelope stretch (on lower notes makes the envelope longer)"))
        {
        }

        ImGui::SameLine();

        auto forcedRelease = _params->Pforcedrelease != 0;
        if (ImGui::Checkbox("frcR.", &forcedRelease))
        {
            _params->Pforcedrelease = forcedRelease ? 1 : 0;
        }
        ShowTooltipOnHover("Forced Release");
    }
    ImGui::EndChild();
}
