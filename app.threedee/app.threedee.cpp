#include "app.threedee.h"

#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnoteParams.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_addons/imgui_knob.h"
#include <algorithm>
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

static ImVec4 clear_color = ImColor(114, 144, 154);
static int activeInstrument = 0;
static std::map<int, std::string> tracksOfPatterns[NUM_MIXER_CHANNELS];
static int activePattern = -1;

static bool showInstrumentEditor = false;
static bool show_simple_user_interface = false;
static int keyboardChannel = 0;

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

void AppThreeDee::AddPattern(int trackIndex, int patternIndex, char const *label)
{
    activeInstrument = trackIndex;
    activePattern = patternIndex;
    tracksOfPatterns[trackIndex].insert(std::make_pair(patternIndex, label));
}

void AppThreeDee::RemoveActivePattern()
{
    if (activeInstrument < 0 || activePattern < 0)
    {
        return;
    }

    std::cout << "Removing pattern #" << activePattern << " of track #" << activeInstrument << std::endl;

    tracksOfPatterns[activeInstrument].erase(activePattern);
    activeInstrument = -1;
    activePattern = -1;
}

void AppThreeDee::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuiIO &io = ImGui::GetIO();

    //ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("TEstDockspace", nullptr, window_flags); // Create a window called "Hello, world!" and append into it.

        ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
        ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

        ImGui::End();
        ImGui::PopStyleVar(2);

        ImGui::Begin("Sequencer"); // Create a window called "Hello, world!" and append into it.

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
                char num_buf[16];
                sprintf(num_buf, "%d", pattern);
                float hue = track * 0.05f;
                bool isActive = track == activeInstrument && pattern == activePattern;
                if (isActive)
                {
                    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
                }

                if (tracksOfPatterns[track].find(pattern) != tracksOfPatterns[track].end())
                {
                    ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

                    if (ImGui::Button(tracksOfPatterns[track][pattern].c_str(), ImVec2(120.0f, trackHeight)))
                    {
                        activeInstrument = track;
                        activePattern = pattern;
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
                ImGui::PushID(track * 1010);
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

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Delete)))
    {
        RemoveActivePattern();
    }

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
    if (key >= GLFW_KEY_F1 && key <= GLFW_KEY_F12)
    {
        activeInstrument = keyboardChannel = key - GLFW_KEY_F1;

        return;
    }

    if (key == GLFW_KEY_LEFT && action == 1)
    {
        activeInstrument--;
        if (activeInstrument < 0)
        {
            activeInstrument = NUM_MIDI_CHANNELS + activeInstrument;
        }
        return;
    }

    if (key == GLFW_KEY_UP && action == 1)
    {
        activeInstrument -= 4;
        if (activeInstrument < 0)
        {
            activeInstrument = NUM_MIDI_CHANNELS + activeInstrument;
        }
        return;
    }

    if (key == GLFW_KEY_RIGHT && action == 1)
    {
        activeInstrument = (activeInstrument + 1) % NUM_MIDI_CHANNELS;
        return;
    }

    if (key == GLFW_KEY_DOWN && action == 1)
    {
        activeInstrument = (activeInstrument + 4) % NUM_MIDI_CHANNELS;
        return;
    }

    if (key == GLFW_KEY_SPACE && action == 1)
    {
        _mixer->GetChannel(activeInstrument)->Penabled = !_mixer->GetChannel(activeInstrument)->Penabled;
        return;
    }

    if (key == GLFW_KEY_ENTER && action == 1)
    {
        EditInstrument(activeInstrument);
        return;
    }
    //    std::cout << key << "-" << scancode << "\n";
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

