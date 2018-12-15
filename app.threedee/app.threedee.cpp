#include "app.threedee.h"

#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnoteParams.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_addons/imgui_knob.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <map>

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _mixer(mixer), _window(window), _display_w(800), _display_h(600),
      showAddSynthEditor(false)
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

bool AppThreeDee::SetUp()
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

    return true;
}

static ImVec4 clear_color = ImColor(114, 144, 154);
static std::map<int, TrackPattern> tracksOfPatterns[NUM_MIXER_CHANNELS];
static int activeInstrument = 0;
static int activePattern = -1;

static bool showInstrumentEditor = false;
static bool showPatternEditor = false;
static int keyboardChannel = 0;
static bool showADNoteEditor = true;
static bool isPlaying = false;

void AppThreeDee::AddPattern(int trackIndex, int patternIndex, char const *label)
{
    static int n = std::rand() % 255;
    float hue = n * 0.05f;
    activeInstrument = trackIndex;
    activePattern = patternIndex;
    tracksOfPatterns[trackIndex].insert(std::make_pair(patternIndex, TrackPattern(label, hue)));
    n = (n + std::rand()) % 255;
}

void AppThreeDee::RemoveActivePattern()
{
    if (activeInstrument < 0 || activePattern < 0)
    {
        return;
    }

    tracksOfPatterns[activeInstrument].erase(activePattern);
    activeInstrument = -1;
    activePattern = -1;
}

void AppThreeDee::MovePatternLeftIfPossible()
{
    auto ap = tracksOfPatterns[activeInstrument].find(activePattern);

    if (activeInstrument < 0 || activePattern < 0 || ap == tracksOfPatterns[activeInstrument].end())
    {
        return;
    }

    auto currentKey = ap->first;

    if (currentKey == 0)
    {
        return;
    }

    auto currentValue = ap->second;
    auto newKey = currentKey - 1;

    if (tracksOfPatterns[activeInstrument].find(newKey) == tracksOfPatterns[activeInstrument].end())
    {
        tracksOfPatterns[activeInstrument].insert(std::make_pair(newKey, currentValue));
        tracksOfPatterns[activeInstrument].erase(currentKey);
        activePattern = newKey;
    }
}

void AppThreeDee::MovePatternLeftForced()
{
    auto ap = tracksOfPatterns[activeInstrument].find(activePattern);

    if (activeInstrument < 0 || activePattern < 0 || ap == tracksOfPatterns[activeInstrument].end())
    {
        return;
    }

    if (tracksOfPatterns[activeInstrument].begin()->first == 0)
    {
        return;
    }

    for (int i = tracksOfPatterns[activeInstrument].begin()->first; i <= ap->first; i++)
    {
        auto itr = tracksOfPatterns[activeInstrument].find(i);
        if (itr == tracksOfPatterns[activeInstrument].end())
        {
            continue;
        }
        tracksOfPatterns[activeInstrument].insert(std::make_pair(i - 1, itr->second));
        tracksOfPatterns[activeInstrument].erase(i);
    }

    activePattern = activePattern - 1;
}

void AppThreeDee::SwitchPatternLeft()
{
    auto ap = tracksOfPatterns[activeInstrument].find(activePattern);

    if (activeInstrument < 0 || activePattern < 0 || ap == tracksOfPatterns[activeInstrument].end())
    {
        return;
    }

    auto currentKey = ap->first;

    if (currentKey <= 0)
    {
        return;
    }

    auto currentValue = ap->second;
    auto newKey = currentKey - 1;

    tracksOfPatterns->erase(currentKey);

    if (tracksOfPatterns[activeInstrument].find(newKey) != tracksOfPatterns[activeInstrument].end())
    {
        auto tmpValue = tracksOfPatterns[activeInstrument].find(newKey)->second;
        tracksOfPatterns[activeInstrument].erase(newKey);
        tracksOfPatterns[activeInstrument].insert(std::make_pair(currentKey, tmpValue));
    }

    tracksOfPatterns[activeInstrument].insert(std::make_pair(newKey, currentValue));

    activePattern = newKey;
}

void AppThreeDee::MovePatternRightIfPossible()
{
    auto ap = tracksOfPatterns[activeInstrument].find(activePattern);

    if (activeInstrument < 0 || activePattern < 0 || ap == tracksOfPatterns[activeInstrument].end())
    {
        return;
    }

    auto currentKey = ap->first;
    auto currentValue = ap->second;
    auto newKey = currentKey + 1;

    ap++;
    auto nextKey = ap->first;
    if (ap == tracksOfPatterns[activeInstrument].end() || newKey < nextKey)
    {
        tracksOfPatterns[activeInstrument].insert(std::make_pair(newKey, currentValue));
        tracksOfPatterns[activeInstrument].erase(currentKey);
        activePattern = newKey;
    }
}

