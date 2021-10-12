#include "mixereditor.h"

#include <cmath>
#include <imgui.h>
#include <imgui_internal.h>
#include <iostream>
#include <zyn.mixer/Track.h>

char const *MixerEditor::ID = "MixerEditor";

#define TRACK_WIDTH 150

static ImVec2 trackSize = ImVec2(TRACK_WIDTH, 0);

namespace mixer_editor
{
    unsigned int EffectNameCount = 9;

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
} // namespace mixer_editor

MixerEditor::MixerEditor() = default;

void MixerEditor::SetUp(
    ApplicationSession *session)
{
    _session = session;
}

namespace ImGui
{
    ImVec2 operator+(ImVec2 const &a, ImVec2 const &b)
    {
        return ImVec2(a.x + b.x, a.y + b.y);
    }

    ImVec2 operator-(ImVec2 const &a, ImVec2 const &b)
    {
        return ImVec2(a.x - b.x, a.y - b.y);
    }

    ImVec4 operator*(ImVec4 const &a, ImVec4 const &b)
    {
        return ImVec4(a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w);
    }

    bool TextCentered(ImVec2 const &size, char const *label)
    {
        ImGuiStyle &style = ImGui::GetStyle();

        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        auto result = ImGui::InvisibleButton(label, size);

        auto textSize = CalcTextSize(label);
        draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

        return result;
    }

    void UvMeter(char const *label, ImVec2 const &size, int *value, int v_min, int v_max)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        ImVec2 pos = ImGui::GetCursorScreenPos();

        ImGui::InvisibleButton(label, size);

