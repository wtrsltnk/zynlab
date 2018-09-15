
#include "app.threedee.h"
#include <zyn.mixer/Instrument.h>

#include <imgui.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include <algorithm>
#include <map>

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _mixer(mixer), _window(window), _display_w(800), _display_h(600)
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

    _mixer->bank.rescanforbanks();

    return true;
}

// Implementing a simple custom widget using the public API.
// You may also use the <imgui_internal.h> API to get raw access to more data/helpers, however the internal API isn't guaranteed to be forward compatible.
// FIXME: Need at least proper label centering + clipping (internal functions RenderTextClipped provides both but api is flaky/temporary)
static bool MyKnob(const char *label, float *p_value, float v_min, float v_max)
{
    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();

    float radius_outer = 20.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);
    float line_height = ImGui::GetTextLineHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.75f;
    float ANGLE_MAX = 3.141592f * 2.25f;

    ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + line_height + style.ItemInnerSpacing.y));
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

    float t = (*p_value - v_min) / (v_max - v_min);
    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * t;
    float angle_cos = cosf(angle), angle_sin = sinf(angle);
    float radius_inner = radius_outer * 0.40f;
    draw_list->AddCircleFilled(center, radius_outer, ImGui::GetColorU32(ImGuiCol_FrameBg), 16);
    draw_list->AddLine(ImVec2(center.x + angle_cos * radius_inner, center.y + angle_sin * radius_inner), ImVec2(center.x + angle_cos * (radius_outer - 2), center.y + angle_sin * (radius_outer - 2)), ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
    draw_list->AddCircleFilled(center, radius_inner, ImGui::GetColorU32(is_active ? ImGuiCol_FrameBgActive : is_hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), 16);
    draw_list->AddText(ImVec2(pos.x, pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - line_height - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        ImGui::Text("%s %.3f", label, *p_value);
        ImGui::EndTooltip();
    }

    return value_changed;
}

static bool MyKnobUchar(const char *label, char *p_value, unsigned char v_min, unsigned char v_max)
{
    float val = (p_value[0]) / 128.0f;

    if (MyKnob(label, &val, v_min / 128.0f, v_max / 128.0f))
    {
        p_value[0] = static_cast<char>(val * 128);

        return true;
    }

    return false;
}

static bool MyKnobUchar(const char *label, unsigned char *p_value, unsigned char v_min, unsigned char v_max)
{
    float val = (p_value[0]) / 128.0f;

    if (MyKnob(label, &val, v_min / 128.0f, v_max / 128.0f))
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
    if (i < 0 || i >= NUM_MIDI_PARTS)
    {
        return;
    }

    _mixer->part[i]->Penabled = true;

    activeInstrument = i;
    showInstrumentEditor = true;
    std::cout << "EditInstrument: " << i << std::endl;
}

void AppThreeDee::SelectInstrument(int i)
{
    activeInstrument = i;
    std::cout << "SelectInstrument: " << i << std::endl;
}

void AppThreeDee::Render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    int openSelectInstrument = -1;

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

            ImGui::Button("MASTER", ImVec2(50, 20));
            ImGui::Button(" ", ImVec2(50, 20));
            char v = char(_mixer->Pvolume);
            if (MyKnobUchar("Master", &(v), 0, 128))
            {
                _mixer->setPvolume(v);
            }

            ImGui::NextColumn();

            ImGui::BeginChild("Channels");
            {
                ImGui::Columns(4);
                for (int i = 0; i < NUM_MIDI_PARTS; i++)
                {
                    ImGui::PushID(i);

                    if (i == activeInstrument)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 0.4f, 0.4f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1.0f, 0.6f, 0.6f));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1.0f, 0.6f, 0.6f));
                    }
                    else
                    {
                        ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_Button));
                        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
                        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
                    }

                    ImVec2 size = ImVec2(85, 20);
                    char label[32];
                    sprintf(label, "CH%d", i + 1);
                    bool channelEnabled = _mixer->part[i]->Penabled != '\0';
                    if (ImGui::Checkbox(label, &channelEnabled))
                    {
                        _mixer->part[i]->Penabled = channelEnabled ? 1 : 0;
                    }
                    if (ImGui::Button("Edit", size))
                    {
                        EditInstrument(i);
                    }

                    if (ImGui::Button((char *)_mixer->part[i]->Pname, size))
                    {
                        SelectInstrument(i);
                        openSelectInstrument = i;
                    }

                    if (ImGui::IsItemHovered() && _mixer->part[i]->Pname[0] > '\0')
                    {
                        ImGui::BeginTooltip();
                        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
                        ImGui::TextUnformatted((char *)_mixer->part[i]->Pname);
                        ImGui::PopTextWrapPos();
                        ImGui::EndTooltip();
                    }
                    auto volume = _mixer->part[i]->Pvolume;
                    if (MyKnobUchar("volume", &(volume), 0, 128))
                    {
                        _mixer->part[i]->setPvolume(volume);
                    }
                    ImGui::SameLine();
                    auto panning = _mixer->part[i]->Ppanning;
                    if (MyKnobUchar(" pann", &(panning), 0, 128))
                    {
                        _mixer->part[i]->setPpanning(panning);
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
        ImGui::Begin("Instrument editor", &showInstrumentEditor);

        ImGui::Columns(2);

        const char *listbox_items[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14", "15", "16"};
        ImGui::Combo("Part", &activeInstrument, listbox_items, 15);

        bool penabled = _mixer->part[activeInstrument]->Penabled != 0;
        if (ImGui::Checkbox("Enabled", &penabled))
        {
            _mixer->part[activeInstrument]->Penabled = penabled ? 1 : 0;
        }

        MyKnobUchar("Volume", &(_mixer->part[activeInstrument]->Pvolume), 0, 128);
        ImGui::SameLine();
        MyKnobUchar("Pan", &(_mixer->part[activeInstrument]->Ppanning), 0, 128);
        ImGui::SameLine();
        MyKnobUchar("Vel.Sns", &(_mixer->part[activeInstrument]->Pvelsns), 0, 128);
        ImGui::SameLine();
        MyKnobUchar("Vel.Ofs", &(_mixer->part[activeInstrument]->Pveloffs), 0, 128);

        if (ImGui::Button("Edit"))
        {
            EditInstrument(activeInstrument);
        }

        if (ImGui::Button((char *)_mixer->part[activeInstrument]->Pname))
        {
            SelectInstrument(activeInstrument);
            openSelectInstrument = activeInstrument;
        }
        ImGui::NextColumn();
        ImGui::Text("To Sys.Efx.");
        MyKnobUchar("1", &(_mixer->part[activeInstrument]->Pefxroute[0]), 0, 128);
        MyKnobUchar("2", &(_mixer->part[activeInstrument]->Pefxroute[1]), 0, 128);
        MyKnobUchar("3", &(_mixer->part[activeInstrument]->Pefxroute[2]), 0, 128);
        MyKnobUchar("4", &(_mixer->part[activeInstrument]->Pefxroute[3]), 0, 128);

        ImGui::Columns(1);

        ImGui::Button("Reset");
        MyKnobUchar("Find Detune", &(_mixer->microtonal.Pglobalfinedetune), 0, 128);

        static bool noteOn = false;
        if (ImGui::Button("Panic!") && !noteOn)
        {
            _mixer->NoteOn(0, 65, 120);
            noteOn = true;
        }
        if (ImGui::Button("vK") && noteOn)
        {
            _mixer->NoteOff(0, 65);
            noteOn = false;
        }
        ImGui::SameLine();
        ImGui::Button("Scales");
        ImGui::Button("Panel Window");

        int t = int(_mixer->Pkeyshift);
        if (ImGui::SliderInt("Master KeyShift", &t, 0, 64))
        {
            _mixer->Pkeyshift = static_cast<unsigned char>(t);
        }

        ImGui::Separator();

        enum EffectsTab
        {
            SystemEffects,
            InsertionEffects
        };

        static EffectsTab effectsTab = EffectsTab::SystemEffects;
        if (ImGui::Button("System Effects"))
        {
            effectsTab = EffectsTab::SystemEffects;
        }
        ImGui::SameLine();
        if (ImGui::Button("Insertion Effects"))
        {
            effectsTab = EffectsTab::InsertionEffects;
        }
        if (effectsTab == EffectsTab::SystemEffects)
        {
            ImGui::Text("SystemEffects");
        }
        if (effectsTab == EffectsTab::InsertionEffects)
        {
            ImGui::Text("InsertionEffects");
        }
        ImGui::End();
    }

    if (openSelectInstrument >= 0)
    {
        ImGui::OpenPopup("popup");
    }

    ImGui::SetNextWindowSize(ImVec2(700, 600));
    if (ImGui::BeginPopupModal("popup"))
    {
        auto count = _mixer->bank.banks.size();
        std::vector<const char *> bankNames;
        bankNames.push_back("");
        for (int i = 0; i < int(_mixer->bank.banks.size()); i++)
        {
            bankNames.push_back(_mixer->bank.banks[static_cast<size_t>(i)].name.c_str());
        }
        static int currentBank = 0;
        if (ImGui::Combo("Bank", &currentBank, &(bankNames[0]), int(count)))
        {
            _mixer->bank.loadbank(_mixer->bank.banks[static_cast<size_t>(currentBank - 1)].dir);
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
                auto instrumentName = _mixer->bank.getname(i);

                if (ImGui::Button(instrumentName.c_str(), ImVec2(120, 20)))
                {
                    pthread_mutex_lock(&_mixer->part[activeInstrument]->load_mutex);
                    _mixer->bank.loadfromslot(i, _mixer->part[activeInstrument]);
                    pthread_mutex_unlock(&_mixer->part[activeInstrument]->load_mutex);
                    _mixer->part[activeInstrument]->applyparameters();
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

    ImGui::Render();
    int display_w, display_h;
    glfwMakeContextCurrent(_window);
    glfwGetFramebufferSize(_window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

static std::map<int, char> mappedNotes{
    {int('Z'), static_cast<char>(65)},
    {int('X'), static_cast<char>(66)},
    {int('C'), static_cast<char>(67)},
    {int('V'), static_cast<char>(68)},
    {int('B'), static_cast<char>(69)},
    {int('N'), static_cast<char>(70)},
    {int('M'), static_cast<char>(71)},
};

void AppThreeDee::onKeyAction(int key, int scancode, int action, int /*mods*/)
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
        _mixer->part[activeInstrument]->Penabled = !_mixer->part[activeInstrument]->Penabled;
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
            _mixer->NoteOn(keyboardChannel, found->second, 128);
        }
        else if (action == 0)
        {
            _mixer->NoteOff(keyboardChannel, found->second);
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
