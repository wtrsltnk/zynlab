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

bool show_demo_window = true;
bool show_another_window = false;

void AppThreeDee::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
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

    ImGuiPlayback();

    ImGui::Render();

    ImGuiIO &io = ImGui::GetIO();
    (void)io;
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

        //MyKnob("Speed", &_stepLength, 1.0f, 120.0f, ImVec2(30.0f, 30.0f));

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
