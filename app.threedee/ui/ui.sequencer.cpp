#include "ui.sequencer.h"

#include "../appstate.h"
#include "../imgui_addons/imgui_knob.h"
#include <imgui.h>
#include <zyn.mixer/Mixer.h>

const char *TrackPatternTypeNames[]{
    "Step",
    "Clip",
    "Arpeggiator"};

zyn::ui::Sequencer::Sequencer(AppState *state)
    : _state(state), _pianoRollUi(state), _stepPatternUi(state)
{}

zyn::ui::Sequencer::~Sequencer() = default;

bool zyn::ui::Sequencer::Setup()
{
    if (!_pianoRollUi.Setup())
    {
        return false;
    }

    if (!_stepPatternUi.Setup())
    {
        return false;
    }

    return true;
}

void zyn::ui::Sequencer::Render()
{
    if (!_state->_showEditor)
    {
        return;
    }

    if (ImGui::Begin("Sequencer", &_state->_showEditor))
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 5.0f));

        ImGui::PushItemWidth(100);
        ImGui::SliderInt("##Vertical zoom", &_state->_sequencerVerticalZoom, 40, 200);
        ImGui::ShowTooltipOnHover("Vertical zoom");

        ImGui::SameLine();

        ImGui::PushItemWidth(100);
        ImGui::SliderInt("##Horizontal zoom", &_state->_sequencerHorizontalZoom, 40, 200);
        ImGui::ShowTooltipOnHover("Horizontal zoom");

        ImGui::BeginChild("scrolling", ImVec2(0, -100.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (int trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; trackIndex++)
        {
            ImGui::PushID(trackIndex * 1100);
            auto trackEnabled = _state->_mixer->GetTrack(trackIndex)->Penabled == 1;
            if (ImGui::Checkbox("##SequencerTrackEnabled", &trackEnabled))
            {
                _state->_mixer->GetTrack(trackIndex)->Penabled = trackEnabled ? 1 : 0;
            }
            ImGui::SameLine();

            float hue = trackIndex * 0.05f;
            char trackLabel[32] = {'\0'};
            sprintf(trackLabel, "%02d", trackIndex + 1);
            bool highLight = (trackIndex == _state->_activeTrack);
            if (highLight)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            }
            if (ImGui::Button(trackLabel, ImVec2(60.0f, _state->_sequencerVerticalZoom)))
            {
                _state->_activeTrack = trackIndex;
            }
            if (highLight)
            {
                ImGui::PopStyleColor();
            }
            ImGui::PopID();

            ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

            switch (_state->_trackPatternType[trackIndex])
            {
                case TrackPatternTypes::Clip:
                {
                    //_pianoRollUi.Render(trackIndex, 0);
                    _stepPatternUi.Render(trackIndex, _state->_sequencerVerticalZoom);
                    break;
                }
                case TrackPatternTypes::Step:
                {
                    _stepPatternUi.Render(trackIndex, _state->_sequencerVerticalZoom);
                    break;
                }
                case TrackPatternTypes::Arpeggiator:
                {
                    break;
                }
                default:
                {
                    break;
                }
            }

            ImGui::PopStyleColor(3);
        }
        float scroll_y = ImGui::GetScrollY();
        ImGui::EndChild();

        ImGui::PopStyleVar(2);

        ImGui::Text("Track %02d", _state->_activeTrack + 1);

        ImGui::SameLine();

        ImGui::PushItemWidth(100);
        unsigned char trackPatternType = static_cast<unsigned char>(_state->_trackPatternType[_state->_activeTrack]);
        if (ImGui::DropDown("##track-pattern-type", trackPatternType, TrackPatternTypeNames, int(TrackPatternTypes::Count), "Pattern type"))
        {
            _state->_trackPatternType[_state->_activeTrack] = static_cast<TrackPatternTypes>(trackPatternType);
        }

        float scroll_x_delta = 0.0f;
        ImGui::SmallButton("<<");
        if (ImGui::IsItemActive()) scroll_x_delta = -ImGui::GetIO().DeltaTime * 1000.0f;
        ImGui::SameLine();
        ImGui::Text("Scroll from code");
        ImGui::SameLine();
        ImGui::SmallButton(">>");
        if (ImGui::IsItemActive()) scroll_x_delta = +ImGui::GetIO().DeltaTime * 1000.0f;
        if (scroll_x_delta != 0.0f)
        {
            ImGui::BeginChild("scrolling");
            ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
            ImGui::End();
        }

        ImGui::BeginChild("info");
        ImGui::SetScrollY(scroll_y);
        ImGui::EndChild();

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        {
            switch (_state->_trackPatternType[_state->_activeTrack])
            {
                case TrackPatternTypes::Clip:
                {
                    // _pianoRollUi.EventHandling();
                    break;
                }
                case TrackPatternTypes::Step:
                {
                    _stepPatternUi.EventHandling();
                    break;
                }
                case TrackPatternTypes::Arpeggiator:
                {
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    ImGui::End();

    switch (_state->_trackPatternType[_state->_activeTrack])
    {
        case TrackPatternTypes::Clip:
        {
            break;
        }
        case TrackPatternTypes::Step:
        {
            _stepPatternUi.RenderStepPatternEditorWindow();
            break;
        }
        case TrackPatternTypes::Arpeggiator:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}