void AppThreeDee::MovePatternRightForced()
{
    auto ap = tracksOfPatterns[activeInstrument].find(activePattern);

    if (activeInstrument < 0 || activePattern < 0 || ap == tracksOfPatterns[activeInstrument].end())
    {
        return;
    }

    for (int i = tracksOfPatterns[activeInstrument].rbegin()->first; i >= ap->first; i--)
    {
        auto itr = tracksOfPatterns[activeInstrument].find(i);
        if (itr == tracksOfPatterns[activeInstrument].end())
        {
            continue;
        }
        tracksOfPatterns[activeInstrument].insert(std::make_pair(i + 1, itr->second));
        tracksOfPatterns[activeInstrument].erase(i);
    }

    activePattern = activePattern + 1;
}

void AppThreeDee::SwitchPatternRight()
{
    auto ap = tracksOfPatterns[activeInstrument].find(activePattern);

    if (activeInstrument < 0 || activePattern < 0 || ap == tracksOfPatterns[activeInstrument].end())
    {
        return;
    }

    auto currentKey = ap->first;
    auto currentValue = ap->second;
    auto newKey = currentKey + 1;

    tracksOfPatterns->erase(currentKey);

    if (tracksOfPatterns[activeInstrument].find(newKey) != tracksOfPatterns[activeInstrument].end())
    {
        auto tmpValue = tracksOfPatterns[activeInstrument].find(newKey)->second;
        tracksOfPatterns[activeInstrument].erase(newKey);
        tracksOfPatterns[activeInstrument].insert(std::make_pair(currentKey, tmpValue));
    }

    tracksOfPatterns[activeInstrument].insert(std::make_pair(newKey, currentValue));

    activePattern = newKey;
}

void AppThreeDee::SelectFirstPatternInTrack()
{
    if (activeInstrument < 0)
    {
        return;
    }

    activePattern = tracksOfPatterns[activeInstrument].begin()->first;
}

void AppThreeDee::SelectLastPatternInTrack()
{
    if (activeInstrument < 0)
    {
        return;
    }

    activePattern = tracksOfPatterns[activeInstrument].rbegin()->first;
}

void AppThreeDee::EditSelectedPattern()
{
    showPatternEditor = true;
}

void AppThreeDee::SelectPreviousPattern()
{
    if (activeInstrument < 0)
    {
        return;
    }

    if (activePattern <= 0)
    {
        return;
    }

    int newIndex = activePattern - 1;
    while (newIndex >= 0)
    {
        if (DoesPatternExistAtIndex(activeInstrument, newIndex))
        {
            activePattern = newIndex;
            break;
        }
        newIndex--;
    }
}

void AppThreeDee::SelectNextPattern()
{
    if (activeInstrument < 0)
    {
        return;
    }

    auto lastIndex = LastPatternIndex(activeInstrument);
    if (activePattern == lastIndex)
    {
        return;
    }

    int newIndex = activePattern + 1;
    while (newIndex <= lastIndex)
    {
        if (DoesPatternExistAtIndex(activeInstrument, newIndex))
        {
            activePattern = newIndex;
            break;
        }
        newIndex++;
    }
}

int AppThreeDee::LastPatternIndex(int trackIndex)
{
    return tracksOfPatterns[trackIndex].empty() ? -1 : tracksOfPatterns[trackIndex].rbegin()->first;
}

bool AppThreeDee::DoesPatternExistAtIndex(int trackIndex, int patternIndex)
{
    return tracksOfPatterns[trackIndex].find(patternIndex) != tracksOfPatterns[trackIndex].end();
}

TrackPattern &AppThreeDee::GetPattern(int trackIndex, int patternIndex)
{
    return tracksOfPatterns[trackIndex][patternIndex];
}

