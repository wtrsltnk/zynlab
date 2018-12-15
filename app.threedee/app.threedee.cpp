#include "app.threedee.h"

#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnoteParams.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_addons/imgui_knob.h"
#include <algorithm>
#include <cstdlib>
#include <map>

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _mixer(mixer), _window(window), _display_w(800), _display_h(600),
      _tracker(mixer->GetSettings()), showAddSynthEditor(false)
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

    _tracker.SetBpm(120);

    return true;
}

class TrackPattern
{
public:
    TrackPattern() {}
    TrackPattern(std::string const &name, float hue) : _name(name), _hue(hue) {}
    std::string _name;
    float _hue;
};

static ImVec4 clear_color = ImColor(114, 144, 154);
static int activeInstrument = 0;
static std::map<int, TrackPattern> tracksOfPatterns[NUM_MIXER_CHANNELS];
static int activePattern = -1;

static bool showInstrumentEditor = false;
static bool show_simple_user_interface = false;
static int keyboardChannel = 0;
static bool showPatternEditor = false;

void MainMenu();

void AppThreeDee::EditInstrument(int i)
{
    if (i < 0 || i >= NUM_MIXER_CHANNELS)
    {
        return;
    }

    _mixer->GetChannel(i)->Penabled = true;

    activeInstrument = i;
    showInstrumentEditor = true;
}

void AppThreeDee::EditADSynth(int i)
{
    if (i < 0 || i >= NUM_MIXER_CHANNELS)
    {
        return;
    }

    _mixer->GetChannel(i)->Penabled = true;

    activeInstrument = i;
    showAddSynthEditor = true;
}

void AppThreeDee::SelectInstrument(int i)
{
    activeInstrument = i;
}

static int openSelectInstrument = -1;
static int openSelectSinkSource = -1;

static bool show_demo_window = true;
static bool show_another_window = true;
static bool show_pattern_window = false;

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

    if (show_pattern_window)
    {
        ImGui::Begin("Pattern Window", &show_pattern_window);
        ImGui::Text("Hello from pattern window!");
        if (ImGui::Button("Close Me"))
            show_pattern_window = false;
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window); // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;

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
    _tracker.Stop();
}

void AppThreeDee::PlayPause()
{
    if (_tracker.IsPlaying())
    {
        _tracker.Pause();
    }
    else
    {
        _tracker.Play();
    }
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

        if (_tracker.IsPlaying())
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

void AppThreeDee::ImGuiSequencer()
{
    ImGuiIO &io = ImGui::GetIO();

    ImGui::Begin("Sequencer");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 10.0f));

        static int trackHeight = 30;
        ImGui::BeginChild("scrolling", ImVec2(0, -30), false, ImGuiWindowFlags_HorizontalScrollbar);
        for (int track = 0; track < NUM_MIXER_CHANNELS; track++)
        {
            ImGui::PushID(track * 1100);
            auto trackEnabled = _mixer->GetChannel(track)->Penabled == 1;
            if (ImGui::Checkbox("##trackEnabled", &trackEnabled))
            {
                _mixer->GetChannel(track)->Penabled = trackEnabled ? 1 : 0;
            }
            ImGui::SameLine();
            ImGui::Text("#%d", track + 1);
            ImGui::PopID();

            auto lastIndex = tracksOfPatterns[track].empty() ? -1 : tracksOfPatterns[track].rbegin()->first;
            for (int pattern = 0; pattern <= lastIndex; pattern++)
            {
                ImGui::SameLine();
                ImGui::PushID(pattern + track * 1000);
                bool isActive = track == activeInstrument && pattern == activePattern;
                if (isActive)
                {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                }

                if (tracksOfPatterns[track].find(pattern) != tracksOfPatterns[track].end())
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(tracksOfPatterns[track][pattern]._hue, 0.6f, 0.6f)));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(tracksOfPatterns[track][pattern]._hue, 0.7f, 0.7f)));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(tracksOfPatterns[track][pattern]._hue, 0.8f, 0.8f)));

                    if (ImGui::Button(tracksOfPatterns[track][pattern]._name.c_str(), ImVec2(120.0f, trackHeight)))
                    {
                        activeInstrument = track;
                        activePattern = pattern;
                        if (ImGui::IsMouseDoubleClicked(0))
                        {
                            show_pattern_window = true;
                        }
                    }
                    ImGui::PopStyleColor(3);
                }
                else if (_mixer->GetChannel(track)->Penabled)
                {
                    if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
                    {
                        AddPattern(track, pattern, "");
                    }
                }
                if (isActive)
                {
                    ImGui::PopStyleColor(1);
                    ImGui::PopStyleVar(1);
                }

                ImGui::PopID();
            }
            if (_mixer->GetChannel(track)->Penabled)
            {
                ImGui::SameLine();
                ImGui::PushID((100 + track) * 2010);
                if (ImGui::Button("+", ImVec2(120.0f, trackHeight)))
                {
                    AddPattern(track, lastIndex + 1, "");
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
                if (io.KeyShift)
                {
                    MovePatternLeftForced();
                }
                else
                {
                    MovePatternLeftIfPossible();
                }
            }
            if (ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
            {
                if (io.KeyShift)
                {
                    MovePatternRightForced();
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
        }
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

void MainMenu()
{
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New (erase all)..."))
            {
            }
            if (ImGui::MenuItem("Open parameters..."))
            {
            }
            if (ImGui::MenuItem("Save All parameters..."))
            {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Load Scale Settings..."))
            {
            }
            if (ImGui::MenuItem("Save Scale Settings..."))
            {
            }
            if (ImGui::MenuItem("Show Scale Settings..."))
            {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Settings..."))
            {
            }
            if (ImGui::MenuItem("Copyrights..."))
            {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit"))
            {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Instrument"))
        {
            if (ImGui::MenuItem("Clear instrument..."))
            {
            }
            if (ImGui::MenuItem("Open instrument..."))
            {
            }
            if (ImGui::MenuItem("Save instrument..."))
            {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Show instrument Bank..."))
            {
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Virtual keyboard..."))
            {
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Misc"))
        {
            ImGui::MenuItem("Switch User Interface Mode", nullptr, &show_simple_user_interface);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("NIO"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
}
