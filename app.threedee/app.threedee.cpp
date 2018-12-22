#include "app.threedee.h"

#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnoteParams.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_addons/imgui_checkbutton.h"
#include "imgui_addons/imgui_knob.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _mixer(mixer), _window(window), _stepper(&_sequencer, mixer),
      _display_w(800), _display_h(600),
      _currentBank(0), showAddSynthEditor(false)
{
    glfwSetWindowUserPointer(this->_window, static_cast<void *>(this));
}

AppThreeDee::~AppThreeDee()
{
    glfwSetWindowUserPointer(this->_window, nullptr);
}

void AppThreeDee::KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<AppThreeDee *>(glfwGetWindowUserPointer(window));

    if (app != nullptr)
    {
        app->onKeyAction(key, scancode, action, mods);
        ImGui_ImplGlfw_KeyCallback(app->_window, key, scancode, action, mods);
    }
}

void AppThreeDee::ResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = static_cast<AppThreeDee *>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onResize(width, height);
}

void AppThreeDee::onResize(int width, int height)
{
    this->_display_w = width;
    this->_display_h = height;

    glViewport(0, 0, width, height);
}

bool AppThreeDee::Setup()
{
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    io.ConfigDockingWithShift = false;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Setup style
    ImGui::StyleColorsDark();

    _mixer->GetBankManager()->RescanForBanks();
    _mixer->GetBankManager()->LoadBank(_currentBank);

    _stepper.Setup();

    return true;
}

static ImVec4 clear_color = ImColor(114, 144, 154);

static bool showInstrumentEditor = false;
static bool showPatternEditor = false;
static bool showADNoteEditor = true;
static int openSelectInstrument = -1;

void AppThreeDee::HitNote(int trackIndex, int note, int durationInMs)
{
    _stepper.HitNote(static_cast<unsigned char>(trackIndex), static_cast<unsigned char>(note), durationInMs);
}

void AppThreeDee::EditSelectedPattern()
{
    showPatternEditor = true;
}

void AppThreeDee::ImGuiSelectedTrack()
{
    if (_sequencer.ActiveInstrument() < 0 || _sequencer.ActiveInstrument() >= NUM_MIXER_CHANNELS)
    {
        _sequencer.ActiveInstrument(-1);
        return;
    }

    auto io = ImGui::GetStyle();

    ImGui::Begin("Selected Track");
    {
        auto lh = ImGui::GetItemsLineHeightWithSpacing();
        auto width = ImGui::GetContentRegionAvail().x;
        float const sliderHeight = 200.0f;

        auto sliderPanelHeight =
            sliderHeight                                   /*Fader height*/
            + lh                                           /*Instrument button*/
            + ((width / 2) + lh + (io.ItemSpacing.y * 2)); /*panning knob*/

        auto spaceLeft =
            ImGui::GetContentRegionAvail().y /*Available height*/
            - sliderPanelHeight              /*Volume fader*/
            - lh                             /*Settings*/
            - lh;                            /*Input channel*/

        ImGui::BeginChild("item view", ImVec2(0, -sliderPanelHeight));
        {
            if (ImGui::Button("Settings", ImVec2(width, 0)))
            {
                showAddSynthEditor = true;
            }

            const char *channels[] = {
                "1",
                "2",
                "3",
                "4",
                "5",
                "6",
                "7",
                "8",
                "9",
                "10",
                "11",
                "12",
                "13",
                "14",
                "15",
                "16",
            };
            int channel = static_cast<int>(_mixer->GetChannel(_sequencer.ActiveInstrument())->Prcvchn);
            ImGui::PushItemWidth(width);
            if (ImGui::Combo("##KeyboardChannel", &channel, channels, NUM_MIXER_CHANNELS))
            {
                _mixer->GetChannel(_sequencer.ActiveInstrument())->Prcvchn = static_cast<unsigned char>(channel);
            }

            auto fxButtonCount = spaceLeft / ImGui::GetItemsLineHeightWithSpacing() - 1;
            for (int fx = 0; fx < fxButtonCount; fx++)
            {
                char fxButton[32] = {0};
                sprintf(fxButton, "fx %d", fx + 1);
                ImGui::Button(fxButton, ImVec2(width, 0));
            }
        }
        ImGui::EndChild();

        ImGui::Spacing();
        ImGui::SameLine(0.0f, width / 4);
        auto panning = _mixer->GetChannel(_sequencer.ActiveInstrument())->Ppanning;
        if (ImGui::KnobUchar("panning", &panning, 0, 128, ImVec2(width / 2, width / 2)))
        {
            _mixer->GetChannel(_sequencer.ActiveInstrument())->setPpanning(panning);
        }

        float peakl, peakr;
        _mixer->GetChannel(_sequencer.ActiveInstrument())->ComputePeakLeftAndRight(_mixer->GetChannel(_sequencer.ActiveInstrument())->Pvolume, peakl, peakr);

        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::SameLine(0.0f, (width - 20) / 2);
        int v = static_cast<int>(_mixer->GetChannel(_sequencer.ActiveInstrument())->Pvolume);
        if (ImGui::VSliderInt("##vol", ImVec2(20, sliderHeight - io.ItemSpacing.y), &v, 0, 128))
        {
            _mixer->GetChannel(_sequencer.ActiveInstrument())->setPvolume(static_cast<unsigned char>(v));
        }

        auto hue = _sequencer.ActiveInstrument() * 0.05f;
        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

        auto name = std::string(reinterpret_cast<char *>(_mixer->GetChannel(_sequencer.ActiveInstrument())->Pname));
        if (ImGui::Button(name.size() == 0 ? "default" : name.c_str(), ImVec2(width, 0)))
        {
            openSelectInstrument = _sequencer.ActiveInstrument();
        }

        ImGui::PopStyleColor(3);
    }
    ImGui::End();
}

