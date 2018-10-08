
#include "app.threedee.h"
#include <zyn.mixer/Instrument.h>
#include <zyn.synth/ADnoteParams.h>

#include <imgui.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include <algorithm>
#include <map>

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _mixer(mixer), _window(window), _display_w(800), _display_h(600), _isPlaying(false), _currentStep(0)
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

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Setup style
    ImGui::StyleColorsDark();

    _mixer->GetBankManager()->RescanForBanks();

    return true;
}

// Implementing a simple custom widget using the public API.
// You may also use the <imgui_internal.h> API to get raw access to more data/helpers, however the internal API isn't guaranteed to be forward compatible.
// FIXME: Need at least proper label centering + clipping (internal functions RenderTextClipped provides both but api is flaky/temporary)
static bool MyKnob(const char *label, float *p_value, float v_min, float v_max, ImVec2 const &size)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    float radius_outer = std::min(size.x, size.y) / 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);

    float line_height = ImGui::GetTextLineHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.75f;
    float ANGLE_MAX = 3.141592f * 2.25f;

    if (size.x != 0.0f && size.y != 0.0f)
    {
        center.x = pos.x + (size.x / 2.0f);
        center.y = pos.y + (size.y / 2.0f);
        ImGui::InvisibleButton(label, ImVec2(size.x, size.y + line_height + style.ItemInnerSpacing.y));
    }
    else
    {
        ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + line_height + style.ItemInnerSpacing.y));
    }
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.x != 0.0f)
    {
        float step = (v_max - v_min) / 200.0f;
        *p_value += io.MouseDelta.x * step;
        if (*p_value < v_min)
            *p_value = v_min;
        if (*p_value > v_max)
            *p_value = v_max;
        value_changed = true;
    }

    float radius_inner = radius_outer * 0.40f;

    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * (*p_value - v_min) / (v_max - v_min);
    float angle_cos = cosf(angle);
    float angle_sin = sinf(angle);

    draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImGuiCol_FrameBg), 16);
    draw_list->AddLine(
        ImVec2(center.x + angle_cos * radius_inner, center.y + angle_sin * radius_inner),
        ImVec2(center.x + angle_cos * (radius_outer - 2), center.y + angle_sin * (radius_outer - 2)),
        ImGui::GetColorU32(ImGuiCol_SliderGrabActive),
        2.0f);
    draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    draw_list->AddText(ImVec2(pos.x, pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - line_height - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        ImGui::Text("%s %.3f", label, static_cast<double>(*p_value));
        ImGui::EndTooltip();
    }

    return value_changed;
}