void AppThreeDee::ImGuiSequencer()
{
    ImGuiIO &io = ImGui::GetIO();

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
            ImGui::Text("#%d", trackIndex + 1);
            ImGui::PopID();

            auto lastIndex = LastPatternIndex(trackIndex);
            for (int patternIndex = 0; patternIndex <= lastIndex; patternIndex++)
            {
                ImGui::SameLine();
                ImGui::PushID(patternIndex + trackIndex * 1000);
                bool isActive = trackIndex == activeInstrument && patternIndex == activePattern;
                if (isActive)
                {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                }

                if (DoesPatternExistAtIndex(trackIndex, patternIndex))
                {
                    auto &pattern = GetPattern(trackIndex, patternIndex);
                    ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(pattern._hue, 0.6f, 0.6f)));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(pattern._hue, 0.7f, 0.7f)));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(pattern._hue, 0.8f, 0.8f)));

                    if (ImGui::Button(pattern._name.c_str(), ImVec2(120.0f, trackHeight)))
                    {
                        activeInstrument = trackIndex;
                        activePattern = patternIndex;
                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            EditSelectedPattern();
                        }
                    }
                    ImGui::PopStyleColor(3);
                }
                else if (_mixer->GetChannel(trackIndex)->Penabled)
                {
                    if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
                    {
                        AddPattern(trackIndex, patternIndex, "");
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
                    AddPattern(trackIndex, lastIndex + 1, "");
                }
                ImGui::PopID();
            }
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
            ImGui::BeginChild("scrolling"); // Demonstrate a trick: you can use Begin to set yourself in the context of another window (here we are already out of your child window)
            ImGui::SetScrollX(ImGui::GetScrollX() + scroll_x_delta);
            ImGui::End();
        }

        ImGui::BeginChild("info");
        ImGui::SetScrollY(scroll_y);
        ImGui::EndChild();

        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
        {
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Delete)))
            {
                RemoveActivePattern();
            }
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
            {
                if (io.KeyShift && !io.KeyCtrl)
                {
                    MovePatternLeftForced();
                }
                else if (!io.KeyShift && io.KeyCtrl)
                {
                    SwitchPatternLeft();
                }
                else
                {
                    MovePatternLeftIfPossible();
                }
            }
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
            {
                if (io.KeyShift && !io.KeyCtrl)
                {
                    MovePatternRightForced();
                }
                else if (!io.KeyShift && io.KeyCtrl)
                {
                    SwitchPatternRight();
                }
                else
                {
                    MovePatternRightIfPossible();
                }
            }
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Home)))
            {
                SelectFirstPatternInTrack();
            }
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_End)))
            {
                SelectLastPatternInTrack();
            }
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Enter)))
            {
                EditSelectedPattern();
            }
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Tab)))
            {
                if (io.KeyShift)
                {
                    SelectPreviousPattern();
                }
                else
                {
                    SelectNextPattern();
                }
            }
        }
        ImGui::End();
    }
}

const char *notes[] = {
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
    const float rowHeight = 30.0f;
    if (showPatternEditor)
    {
        auto &style = ImGui::GetStyle();
        auto &selectedPattern = GetPattern(activeInstrument, activePattern);

        ImGui::Begin("PatternEditor", &showPatternEditor);
        auto width = std::fmin(ImGui::GetWindowWidth() - noteLabelWidth, (rowHeight + style.ItemSpacing.x) * 16);
        auto itemWidth = (width / 16) - (style.ItemSpacing.x * 2);
        for (int i = 0; i < 88; i++)
        {
            ImGui::PushID(i);
            ImGui::Button(notes[i % 12], ImVec2(noteLabelWidth, itemWidth));
            for (int j = 0; j < 16; j++)
            {
                ImGui::SameLine();
                ImGui::PushID(j);
                auto found = selectedPattern._notes.find(TrackPatternNote(i, j));
                bool s = found != selectedPattern._notes.end();
                if (ImGui::Selectable("##note", &s, 0, ImVec2(itemWidth, itemWidth)))
                {
                    if (!s)
                    {
                        selectedPattern._notes.erase(TrackPatternNote(i, j));
                    }
                    else
                    {
                        selectedPattern._notes.insert(TrackPatternNote(i, j));
                    }
                }
                ImGui::PopID();
            }
            ImGui::Separator();
            ImGui::PopID();
        }
        ImGui::End();
    }
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
                if (activeInstrument >= 0)
                {
                    ADNoteEditor(_mixer->GetChannel(activeInstrument)->_instruments[0].adpars);
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
                if (activeInstrument >= 0)
                {
                    auto parameters = &_mixer->GetChannel(activeInstrument)->_instruments[0].adpars->VoicePar[i];
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

static std::map<int, unsigned char> mappedNotes{
    {int('Z'), static_cast<unsigned char>(65)},
    {int('X'), static_cast<unsigned char>(66)},
    {int('C'), static_cast<unsigned char>(67)},
    {int('V'), static_cast<unsigned char>(68)},
    {int('B'), static_cast<unsigned char>(69)},
    {int('N'), static_cast<unsigned char>(70)},
    {int('M'), static_cast<unsigned char>(71)},
};

void AppThreeDee::onKeyAction(int key, int /*scancode*/, int action, int /*mods*/)
{
    auto found = mappedNotes.find(key);

    if (found != mappedNotes.end())
    {
        if (action == 1)
        {
            _mixer->NoteOn(static_cast<unsigned char>(keyboardChannel), found->second, 128);
        }
        else if (action == 0)
        {
            _mixer->NoteOff(static_cast<unsigned char>(keyboardChannel), found->second);
        }
    }
}

void AppThreeDee::Stop()
{
}

void AppThreeDee::PlayPause()
{
}

void AppThreeDee::ImGuiPlayback()
{
    ImGui::Begin("Playback");
    {
        if (ImGui::Button("Stop"))
        {
            Stop();
        }

        ImGui::SameLine();

        if (isPlaying)
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
            PlayPause();
        }

        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

        ImGui::End();
    }
}

void AppThreeDee::CleanUp()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