void AppThreeDee::ImGuiSequencer(Track *tracks, int count)
{
    auto spacing = 15;
    auto eventHeight = 17;
    auto buttonWidth = 120;
    auto itemRectCursor = ImVec2();
    auto min = ImVec2();
    auto itemHeight = 0.0f;

    auto maxPatterns = _tracker.maxPatternCount();

    ImGui::Begin("Pattern Sequencer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);
    {
        ImGui::BeginChild("PatternControllers", ImVec2((buttonWidth + spacing) * count, 3 * (eventHeight + ImGui::GetStyle().ColumnsMinSpacing + ImGui::GetStyle().ColumnsMinSpacing)), true, ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::Columns(NUM_MIDI_CHANNELS);

        for (int trackIndex = 0; trackIndex < count; trackIndex++)
        {
            ImGui::SetColumnWidth(trackIndex, buttonWidth + spacing);
            ImGui::PushID(trackIndex);
            auto name = std::string(reinterpret_cast<char *>(_mixer->GetChannel(trackIndex)->Pname));
            ImGui::BeginGroup();
            if (ImGui::Button(name.size() == 0 ? "default" : name.c_str(), ImVec2(buttonWidth, eventHeight)))
            {
                SelectInstrument(trackIndex);
                openSelectInstrument = trackIndex;
            }
            if (ImGui::Button("edit", ImVec2(buttonWidth, eventHeight)))
            {
                EditInstrument(trackIndex);
            }
            if (ImGui::Button("+", ImVec2(buttonWidth, eventHeight)))
            {
                AddPatternToTrack(trackIndex);
            }
            if (ImGui::IsItemHovered())
            {
                ImGui::BeginTooltip();
                ImGui::Text("Insert pattern");
                ImGui::EndTooltip();
            }
            ImGui::PopID();
            ImGui::EndGroup();
            ImGui::NextColumn();

            if (trackIndex == 0)
            {
                min = ImVec2(ImGui::GetItemRectMin().x, itemRectCursor.y);
            }
        }
        ImGui::EndChild();

        ImGui::BeginChild("PatternContainer", ImVec2((buttonWidth + spacing) * count, NUM_PATTERN_EVENTS * maxPatterns * (eventHeight + ImGui::GetStyle().ColumnsMinSpacing + ImGui::GetStyle().ColumnsMinSpacing)), true, ImGuiWindowFlags_NoScrollWithMouse);
        ImGui::Columns(NUM_MIDI_CHANNELS);

        for (int trackIndex = 0; trackIndex < count; trackIndex++)
        {
            ImGui::SetColumnWidth(trackIndex, buttonWidth + spacing);
            ImGui::PushID(trackIndex);
            auto name = std::string(reinterpret_cast<char *>(_mixer->GetChannel(trackIndex)->Pname));
            ImGui::BeginGroup();
            unsigned int step = 0;
            for (int patternIndex = 0; patternIndex < static_cast<int>(tracks[trackIndex]._patterns.size()); patternIndex++)
            {
                auto &pattern = tracks[trackIndex]._patterns[static_cast<size_t>(patternIndex)];
                for (int eventIndex = 0; eventIndex < NUM_PATTERN_EVENTS; eventIndex++)
                {
                    auto &event = pattern._events[eventIndex];
                    ImGui::PushID((trackIndex + 1) * 1000 + (patternIndex + 1) * 100 + eventIndex);
                    bool selected = event._velocity > 0;
                    auto color = ImGui::GetStyleColorVec4(ImGuiCol_Button);
                    if (eventIndex % 4 == 0)
                    {
                        color.w = 1.0f;
                    }
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    if (ImGui::Selectable("...", &selected, ImGuiSelectableFlags_None, ImVec2(buttonWidth, eventHeight)))
                    {
                        event._velocity = selected ? 100 : 0;
                        event._note = selected ? 65 : 0;
                        event._gate = 60;
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                    if (step == _tracker.CurrentStep())
                    {
                        itemRectCursor = ImGui::GetItemRectMin();
                        itemHeight = ImGui::GetItemRectSize().y;
                    }
                    step++;
                }
                ImGui::Text("___");
            }
            ImGui::PopID();
            ImGui::EndGroup();
            ImGui::NextColumn();

            if (trackIndex == 0)
            {
                min = ImVec2(ImGui::GetItemRectMin().x, itemRectCursor.y);
            }
        }
        ImGui::EndChild();

        auto dl = ImGui::GetOverlayDrawList();
        auto width = (buttonWidth + ImGui::GetStyle().ColumnsMinSpacing + ImGui::GetStyle().ColumnsMinSpacing + ImGui::GetStyle().ChildBorderSize + ImGui::GetStyle().ChildBorderSize) * count + ImGui::GetStyle().ChildBorderSize;
        auto max = ImVec2(min.x + width, min.y + itemHeight);
        auto clipMin = ImGui::GetWindowPos();
        clipMin.x += ImGui::GetStyle().FramePadding.x;
        clipMin.y += ImGui::GetStyle().FramePadding.y;
        auto clipMax = ImVec2(clipMin.x + ImGui::GetWindowWidth() - (ImGui::GetStyle().FramePadding.x * 2) - ImGui::GetStyle().ScrollbarSize,
                              clipMin.y + ImGui::GetWindowHeight() - (ImGui::GetStyle().FramePadding.y * 2));

        ImGui::End();
        dl->PushClipRect(clipMin, clipMax);
        dl->AddRectFilled(min, max, ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, 0.2f)));
        dl->PopClipRect();
    }
}

void AppThreeDee::AddPatternToTrack(int trackIndex)
{
    _tracker.Tracks()[trackIndex]._patterns.push_back(Pattern());
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

//    ImGui::Begin("Zynadsubfx", nullptr, ImGuiWindowFlags_NoTitleBar);
//    {
//        MainMenu();
//        const char *channels[] = {
//            "1",
//            "2",
//            "3",
//            "4",
//            "5",
//            "6",
//            "7",
//            "8",
//            "9",
//            "10",
//            "11",
//            "12",
//            "13",
//            "14",
//            "15",
//            "16",
//        };
//        ImGui::Combo("Keyboard channel", &keyboardChannel, channels, NUM_MIDI_CHANNELS);

//        ImGui::BeginGroup();
//        {
//            ImGui::Columns(2);
//            ImGui::SetColumnWidth(0, 70);
//            ImGui::SetColumnWidth(1, 8 * 100);

//            ImGui::Text("MASTER");
//            ImGui::NextColumn();
//            ImGui::Text("CHANNELS");
//            ImGui::NextColumn();
//            ImGui::Separator();

//            ImGui::Text("Input");
//            if (ImGui::Button(Nio::GetSelectedSource().c_str(), ImVec2(55, 20)))
//            {
//                openSelectSinkSource = 1;
//            }
//            ImGui::Text("Output");
//            if (ImGui::Button(Nio::GetSelectedSink().c_str(), ImVec2(55, 20)))
//            {
//                openSelectSinkSource = 1;
//            }

//            auto v = _mixer->Pvolume;
//            if (ImGui::KnobUchar("Volume", &(v), 0, 128, ImVec2(55, 55)))
//            {
//                _mixer->setPvolume(v);
//            }

//            ImGui::NextColumn();

//            ImGui::BeginChild("Channels");
//            {
//                ImGui::Columns(4);
//                for (int i = 0; i < NUM_MIXER_CHANNELS; i++)
//                {
//                    ImGui::PushID(i);

//                    if (i == activeInstrument)
//                    {
//                        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.4f, 0.4f)));
//                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.6f, 0.6f)));
//                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.6f, 0.6f)));
//                    }
//                    else
//                    {
//                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));
//                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
//                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
//                    }

//                    ImVec2 size = ImVec2(87, 18);
//                    char label[32];
//                    sprintf(label, "CH%d", i + 1);
//                    bool channelEnabled = _mixer->GetChannel(i)->Penabled != '\0';
//                    if (ImGui::Checkbox(label, &channelEnabled))
//                    {
//                        _mixer->GetChannel(i)->Penabled = channelEnabled ? 1 : 0;
//                    }

//                    if (ImGui::Button("Edit", size))
//                    {
//                        EditInstrument(i);
//                    }

//                    if (ImGui::Button(reinterpret_cast<char *>(_mixer->GetChannel(i)->Pname), size))
//                    {
//                        SelectInstrument(i);
//                        openSelectInstrument = i;
//                    }

//                    if (ImGui::IsItemHovered() && _mixer->GetChannel(i)->Pname[0] > '\0')
//                    {
//                        ImGui::BeginTooltip();
//                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
//                        ImGui::TextUnformatted(reinterpret_cast<char *>(_mixer->GetChannel(i)->Pname));
//                        ImGui::PopTextWrapPos();
//                        ImGui::EndTooltip();
//                    }
//                    auto volume = _mixer->GetChannel(i)->Pvolume;
//                    if (ImGui::KnobUchar("volume", &(volume), 0, 128, ImVec2(40, 40)))
//                    {
//                        _mixer->GetChannel(i)->setPvolume(volume);
//                    }
//                    ImGui::SameLine();
//                    auto panning = _mixer->GetChannel(i)->Ppanning;
//                    if (ImGui::KnobUchar(" pann", &(panning), 0, 128, ImVec2(40, 40)))
//                    {
//                        _mixer->GetChannel(i)->setPpanning(panning);
//                    }
//                    ImGui::PopStyleColor(3);
//                    ImGui::PopID();

//                    ImGui::NextColumn();
//                    if ((i + 1) % 4 == 0)
//                    {
//                        ImGui::Separator();
//                    }
//                }
//            }
//            ImGui::EndChild();
//        }
//        ImGui::EndGroup();
//    }
//    ImGui::End();

//    ImGuiSequencer(_tracker.Tracks(), NUM_MIDI_CHANNELS);

//    if (showAddSynthEditor)
//    {
//        ADNoteEditor(_mixer->GetChannel(activeInstrument)->_instruments[0].adpars);
//    }

//    if (showInstrumentEditor)
//    {
//        if (ImGui::Begin("Instrument editor", &showInstrumentEditor))
//        {
//            ImGui::Columns(2);

//            const char *listbox_items[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
//            ImGui::Combo("Part", &activeInstrument, listbox_items, 15);

//            bool penabled = _mixer->GetChannel(activeInstrument)->Penabled != 0;
//            if (ImGui::Checkbox("Enabled", &penabled))
//            {
//                _mixer->GetChannel(activeInstrument)->Penabled = penabled ? 1 : 0;
//            }

//            bool add = _mixer->GetChannel(activeInstrument)->_instruments[0].Padenabled;
//            if (ImGui::Checkbox("Add", &add))
//            {
//                _mixer->GetChannel(activeInstrument)->_instruments[0].Padenabled = add;
//            }
//            ImGui::SameLine();
//            if (ImGui::Button("Edit"))
//            {
//                EditADSynth(activeInstrument);
//            }

//            bool sub = _mixer->GetChannel(activeInstrument)->_instruments[0].Psubenabled;
//            if (ImGui::Checkbox("Sub", &sub))
//            {
//                _mixer->GetChannel(activeInstrument)->_instruments[0].Psubenabled = sub;
//            }

//            bool pad = _mixer->GetChannel(activeInstrument)->_instruments[0].Ppadenabled;
//            if (ImGui::Checkbox("Pad", &pad))
//            {
//                _mixer->GetChannel(activeInstrument)->_instruments[0].Ppadenabled = pad;
//            }

//            if (ImGui::Button(reinterpret_cast<char *>(_mixer->GetChannel(activeInstrument)->Pname)))
//            {
//                SelectInstrument(activeInstrument);
//                openSelectInstrument = activeInstrument;
//            }
//            ImGui::NextColumn();
//            ImGui::Columns(1);

//            static bool noteOn = false;
//            if (ImGui::Button("Note On") && !noteOn)
//            {
//                _mixer->NoteOn(0, 65, 120);
//                noteOn = true;
//            }
//            if (ImGui::Button("Note Off") && noteOn)
//            {
//                _mixer->NoteOff(0, 65);
//                noteOn = false;
//            }
//            ImGui::End();
//        }
//    }

//    if (openSelectInstrument >= 0)
//    {
//        ImGui::OpenPopup("popup");
//    }

//    ImGui::SetNextWindowSize(ImVec2(700, 600));
//    if (ImGui::BeginPopupModal("popup"))
//    {
//        auto count = _mixer->GetBankManager()->GetBankCount();
//        std::vector<const char *> bankNames;
//        bankNames.push_back("");
//        for (int i = 0; i < count; i++)
//        {
//            bankNames.push_back(_mixer->GetBankManager()->GetBank(i).name.c_str());
//        }
//        static int currentBank = 0;
//        if (ImGui::Combo("Bank", &currentBank, &(bankNames[0]), int(count)))
//        {
//            _mixer->GetBankManager()->LoadBank(currentBank - 1);
//        }
//        ImGui::SameLine();

//        if (ImGui::Button("Close"))
//        {
//            openSelectInstrument = -1;
//            ImGui::CloseCurrentPopup();
//        }

//        ImGui::Columns(5);
//        if (currentBank > 0)
//        {
//            for (unsigned int i = 0; i < BANK_SIZE; i++)
//            {
//                auto instrumentName = _mixer->GetBankManager()->GetName(i);

//                if (ImGui::Button(instrumentName.c_str(), ImVec2(120, 20)))
//                {
//                    auto const &instrument = _mixer->GetChannel(activeInstrument);
//                    instrument->Lock();
//                    _mixer->GetBankManager()->LoadFromSlot(i, instrument);
//                    instrument->Unlock();
//                    instrument->ApplyParameters();
//                    openSelectInstrument = -1;
//                    ImGui::CloseCurrentPopup();
//                }
//                if ((i + 1) % 32 == 0)
//                {
//                    ImGui::NextColumn();
//                }
//            }
//        }
//        ImGui::EndPopup();
//    }

//    if (openSelectSinkSource >= 0)
//    {
//        ImGui::OpenPopup("NioPopup");
//    }

//    if (ImGui::BeginPopupModal("NioPopup"))
//    {
//        static int selectedSink = 0, selectedSource = 0;
//        int sinkIndex = 0;
//        for (auto sink : Nio::GetSinks())
//        {
//            if (ImGui::RadioButton(sink.c_str(), &selectedSink, int(sinkIndex++)))
//            {
//                Nio::SelectSink(sink);
//            }
//        }
//        int sourceIndex = 0;
//        for (auto source : Nio::GetSources())
//        {
//            if (ImGui::RadioButton(source.c_str(), &selectedSource, int(sourceIndex++)))
//            {
//                Nio::SelectSource(source);
//            }
//        }

//        if (ImGui::Button("Close"))
//        {
//            openSelectSinkSource = -1;
//            ImGui::CloseCurrentPopup();
//        }
//        ImGui::EndPopup();
//    }