void AppThreeDee::ImGuiSelectInstrumentPopup()
{
    if (openSelectInstrument >= 0)
    {
        ImGui::OpenPopup("Select Instrument");
    }

    ImGui::SetNextWindowSize(ImVec2(700, 850));
    if (ImGui::BeginPopupModal("Select Instrument"))
    {
        auto count = _mixer->GetBankManager()->GetBankCount();
        auto const &bankNames = _mixer->GetBankManager()->GetBankNames();
        if (ImGui::Combo("Bank", &_currentBank, &(bankNames[0]), int(count)))
        {
            _mixer->GetBankManager()->LoadBank(_currentBank);
        }

        static bool autoClose = false;
        ImGui::SameLine();
        ImGui::Checkbox("Auto close", &autoClose);

        ImGui::SameLine();
        if (ImGui::Button("Close"))
        {
            openSelectInstrument = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::BeginChild("banks", ImVec2(0, -20));
        ImGui::Columns(5);
        if (_currentBank >= 0)
        {
            for (unsigned int i = 0; i < BANK_SIZE; i++)
            {
                auto instrumentName = _mixer->GetBankManager()->GetName(i);

                if (ImGui::Button(instrumentName.c_str(), ImVec2(120, 20)))
                {
                    auto const &instrument = _mixer->GetChannel(_sequencer.ActiveInstrument());
                    instrument->Lock();
                    _mixer->GetBankManager()->LoadFromSlot(i, instrument);
                    instrument->Unlock();
                    instrument->ApplyParameters();
                    openSelectInstrument = -1;
                    if (autoClose)
                    {
                        ImGui::CloseCurrentPopup();
                    }
                }
                if ((i + 1) % 32 == 0)
                {
                    ImGui::NextColumn();
                }
            }
        }
        ImGui::EndChild();
        ImGui::EndPopup();
    }
}

void AppThreeDee::ImGuiSequencer()
{
    ImGui::Begin("Sequencer");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));

        static int trackHeight = 30;
        ImGui::BeginChild("scrolling", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (int trackIndex = 0; trackIndex < NUM_MIXER_CHANNELS; trackIndex++)
        {
            ImGui::PushID(trackIndex * 1100);
            auto trackEnabled = _mixer->GetChannel(trackIndex)->Penabled == 1;
            if (ImGui::Checkbox("##trackEnabled", &trackEnabled))
            {
                _mixer->GetChannel(trackIndex)->Penabled = trackEnabled ? 1 : 0;
            }
            ImGui::SameLine();

            float hue = trackIndex * 0.05f;
            char trackLabel[32] = {'\0'};
            sprintf(trackLabel, "%02d", trackIndex + 1);
            bool highLight = trackIndex == _sequencer.ActiveInstrument();
            if (highLight)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            }
            if (ImGui::Button(trackLabel, ImVec2(trackHeight, trackHeight)))
            {
                _sequencer.ActiveInstrument(trackIndex);
            }
            if (highLight)
            {
                ImGui::PopStyleColor();
            }
            ImGui::PopID();

            ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

            //if (_mixer->GetChannel(trackIndex)->Pkitmode != 0)
            {
                ImGuiStepSequencer(trackIndex, trackHeight);
            }
            //else
            {
                ImGuiPianoRollSequencer(trackIndex, trackHeight);
            }

            ImGui::PopStyleColor(3);
        }
        float scroll_y = ImGui::GetScrollY();
        ImGui::EndChild();

        ImGui::PopStyleVar(2);

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
            //if (_sequencer.ActiveInstrument() >= 0 && _mixer->GetChannel(_sequencer.ActiveInstrument())->Pkitmode != 0)
            {
                ImGuiStepSequencerEventHandling();
            }
            //else
            {
                ImGuiPianoRollSequencerEventHandling();
            }
        }
    }
    ImGui::End();
}

