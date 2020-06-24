#include "syntheditor.h"

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"
#include "imgui_helpers.h"
#include <imgui.h>
#include <zyn.synth/ADnoteGlobalParam.h>
#include <zyn.synth/ADnoteParams.h>

char const *SynthEditor::ID = "SynthEditor";

unsigned char min = 0, max = 127;

SynthEditor::SynthEditor()
{}

void SynthEditor::SetUp(ApplicationSession *session)
{
    _session = session;

    _sampleNoteEditor.SetUp(_session);
}

bool CheckButton(const char *label, bool *p_value, ImVec2 const &size)
{
    bool result = false;
    auto &s = ImGui::GetStyle();

    ImGui::PushStyleColor(ImGuiCol_Button, (*p_value) ? s.Colors[ImGuiCol_ButtonActive] : s.Colors[ImGuiCol_Button]);

    if (ImGui::Button(label, size))
    {
        (*p_value) = !(*p_value);
        result = true;
    }

    ImGui::PopStyleColor();

    return result;
}

enum class ActiveSynth
{
    Add,
    Sub,
    Pad,
    Smpl,
};

int getpointx(
    EnvelopeParams *env,
    int n,
    int w)
{
    int lx = w - 20;
    int npoints = env->Penvpoints;

    float sum = 0;
    for (int i = 1; i < npoints; i++)
        sum += env->getdt(i) + 1;

    float sumbefore = 0; //the sum of all points before the computed point
    for (int i = 1; i <= n; i++)
        sumbefore += env->getdt(i) + 1;

    return ((int)(sumbefore / (float)sum * lx));
}

int getpointy(
    EnvelopeParams *env,
    int n,
    int h)
{
    int ly = h - 20;

    return ((int)((1.0 - env->Penvval[n] / 127.0) * ly));
}

static int lfo_type_count = 7;
static char const *lfo_types[] = {
    "SINE",
    "TRI",
    "SQR",
    "R.up",
    "R.dn",
    "E1dn",
    "E2dn",
};
static char const *lfo_icons[] = {
    ICON_FAD_MODSINE,
    ICON_FAD_MODTRI,
    ICON_FAD_MODSQUARE,
    ICON_FAD_MODSAWUP,
    ICON_FAD_MODSAWDOWN,
    ICON_FAD_MODSINE,
    ICON_FAD_MODSINE,
};