        float stepHeight = (v_max - v_min + 1) / size.y;
        auto y = pos.y + size.y;
        auto hue = 0.4f;
        auto sat = 0.6f;
        for (int i = v_min; i <= v_max; i += 5)
        {
            hue = 0.4f - (static_cast<float>(i) / static_cast<float>(v_max - v_min)) / 2.0f;
            sat = (*value < i ? 0.0f : 0.6f);
            draw_list->AddRectFilled(ImVec2(pos.x, y), ImVec2(pos.x + size.x, y - (stepHeight * 4)), static_cast<ImU32>(ImColor::HSV(hue, sat, 0.6f)));
            y = pos.y + size.y - (i * stepHeight);
        }
    }

    void ShowTooltipOnHover(
        char const *tooltip)
    {
        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::Text("%s", tooltip);
            ImGui::EndTooltip();
        }
    }

    void MoveCursorPos(
        ImVec2 delta)
    {
        SetCursorPos(GetCursorPos() + delta);
    }

    bool Fader(const char *label, const ImVec2 &size, int *v, const int v_min, const int v_max, const char *format, float power)
    {
        ImGuiDataType data_type = ImGuiDataType_S32;
        ImGuiWindow *window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        const ImGuiID id = window->GetID(label);

        const ImVec2 label_size = CalcTextSize(label, nullptr, true);
        const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
        const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

        ItemSize(bb, style.FramePadding.y);
        if (!ItemAdd(frame_bb, id))
            return false;

        IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
        if (format == nullptr)
            format = "%d";

        const bool hovered = ItemHoverable(frame_bb, id);
        if ((hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || g.NavInputId == id)
        {
            SetActiveID(id, window);
            SetFocusID(id, window);
            FocusWindow(window);
            //        g.ActiveIdAllowNavDirFlags = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        }

        // Draw frame
        const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
        RenderNavHighlight(frame_bb, id);
        RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

        // Slider behavior
        ImRect grab_bb;
        const bool value_changed = SliderBehavior(frame_bb, id, data_type, v, &v_min, &v_max, format, power, ImGuiSliderFlags_Vertical, &grab_bb);
        if (value_changed)
            MarkItemEdited(id);

        ImRect gutter;
        gutter.Min = grab_bb.Min;
        gutter.Max = ImVec2(frame_bb.Max.x - 2.0f, frame_bb.Max.y - 2.0f);
        auto w = ((gutter.Max.x - gutter.Min.x) - 4.0f) / 2.0f;
        gutter.Min.x += w;
        gutter.Max.x -= w;
        window->DrawList->AddRectFilled(gutter.Min, gutter.Max, GetColorU32(ImGuiCol_ButtonActive), style.GrabRounding);

        // Render grab
        window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(ImGuiCol_Text), style.GrabRounding);

        // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
        // For the vertical slider we allow centered text to overlap the frame padding
        char value_buf[64];
        sprintf(value_buf, format, int(*v * power));
        const char *value_buf_end = value_buf + strlen(value_buf);
        RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, nullptr, ImVec2(0.5f, 0.0f));
        if (label_size.x > 0.0f)
            RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

        return value_changed;
    }

    bool KnobUchar(char const *label, unsigned char *p_value, unsigned char v_min, unsigned char v_max, ImVec2 const &size, char const *tooltip)
    {
        bool showLabel = label[0] != '#' && label[1] != '#' && label[0] != '\0';

        ImGuiIO &io = ImGui::GetIO();
        ImGuiStyle &style = ImGui::GetStyle();
        ImVec2 s(size.x - 4, size.y - 4);

        float radius_outer = std::fmin(s.x, s.y) / 2.0f;
        ImVec2 pos = ImGui::GetCursorScreenPos();
        pos = ImVec2(pos.x, pos.y + 2);
        ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);

        float line_height = ImGui::GetFrameHeight();
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        float ANGLE_MIN = 3.141592f * 0.70f;
        float ANGLE_MAX = 3.141592f * 2.30f;

        if (s.x != 0.0f && s.y != 0.0f)
        {
            center.x = pos.x + (s.x / 2.0f);
            center.y = pos.y + (s.y / 2.0f);
            ImGui::InvisibleButton(label, ImVec2(s.x, s.y + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
        }
        else
        {
            ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
        }
        bool value_changed = false;
        bool is_active = ImGui::IsItemActive();

        bool is_hovered = ImGui::IsItemActive();
        if (is_active && io.MouseDelta.y != 0.0f)
        {
            int step = (v_max - v_min) / 127;
            int newVal = static_cast<int>(*p_value - io.MouseDelta.y * step);
            if (newVal < v_min)
                newVal = v_min;
            if (newVal > v_max)
                newVal = v_max;
            *p_value = static_cast<unsigned char>(newVal);
            value_changed = true;
        }

        float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * (*p_value - v_min) / (v_max - v_min);
        float angle_cos = cosf(angle);
        float angle_sin = sinf(angle);

        draw_list->AddCircleFilled(center, radius_outer * 0.7f, ImGui::GetColorU32(ImGuiCol_Button), 16);
        draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, ANGLE_MAX, 16);
        draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBg), false, 3.0f);
        draw_list->AddLine(
            ImVec2(center.x + angle_cos * (radius_outer * 0.35f), center.y + angle_sin * (radius_outer * 0.35f)),
            ImVec2(center.x + angle_cos * (radius_outer * 0.7f), center.y + angle_sin * (radius_outer * 0.7f)),
            ImGui::GetColorU32(ImGuiCol_Text), 2.0f);
        draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, angle + 0.02f, 16);
        draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_SliderGrabActive), false, 3.0f);

        if (showLabel)
        {
            auto textSize = CalcTextSize(label);
            draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);
        }

        if (is_active || is_hovered)
        {
            ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - (line_height * 2) - style.ItemInnerSpacing.y - style.WindowPadding.y));
            ImGui::BeginTooltip();
            if (tooltip != nullptr)
            {
                ImGui::Text("%s\nValue : %d", tooltip, static_cast<unsigned int>(*p_value));
            }
            else if (showLabel)
            {
                ImGui::Text("%s %d", label, static_cast<unsigned int>(*p_value));
            }
            else
            {
                ImGui::Text("%d", static_cast<unsigned int>(*p_value));
            }
            ImGui::EndTooltip();
        }
        else if (ImGui::IsItemHovered() && tooltip != nullptr)
        {
            ImGui::ShowTooltipOnHover(tooltip);
        }

        return value_changed;
    }

} // namespace ImGui

#define MIN_DB (-48)