void AppThreeDee::ImGuiStepSequencer(int trackIndex, float trackHeight)
{
    auto lastIndex = _sequencer.LastPatternIndex(trackIndex);
    for (int patternIndex = 0; patternIndex <= lastIndex; patternIndex++)
    {
        ImGui::SameLine();
        ImGui::PushID(patternIndex + trackIndex * 1000);
        bool isActive = trackIndex == _sequencer.ActiveInstrument() && patternIndex == _sequencer.ActivePattern();
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        }

        if (_sequencer.DoesPatternExistAtIndex(trackIndex, patternIndex))
        {
            auto &pattern = _sequencer.GetPattern(trackIndex, patternIndex);
            if (ImGui::Button(pattern._name.c_str(), ImVec2(120.0f, trackHeight)))
            {
                _sequencer.ActiveInstrument(trackIndex);
                _sequencer.ActivePattern(patternIndex);
            }
            if (ImGui::IsMouseDoubleClicked(0))
            {
                EditSelectedPattern();
            }
        }
        else if (_mixer->GetChannel(trackIndex)->Penabled)
        {
            if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
            {
                _sequencer.AddPattern(trackIndex, patternIndex, "");
            }
        }
        if (isActive)
        {
            ImGui::PopStyleColor(1);
            ImGui::PopStyleVar(1);
        }

        ImGui::PopID();
    }

    if (_mixer->GetChannel(trackIndex)->Penabled)
    {
        ImGui::SameLine();
        ImGui::PushID((100 + trackIndex) * 2010);
        if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
        {
            _sequencer.AddPattern(trackIndex, lastIndex + 1, "");
        }
        ImGui::PopID();
    }
}

void AppThreeDee::ImGuiStepSequencerEventHandling()
{
    ImGuiIO &io = ImGui::GetIO();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
    {
        _sequencer.RemoveActivePattern();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
    {
        if (io.KeyShift && !io.KeyCtrl)
        {
            _sequencer.MovePatternLeftForced();
        }
        else if (!io.KeyShift && io.KeyCtrl)
        {
            _sequencer.SwitchPatternLeft();
        }
        else
        {
            _sequencer.MovePatternLeftIfPossible();
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
    {
        if (io.KeyShift && !io.KeyCtrl)
        {
            _sequencer.MovePatternRightForced();
        }
        else if (!io.KeyShift && io.KeyCtrl)
        {
            _sequencer.SwitchPatternRight();
        }
        else
        {
            _sequencer.MovePatternRightIfPossible();
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home)))
    {
        _sequencer.SelectFirstPatternInTrack();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End)))
    {
        _sequencer.SelectLastPatternInTrack();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
    {
        EditSelectedPattern();
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
    {
        if (io.KeyShift)
        {
            _sequencer.SelectPreviousPattern();
        }
        else
        {
            _sequencer.SelectNextPattern();
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)) && io.KeyCtrl)
    {
        if (_sequencer.DoesPatternExistAtIndex(_sequencer.ActiveInstrument(), _sequencer.ActivePattern()))
        {
            auto pattern = _sequencer.GetPattern(_sequencer.ActiveInstrument(), _sequencer.ActivePattern());
            _clipboardPatterns.push_back(pattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)) && io.KeyCtrl)
    {
        if (!_clipboardPatterns.empty() && _sequencer.DoesPatternExistAtIndex(_sequencer.ActiveInstrument(), _sequencer.ActivePattern()))
        {
            _sequencer.SetPattern(_sequencer.ActiveInstrument(), _sequencer.ActivePattern(), _clipboardPatterns.back());
        }
    }
}

void AppThreeDee::ImGuiPianoRollSequencer(int /*trackIndex*/, float /*trackHeight*/)
{
}

void AppThreeDee::ImGuiPianoRollSequencerEventHandling()
{
}

static const char *notes[] = {
    "A",
    "A#",
    "B",
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
};

void AppThreeDee::ImGuiPatternEditorWindow()
{
    const float noteLabelWidth = 50.0f;
    const float rowHeight = 20.0f;
    if (showPatternEditor)
    {
        if (!_sequencer.DoesPatternExistAtIndex(_sequencer.ActiveInstrument(), _sequencer.ActivePattern()))
        {
            _sequencer.ActivePattern(-1);
            return;
        }

        auto &style = ImGui::GetStyle();
        auto &selectedPattern = _sequencer.GetPattern(_sequencer.ActiveInstrument(), _sequencer.ActivePattern());

        ImGui::Begin("Pattern editor", &showPatternEditor);
        auto width = ImGui::GetWindowWidth() - noteLabelWidth - (style.ItemSpacing.x * 2) - style.ScrollbarSize;
        auto itemWidth = (width / 16) - (style.ItemSpacing.x);
        for (int i = 0; i < 88; i++)
        {
            if (i % 12 == 0)
            {
                ImGui::Separator();
            }
            ImGui::PushID(i);
            if (ImGui::Button(notes[i % 12], ImVec2(noteLabelWidth, rowHeight)))
            {
                HitNote(_sequencer.ActiveInstrument(), i, 200);
            }
            for (int j = 0; j < 16; j++)
            {
                ImGui::SameLine();
                ImGui::PushID(j);
                auto found = selectedPattern._notes.find(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j)));
                bool s = found != selectedPattern._notes.end();
                if (j % 4 == 0)
                {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
                }
                else
                {
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, style.Colors[ImGuiCol_FrameBg]);
                }
                if (ImGui::CheckButton("##note", &s, ImVec2(itemWidth, rowHeight)))
                {
                    if (!s)
                    {
                        selectedPattern._notes.erase(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j)));
                    }
                    else
                    {
                        selectedPattern._notes.insert(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j)));
                    }
                    HitNote(_sequencer.ActiveInstrument(), i, 200);
                }
                ImGui::PopStyleColor();
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        ImGui::End();
    }
}