void SynthEditor::RenderLfo(
    LFOParams *lfo,
    unsigned char *enabled)
{
    if (lfo == nullptr)
    {
        return;
    }

    auto plotSize = ImVec2(200, 80);

    ImGui::BeginChild("LFO", ImVec2(plotSize.x + plotSize.y, plotSize.y + ImGui::GetTextLineHeightWithSpacing()));
    {
        bool envelopeEnabled = enabled == nullptr || (*enabled) == 1;

        if (enabled != nullptr)
        {
            if (ImGui::Checkbox("##LfoEnabled", &envelopeEnabled))
            {
                (*enabled) = envelopeEnabled ? 1 : 0;
            }

            ImGui::SameLine();
        }

        ImGui::Text("LFO");

        ImGui::BeginChild("Wave", ImVec2(100, 200));
        {
            for (int i = 0; i < lfo_type_count; i++)
            {
                char label[64] = {0};
                sprintf_s(label, 64, "%s %s", lfo_icons[i], lfo_types[i]);
                if (ImGui::RadioButton(label, lfo->Pkind == i))
                {
                    lfo->Pkind = i;
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("lfovalues", ImVec2(200, 200));
        {
            ImGui::Text("Frequency");
            ImGui::SameLine(60);
            ImGui::SliderScalar("##freq", ImGuiDataType_U8, &(lfo->Pfreq), &min, &max);
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

void SynthEditor::RenderEnvelope(
    EnvelopeParams *envelope,
    unsigned char *enabled)
{
    if (envelope == nullptr)
    {
        return;
    }

    auto plotSize = ImVec2(200, 80);

    ImGui::BeginChild("Envelope", ImVec2(plotSize.x + plotSize.y, plotSize.y + ImGui::GetTextLineHeightWithSpacing()));
    {
        bool envelopeEnabled = enabled == nullptr || (*enabled) == 1;

        if (enabled != nullptr)
        {
            if (ImGui::Checkbox("##EnvelopeEnabled", &envelopeEnabled))
            {
                (*enabled) = envelopeEnabled ? 1 : 0;
            }

            ImGui::SameLine();
        }

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
            EnvelopeParams::ConvertToFree(envelope);

            ImVec2 start = {
                x + getpointx(envelope, 0, plotSize.x),
                y + getpointy(envelope, 0, plotSize.y),
            };
            for (int i = 1; i < envelope->Penvpoints; i++)
            {
                auto xx = getpointx(envelope, i, plotSize.x);
                auto yy = getpointy(envelope, i, plotSize.y);
                ImVec2 end = {
                    x + xx,
                    y + yy,
                };

                draw_list->AddLine(start, end, col, th);
                start = end;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("sliders", ImVec2(plotSize.y, plotSize.y));
        {
            int slideWidth = 80;
            if (envelope->Envmode >= 3)
            {
                ImGui::Text("A.val");
                ImGui::SameLine(40);
                ImGui::SetNextItemWidth(slideWidth);
                ImGui::SliderScalar("##a.val", ImGuiDataType_U8, &(envelope->PA_val), &min, &max);
            }

            ImGui::Text("A.dt");
            ImGui::SameLine(40);
            ImGui::SetNextItemWidth(slideWidth);
            ImGui::SliderScalar("##A.dt", ImGuiDataType_U8, &(envelope->PA_dt), &min, &max);

            if (envelope->Envmode == 4)
            {
                ImGui::Text("D.val");
                ImGui::SameLine(40);
                ImGui::SetNextItemWidth(slideWidth);
                ImGui::SliderScalar("##D.val", ImGuiDataType_U8, &(envelope->PD_val), &min, &max);
            }

            if (envelope->Envmode != 3 && envelope->Envmode != 5)
            {
                ImGui::Text("D.dt");
                ImGui::SameLine(40);
                ImGui::SetNextItemWidth(slideWidth);
                ImGui::SliderScalar("##D.dt", ImGuiDataType_U8, &(envelope->PD_dt), &min, &max);
            }

            if (envelope->Envmode < 3)
            {
                ImGui::Text("S.val");
                ImGui::SameLine(40);
                ImGui::SetNextItemWidth(slideWidth);
                ImGui::SliderScalar("##S.val", ImGuiDataType_U8, &(envelope->PS_val), &min, &max);
            }

            ImGui::Text("R.dt");
            ImGui::SameLine(40);
            ImGui::SetNextItemWidth(slideWidth);
            ImGui::SliderScalar("##R.dt", ImGuiDataType_U8, &(envelope->PR_dt), &min, &max);

            if (envelope->Envmode >= 3)
            {
                ImGui::Text("R.val");
                ImGui::SameLine(40);
                ImGui::SetNextItemWidth(slideWidth);
                ImGui::SliderScalar("##R.val", ImGuiDataType_U8, &(envelope->PR_val), &min, &max);
            }
            auto forcedRelease = envelope->Pforcedrelease != 0;
            if (ImGui::Checkbox("frcR.", &forcedRelease))
            {
                envelope->Pforcedrelease = forcedRelease ? 1 : 0;
            }
        }
        ImGui::EndChild();
    }
    ImGui::EndChild();
}

void SynthEditor::Render2d()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    static ActiveSynth activeSynth = ActiveSynth::Add;
    static unsigned int activeVoice = NUM_VOICES;

    ImGui::Begin(
        ID,
        nullptr,
        flags);
    {
        auto track = _session->_mixer->GetTrack(_session->currentTrack);
        ImGui::BeginChild("btns", ImVec2(121, 0));
        {
            bool b;

            bool drumKitChecked = track->Pdrummode == 1;
            if (ImGui::Checkbox("DRUMMODE", &drumKitChecked))
            {
                track->Pdrummode = drumKitChecked ? 1 : 0;
            }

            if (drumKitChecked)
            {
                ImGui::Text("Drumkit instruments");
                for (unsigned int i = 0; i < NUM_TRACK_INSTRUMENTS; i++)
                {
                    if (i % 4 != 0)
                    {
                        ImGui::SameLine();
                    }
                    char title[8] = {0};
                    sprintf_s(title, 8, "%u", i + 1);
                    bool active = (_session->currentTrackInstrument == i);
                    if (CheckButton(title, &active, ImVec2(24, 24)))
                    {
                        activeSynth = ActiveSynth::Add;
                        _session->currentTrackInstrument = i;
                    }
                }

                ImGui::Separator();
            }

            auto instrument = track->Instruments[_session->currentTrackInstrument];
            bool addChecked = instrument.Padenabled;
            if (ImGui::Checkbox("##ADD_CHECK", &addChecked))
            {
                instrument.Padenabled = addChecked ? 1 : 0;
            }
            ImGui::SameLine();

            b = (activeSynth == ActiveSynth::Add);
            if (CheckButton("ADD", &b, ImVec2(-1, 0)))
            {
                activeSynth = ActiveSynth::Add;
            }

            bool voiceActive = (activeVoice >= NUM_VOICES);
            if (CheckButton("GLOBAL", &voiceActive, ImVec2(-1, 0)))
            {
                activeSynth = ActiveSynth::Add;
                activeVoice = NUM_VOICES;
            }

            for (unsigned int i = 0; i < NUM_VOICES; i++)
            {
                if (i % 4 != 0)
                {
                    ImGui::SameLine();
                }
                char title[8] = {0};
                sprintf_s(title, 8, "%u", i + 1);
                voiceActive = (activeVoice == i);
                if (CheckButton(title, &voiceActive, ImVec2(24, 24)))
                {
                    activeSynth = ActiveSynth::Add;
                    activeVoice = i;
                }
            }

            ImGui::Separator();

            bool subChecked = instrument.Psubenabled;
            if (ImGui::Checkbox("##SUB_CHECK", &subChecked))
            {
                instrument.Psubenabled = subChecked ? 1 : 0;
            }
            ImGui::SameLine();
            b = (activeSynth == ActiveSynth::Sub);
            if (CheckButton("SUB", &b, ImVec2(-1, 0)))
            {
                activeSynth = ActiveSynth::Sub;
            }

            ImGui::Separator();

            bool padChecked = instrument.Ppadenabled;
            if (ImGui::Checkbox("##PAD_CHECK", &padChecked))
            {
                instrument.Ppadenabled = padChecked ? 1 : 0;
            }
            ImGui::SameLine();
            b = (activeSynth == ActiveSynth::Pad);
            if (CheckButton("PAD", &b, ImVec2(-1, 0)))
            {
                activeSynth = ActiveSynth::Pad;
            }

            ImGui::Separator();

            bool smplChecked = instrument.Psmplenabled;
            if (ImGui::Checkbox("##SMPL_CHECK", &smplChecked))
            {
                instrument.Psmplenabled = smplChecked ? 1 : 0;
            }
            ImGui::SameLine();
            b = (activeSynth == ActiveSynth::Smpl);
            if (CheckButton("SMPL", &b, ImVec2(-1, 0)))
            {
                activeSynth = ActiveSynth::Smpl;
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        if (activeSynth == ActiveSynth::Add)
        {
            ADnoteParameters *addparams = track->Instruments[_session->currentTrackInstrument].adpars;

            ImGui::BeginChild("add-synth");
            {
                if (activeVoice >= NUM_VOICES)
                {
                    if (ImGui::CollapsingHeader("AMPLITUDE"))
                    {
                        ImGui::PushID("AMPLITUDDE");
                        RenderEnvelope(addparams->AmpEnvelope, nullptr);
                        ImGui::SameLine();
                        RenderLfo(addparams->AmpLfo, nullptr);
                        ImGui::PopID();
                    }
                    if (ImGui::CollapsingHeader("FILTER"))
                    {
                        ImGui::PushID("FILTER");
                        RenderEnvelope(addparams->FilterEnvelope, nullptr);
                        ImGui::SameLine();
                        RenderLfo(addparams->FilterLfo, nullptr);
                        ImGui::PopID();
                    }
                    if (ImGui::CollapsingHeader("FREQUENCY"))
                    {
                        ImGui::PushID("FREQUENCY");
                        RenderEnvelope(addparams->FreqEnvelope, nullptr);
                        ImGui::SameLine();
                        RenderLfo(addparams->FreqLfo, nullptr);
                        ImGui::PopID();
                    }
                }
                ImGui::Text("test add synth");
            }
            ImGui::EndChild();
        }
        else if (activeSynth == ActiveSynth::Smpl)
        {
            _sampleNoteEditor.Render2d();
        }
    }

    ImGui::End();
}