void AddInsertFx(
    ApplicationSession *session,
    int track,
    int fx)
{
    if (session == nullptr || session->_mixer == nullptr)
    {
        return;
    }

    if (session->_mixer->GetTrackIndexForInsertEffect(fx) == -1)
    {
        session->_mixer->SetTrackIndexForInsertEffect(fx, static_cast<short>(track));
    }
}

void RemoveInsertFxFromTrack(
    ApplicationSession *session,
    int fx)
{
    if (session == nullptr || session->_mixer == nullptr)
    {
        return;
    }

    session->_mixer->SetTrackIndexForInsertEffect(fx, -1);
}

void MixerEditor::RenderTrack(
    int trackIndex)
{
    auto *track = _session->_mixer->GetTrack(trackIndex);
    if (track == nullptr)
    {
        return;
    }

    auto io = ImGui::GetStyle();

    if (ImGui::BeginChild("MixerTrack", trackSize))
    {
        auto trackEnabled = track->Penabled == 1;
        auto availableRegion = ImGui::GetContentRegionAvail();
        auto width = availableRegion.x;
        auto lineHeight = ImGui::GetTextLineHeight();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 2));

        auto hue = trackIndex * 0.05f;
        ImGui::PushStyleColor(ImGuiCol_Text, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.0f, 1.0f)));
        ImGui::PushStyleColor(ImGuiCol_CheckMark, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.0f, 1.0f)));
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBg, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

        // Enable/disable Track
        if (ImGui::Checkbox("##MixerTrackEnabled", &trackEnabled))
        {
            track->Penabled = trackEnabled ? 1 : 0;
        }

        ImGui::SameLine();

        // Change Track presets
        auto name = std::string(reinterpret_cast<char *>(track->Pname));
        if (ImGui::Button(name.empty() ? "<default>" : name.c_str(), ImVec2(width - 20 - io.ItemSpacing.x, 0)))
        {
        }

        ImGui::PopStyleColor(8);

        ImGui::Spacing();
        ImGui::SameLine(30);

        // System effect
        ImGui::TextCentered(ImVec2(width - 30, 30), "System FX");

        for (int fx = 0; fx < NUM_SYS_EFX; fx++)
        {
            ImGui::PushID(fx);
            ImGui::PushStyleColor(ImGuiCol_Button, _session->_mixer->GetSystemEffectType(fx) == 0 ? ImVec4(0.5f, 0.5f, 0.5f, 0.2f) : io.Colors[ImGuiCol_Button]);

            const float frameHeight = ImGui::GetFrameHeight();
            char tooltip[64] = {'\0'};
            sprintf(tooltip, "Volume for system effect %d", (fx + 1));

            auto value = _session->_mixer->GetSystemEffectVolume(trackIndex, fx);
            if (ImGui::KnobUchar("##sysfx", &value, 0, 127, ImVec2(frameHeight + io.ItemInnerSpacing.x, frameHeight), tooltip))
            {
                _session->_mixer->SetSystemEffectVolume(trackIndex, fx, value);
            }

            ImGui::SameLine();

            if (ImGui::Button(_session->_mixer->GetSystemEffectName(fx), ImVec2(width - lineHeight - 1, 0)))
            {
            }

            ImGui::OpenPopupOnItemClick("SystemEffectSelection", 0);
            if (ImGui::BeginPopupContextItem("SystemEffectSelection"))
            {
                for (unsigned int i = 0; i < mixer_editor::EffectNameCount; i++)
                {
                    if (ImGui::Selectable(mixer_editor::EffectNames[i]))
                    {
                        _session->_mixer->SetSystemEffectType(fx, int(i));
                    }
                }
                ImGui::PushItemWidth(-1);
                ImGui::PopItemWidth();
                ImGui::EndPopup();
            }

            ImGui::PopStyleColor(1);
            ImGui::PopID();
        }

        ImGui::Spacing();
        ImGui::SameLine(30);

        // Insertion effects
        ImGui::TextCentered(ImVec2(width - 30, 30), "Insert FX");

        int fillCount = NUM_INS_EFX;
        for (int fx = 0; fx < NUM_INS_EFX; fx++)
        {
            ImGui::PushID(100 + fx);

            bool insertEffectEnabled = _session->_mixer->GetTrackIndexForInsertEffect(fx) == trackIndex;
            if (ImGui::Checkbox("##InsertEffectEnabled", &insertEffectEnabled))
            {
                if (insertEffectEnabled)
                {
                    AddInsertFx(_session, trackIndex, fx);
                }
                else
                {
                    RemoveInsertFxFromTrack(_session, fx);
                }
            }
            ImGui::ShowTooltipOnHover("Add or remove insert effect from track");

            ImGui::SameLine();

            if (_session->_mixer->GetTrackIndexForInsertEffect(fx) == trackIndex)
            {
                ImGui::Button(_session->_mixer->GetInsertEffectName(fx), ImVec2(width - 30, 0));
                ImGui::OpenPopupOnItemClick("InsertEffectSelection", 0);
                if (ImGui::BeginPopupContextItem("InsertEffectSelection"))
                {
                    for (unsigned int i = 0; i < mixer_editor::EffectNameCount; i++)
                    {
                        if (ImGui::Selectable(mixer_editor::EffectNames[i]))
                        {
                            _session->_mixer->SetInsertEffectType(fx, int(i));
                        }
                    }
                    ImGui::PushItemWidth(-1);
                    ImGui::PopItemWidth();
                    ImGui::EndPopup();
                }
                fillCount--;
            }
            else
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.5f, 0.5f, 0.5f, 0.1f));
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.5f, 0.5f, 0.5f, 0.1f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 0.1f));
                ImGui::Spacing();
                ImGui::SameLine(30);

                ImGui::Button(_session->_mixer->GetInsertEffectName(fx), ImVec2(width - 30, 0));
                ImGui::PopStyleColor(3);
            }
            ImGui::PopID();
        }

        ImGui::PopStyleVar(1);

        ImGui::Spacing();

        auto panning = track->Ppanning;
        if (ImGui::KnobUchar("panning", &panning, 0, 127, ImVec2(width, 40), "Track panning"))
        {
            track->setPpanning(panning);
        }

        ImGui::Spacing();

        auto faderHeight = ImGui::GetWindowContentRegionMax().y - ImGui::GetCursorPos().y - io.ItemSpacing.y - 30;

        // Instrument volume
        if (faderHeight < (40 + ImGui::GetTextLineHeight()))
        {
            ImGui::KnobUchar("volume", &(track->Pvolume), 0, 127, ImVec2(width, 40), "Master Volume");
        }
        else
        {
            float db = rap2dB(_session->_mixer->GetMeter()->GetOutPeak(trackIndex));

            db = (MIN_DB - db) / MIN_DB;
            if (db < 0.0f)
                db = 0.0f;
            else if (db > 1.0f)
                db = 1.0f;

            db *= faderHeight;

            int idb = static_cast<int>(db);

            ImGui::Spacing();
            ImGui::SameLine(0.0f, (width - (40.0f + (3 * io.ItemSpacing.x))) / 2.0f);

            int v = static_cast<int>(track->Pvolume);
            if (ImGui::Fader("##vol", ImVec2(20, faderHeight), &v, 0, 127, "%d", 100.0f / 127.0f))
            {
                track->SetVolume(static_cast<unsigned char>(v));
            }
            ImGui::ShowTooltipOnHover("Track volume");
            ImGui::SameLine();
            ImGui::Spacing();

            ImGui::SameLine();
            ImGui::UvMeter("##instrument_uvr", ImVec2(20, faderHeight), &idb, 0, static_cast<int>(faderHeight));
        }
        ImGui::ShowTooltipOnHover("Instrument volume");

        ImGui::TextCentered(ImVec2(width, 30), "stereo out");
    }
    ImGui::EndChild();
}

void MixerEditor::Render2d()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin(
        MixerEditor::ID,
        nullptr,
        flags);
    {
        if (ImGui::BeginChild("Mixer", ImVec2(), false, ImGuiWindowFlags_AlwaysHorizontalScrollbar))
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));

            for (unsigned int track = 0; track <= _session->_mixer->GetTrackCount(); track++)
            {
                ImGui::PushID(track);

                RenderTrack(track);

                ImGui::PopID();

                if (track < _session->_mixer->GetTrackCount() - 1)
                {
                    ImGui::SameLine();
                }
            }

            ImGui::PopStyleVar();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}