void AppThreeDee::Tick()
{
    _stepper.Tick();
}

void AppThreeDee::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuiIO &io = ImGui::GetIO();

    ImGuiViewport *viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::Begin("TestDockspace", nullptr, window_flags);
    {
        ImGuiID dockspace_id = ImGui::GetID("ZynDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);
    }
    ImGui::End();
    ImGui::PopStyleVar(2);

    ImGuiSequencer();
    ImGuiPatternEditorWindow();
    ImGuiSelectedTrack();
    ImGuiSelectInstrumentPopup();

    // 3. Show another simple window.
    if (showADNoteEditor)
    {
        ImGui::Begin("AD note editor", &showADNoteEditor);
        if (ImGui::Button("Close Me"))
        {
            showADNoteEditor = false;
        }

        if (ImGui::BeginTabBar("blah"))
        {
            if (ImGui::BeginTabItem("Global"))
            {
                if (_sequencer.ActiveInstrument() >= 0)
                {
                    ADNoteEditor(_mixer->GetChannel(_sequencer.ActiveInstrument())->_instruments[0].adpars);
                }
                ImGui::EndTabItem();
            }
            static char voiceIds[][64]{
                "Voice 1",
                "Voice 2",
                "Voice 3",
                "Voice 4",
                "Voice 5",
                "Voice 6",
                "Voice 7",
                "Voice 8",
            };
            for (int i = 0; i < NUM_VOICES; i++)
            {
                if (_sequencer.ActiveInstrument() >= 0)
                {
                    auto parameters = &_mixer->GetChannel(_sequencer.ActiveInstrument())->_instruments[0].adpars->VoicePar[i];
                    if (ImGui::BeginTabItem(voiceIds[i]))
                    {
                        ADNoteVoiceEditor(parameters);

                        ImGui::EndTabItem();
                    }
                }
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    ImGuiPlayback();

    ImGui::Render();

    // Update and Render additional Platform Windows
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    int display_w, display_h;
    glfwMakeContextCurrent(_window);
    glfwGetFramebufferSize(_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void AppThreeDee::onKeyAction(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{
}

void AppThreeDee::ImGuiPlayback()
{
    ImGui::Begin("Playback");
    {
        if (ImGui::Button("Stop"))
        {
            _stepper.Stop();
        }

        ImGui::SameLine();

        if (_stepper.IsPlaying())
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyle().Colors[ImGuiCol_Button]);
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_Text]);
        }

        if (ImGui::Button("Play/Pause"))
        {
            _stepper.PlayPause();
        }

        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushItemWidth(200);
        auto bpm = _stepper.Bpm();
        if (ImGui::SliderInt("##BPM", &bpm, 10, 200, "BPM %d"))
        {
            _stepper.Bpm(bpm);
        }

        ImGui::SameLine();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0 / static_cast<double>(ImGui::GetIO().Framerate), static_cast<double>(ImGui::GetIO().Framerate));

        ImGui::End();
    }
}

void AppThreeDee::Cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