static bool MyKnobUchar(const char *label, unsigned char *p_value, unsigned char v_min, unsigned char v_max, ImVec2 const &size)
{
    float val = (p_value[0]) / 128.0f;

    if (MyKnob(label, &val, v_min / 128.0f, v_max / 128.0f, size))
    {
        p_value[0] = static_cast<unsigned char>(val * 128);

        return true;
    }

    return false;
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

void AppThreeDee::SelectInstrument(int i)
{
    activeInstrument = i;
}

void AppThreeDee::StepSequencer()
{
    const double stepTime = 1.0 / 5.0;

    static std::vector<bool> steps;
    static int lastStepTriggered = -1;
    static double lastTime = glfwGetTime();
    static double noteOffAt = 0;
    static float gate = 0.7f;
    double currentTime = glfwGetTime();

    if (currentTime - lastTime > stepTime)
    {
        if (_isPlaying)
        {
            _currentStep++;
            if (_currentStep >= static_cast<int>(steps.size()))
            {
                _currentStep = 0;
            }
        }

        lastTime += stepTime;
    }

    if (_isPlaying && lastStepTriggered != _currentStep)
    {
        if (steps[_currentStep])
        {
            _mixer->NoteOn(0, 65, 128);
            noteOffAt = currentTime + (stepTime * gate);
        }
        lastStepTriggered = _currentStep;
    }

    if (noteOffAt <= currentTime)
    {
        _mixer->NoteOff(0, 65);
    }

    ImGui::Begin("Step Sequencer", nullptr, ImGuiWindowFlags_NoTitleBar);
    {
        if (ImGui::Checkbox("Play/Pause", &_isPlaying))
        {
            if (steps.empty())
            {
                _isPlaying = false;
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Stop"))
        {
            _isPlaying = false;
            _currentStep = 0;
        }
        int stepSize = static_cast<int>(steps.size());
        if (ImGui::InputInt("Steps", &stepSize))
        {
            while (!steps.empty() && static_cast<int>(steps.size()) > stepSize)
            {
                steps.pop_back();
            }
            while (static_cast<int>(steps.size()) < stepSize)
            {
                steps.push_back(0);
            }
        }
        ImGui::SliderFloat("Gate", &gate, 0.1f, 1.0f);

        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Border, 0xFFFFFFFF);
        for (size_t i = 0; i < steps.size(); i++)
        {
            ImGui::PushID(static_cast<int>(i));
            ImGui::PushStyleColor(ImGuiCol_Text, static_cast<int>(i) == _currentStep ? 0xFFFFFFFF : 0x00FF00FF);
            bool b = steps[i];
            if (ImGui::Selectable("X", &b, 0, ImVec2(50, 50)))
            {
                steps[i] = b;
            }
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::PopStyleColor();
        ImGui::End();
    }
}

void AppThreeDee::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int openSelectInstrument = -1;
    int openSelectSinkSource = -1;

    StepSequencer();

    ImGui::Begin("Zynadsubfx", nullptr, ImGuiWindowFlags_NoTitleBar);
    {
        MainMenu();
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
        ImGui::Combo("Keyboard channel", &keyboardChannel, channels, NUM_MIDI_CHANNELS);

        ImGui::BeginGroup();
        {
            ImGui::Columns(2);
            ImGui::SetColumnWidth(0, 70);
            ImGui::SetColumnWidth(1, 8 * 100);

            ImGui::Text("MASTER");
            ImGui::NextColumn();
            ImGui::Text("CHANNELS");
            ImGui::NextColumn();
            ImGui::Separator();

            ImGui::Text("Input");
            if (ImGui::Button(Nio::GetSelectedSource().c_str(), ImVec2(55, 20)))
            {
                openSelectSinkSource = 1;
            }
            ImGui::Text("Output");
            if (ImGui::Button(Nio::GetSelectedSink().c_str(), ImVec2(55, 20)))
            {
                openSelectSinkSource = 1;
            }

            auto v = _mixer->Pvolume;
            if (MyKnobUchar("Volume", &(v), 0, 128, ImVec2(55, 55)))
            {
                _mixer->setPvolume(v);
            }

            ImGui::NextColumn();

            ImGui::BeginChild("Channels");
            {
                ImGui::Columns(4);
                for (int i = 0; i < NUM_MIXER_CHANNELS; i++)
                {
                    ImGui::PushID(i);

                    if (i == activeInstrument)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.4f, 0.4f)));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.6f, 0.6f)));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(1.0f, 0.6f, 0.6f)));
                    }
                    else
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
                    }

                    ImVec2 size = ImVec2(87, 18);
                    char label[32];
                    sprintf(label, "CH%d", i + 1);
                    bool channelEnabled = _mixer->GetChannel(i)->Penabled != '\0';
                    if (ImGui::Checkbox(label, &channelEnabled))
                    {
                        _mixer->GetChannel(i)->Penabled = channelEnabled ? 1 : 0;
                    }

                    if (ImGui::Button("Edit", size))
                    {
                        EditInstrument(i);
                    }

                    if (ImGui::Button(reinterpret_cast<char *>(_mixer->GetChannel(i)->Pname), size))
                    {
                        SelectInstrument(i);
                        openSelectInstrument = i;
                    }

                    if (ImGui::IsItemHovered() && _mixer->GetChannel(i)->Pname[0] > '\0')
                    {
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        ImGui::TextUnformatted(reinterpret_cast<char *>(_mixer->GetChannel(i)->Pname));
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    auto volume = _mixer->GetChannel(i)->Pvolume;
                    if (MyKnobUchar("volume", &(volume), 0, 128, ImVec2(40, 40)))
                    {
                        _mixer->GetChannel(i)->setPvolume(volume);
                    }
                    ImGui::SameLine();
                    auto panning = _mixer->GetChannel(i)->Ppanning;
                    if (MyKnobUchar(" pann", &(panning), 0, 128, ImVec2(40, 40)))
                    {
                        _mixer->GetChannel(i)->setPpanning(panning);
                    }
                    ImGui::PopStyleColor(3);
                    ImGui::PopID();

                    ImGui::NextColumn();
                    if ((i + 1) % 4 == 0)
                    {
                        ImGui::Separator();
                    }
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndGroup();
    }
    ImGui::End();

    if (showInstrumentEditor)
    {
        if (ImGui::Begin("Instrument editor", &showInstrumentEditor))
        {
            ImGui::Columns(2);

            const char *listbox_items[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
            ImGui::Combo("Part", &activeInstrument, listbox_items, 15);

            bool penabled = _mixer->GetChannel(activeInstrument)->Penabled != 0;
            if (ImGui::Checkbox("Enabled", &penabled))
            {
                _mixer->GetChannel(activeInstrument)->Penabled = penabled ? 1 : 0;
            }

            bool add = _mixer->GetChannel(activeInstrument)->kit[0].Padenabled;
            if (ImGui::Checkbox("Add", &add))
            {
                _mixer->GetChannel(activeInstrument)->kit[0].Padenabled = add;
            }

            bool sub = _mixer->GetChannel(activeInstrument)->kit[0].Psubenabled;
            if (ImGui::Checkbox("Sub", &sub))
            {
                _mixer->GetChannel(activeInstrument)->kit[0].Psubenabled = sub;
            }

            bool pad = _mixer->GetChannel(activeInstrument)->kit[0].Ppadenabled;
            if (ImGui::Checkbox("Pad", &pad))
            {
                _mixer->GetChannel(activeInstrument)->kit[0].Ppadenabled = pad;
            }

            if (ImGui::Button("Edit"))
            {
                EditInstrument(activeInstrument);
            }

            if (ImGui::Button(reinterpret_cast<char *>(_mixer->GetChannel(activeInstrument)->Pname)))
            {
                SelectInstrument(activeInstrument);
                openSelectInstrument = activeInstrument;
            }
            ImGui::NextColumn();
            ImGui::Columns(1);

            static bool noteOn = false;
            if (ImGui::Button("Note On") && !noteOn)
            {
                _mixer->NoteOn(0, 65, 120);
                noteOn = true;
            }
            if (ImGui::Button("Note Off") && noteOn)
            {
                _mixer->NoteOff(0, 65);
                noteOn = false;
            }
            ImGui::End();
        }
    }

    if (openSelectInstrument >= 0)
    {
        ImGui::OpenPopup("popup");
    }

    ImGui::SetNextWindowSize(ImVec2(700, 600));
    if (ImGui::BeginPopupModal("popup"))
    {
        auto count = _mixer->GetBankManager()->GetBankCount();
        std::vector<const char *> bankNames;
        bankNames.push_back("");
        for (int i = 0; i < count; i++)
        {
            bankNames.push_back(_mixer->GetBankManager()->GetBank(i).name.c_str());
        }
        static int currentBank = 0;
        if (ImGui::Combo("Bank", &currentBank, &(bankNames[0]), int(count)))
        {
            _mixer->GetBankManager()->LoadBank(currentBank - 1);
        }
        ImGui::SameLine();

        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::Columns(5);
        if (currentBank > 0)
        {
            for (unsigned int i = 0; i < BANK_SIZE; i++)
            {
                auto instrumentName = _mixer->GetBankManager()->GetName(i);

                if (ImGui::Button(instrumentName.c_str(), ImVec2(120, 20)))
                {
                    pthread_mutex_lock(&_mixer->GetChannel(activeInstrument)->load_mutex);
                    _mixer->GetBankManager()->LoadFromSlot(i, _mixer->GetChannel(activeInstrument));
                    pthread_mutex_unlock(&_mixer->GetChannel(activeInstrument)->load_mutex);
                    _mixer->GetChannel(activeInstrument)->applyparameters();
                    ImGui::CloseCurrentPopup();
                }
                if ((i + 1) % 32 == 0)
                {
                    ImGui::NextColumn();
                }
            }
        }
        ImGui::EndPopup();
    }

    if (openSelectSinkSource >= 0)
    {
        ImGui::OpenPopup("NioPopup");
    }

    if (ImGui::BeginPopupModal("NioPopup"))
    {
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    ImGui::Render();
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
