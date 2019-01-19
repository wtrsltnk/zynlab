#include "ui.sequencer.h"

#include "../appstate.h"
#include "../imgui_addons/imgui_knob.h"
#include <imgui.h>
#include <zyn.mixer/Mixer.h>

const char *ChannelPatternTypeNames[]{
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

    ImGui::Begin("Sequencer");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f,5.0f));

        ImGui::PushItemWidth(100);
        ImGui::SliderInt("##Vertical zoom", &_state->_sequencerVerticalZoom, 40, 200);
        ImGui::ShowTooltipOnHover("Vertical zoom");

        ImGui::SameLine();

        ImGui::PushItemWidth(100);
        ImGui::SliderInt("##Horizontal zoom", &_state->_sequencerHorizontalZoom, 40, 200);
        ImGui::ShowTooltipOnHover("Horizontal zoom");

        ImGui::BeginChild("scrolling", ImVec2(0, -100.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (int trackIndex = 0; trackIndex < NUM_MIXER_CHANNELS; trackIndex++)
        {
            ImGui::PushID(trackIndex * 1100);
            auto trackEnabled = _state->_mixer->GetChannel(trackIndex)->Penabled == 1;
            if (ImGui::Checkbox("##SequencerChannelEnabled", &trackEnabled))
            {
                _state->_mixer->GetChannel(trackIndex)->Penabled = trackEnabled ? 1 : 0;
            }
            ImGui::SameLine();

            float hue = trackIndex * 0.05f;
            char trackLabel[32] = {'\0'};
            sprintf(trackLabel, "%02d", trackIndex + 1);
            bool highLight = (trackIndex == _state->_activeChannel);
            if (highLight)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            }
            if (ImGui::Button(trackLabel, ImVec2(60.0f, _state->_sequencerVerticalZoom)))
            {
                _state->_activeChannel = trackIndex;
            }
            if (highLight)
            {
                ImGui::PopStyleColor();
            }
            ImGui::PopID();

            ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

            switch (_state->_channelPatternType[trackIndex])
            {
                case ChannelPatternTypes::Clip:
                {
                    //_pianoRollUi.Render(trackIndex, 0);
                    break;
                }
                case ChannelPatternTypes::Step:
                {
                    _stepPatternUi.Render(trackIndex, _state->_sequencerVerticalZoom);
                    break;
                }
                case ChannelPatternTypes::Arpeggiator:
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

        ImGui::Text("Channel %02d", _state->_activeChannel+1);

        ImGui::SameLine();

        ImGui::PushItemWidth(100);
        unsigned char channelPatternType = static_cast<unsigned char>(_state->_channelPatternType[_state->_activeChannel]);
        if (ImGui::DropDown("##channel-pattern-type", channelPatternType, ChannelPatternTypeNames, int(ChannelPatternTypes::Count), "Pattern type"))
        {
            _state->_channelPatternType[_state->_activeChannel] = static_cast<ChannelPatternTypes>(channelPatternType);
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
            switch (_state->_channelPatternType[_state->_activeChannel])
            {
                case ChannelPatternTypes::Clip:
                {
                    //            _pianoRollUi.EventHandling();
                    break;
                }
                case ChannelPatternTypes::Step:
                {
                    _stepPatternUi.EventHandling();
                    break;
                }
                case ChannelPatternTypes::Arpeggiator:
                {
                    break;
                }
                default:
                {
                    break;
                }
            }
        }

        _stepPatternUi.RenderStepPatternEditorWindow();
    }
    ImGui::End();
}
