#include "app.drummer.h"

#include <zyn.mixer/Track.h>
#include <zyn.synth/ADnoteParams.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_addons/imgui_Timeline.h"
#include "imgui_addons/imgui_checkbutton.h"
#include "imgui_addons/imgui_knob.h"
#include "stb_image.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>

const char *NoteNames[] = {
    "C",
    "C#",
    "D",
    "D#",
    "E",
    "F",
    "F#",
    "G",
    "G#",
    "A",
    "A#",
    "B",
};

int NoteNameCount = 12;

static ImVec4 clear_color = ImColor(90, 90, 100);

AppDrummer::AppDrummer(GLFWwindow *window, Mixer *mixer, IBankManager *banks)
    : _state(mixer, &_stepper, banks), _effectUi(&_state), _libraryUi(&_state), _mixerUi(&_state), _sequencerUi(&_state),
      _window(window), _stepper(&_state._sequencer, mixer),
      _display_w(800), _display_h(600)
{
    glfwSetWindowUserPointer(this->_window, static_cast<void *>(this));
}

AppDrummer::~AppDrummer()
{
    glfwSetWindowUserPointer(this->_window, nullptr);
}

void AppDrummer::KeyActionCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto app = static_cast<AppDrummer *>(glfwGetWindowUserPointer(window));

    if (app != nullptr)
    {
        app->onKeyAction(key, scancode, action, mods);
        ImGui_ImplGlfw_KeyCallback(app->_window, key, scancode, action, mods);
    }
}

void AppDrummer::ResizeCallback(GLFWwindow *window, int width, int height)
{
    auto app = static_cast<AppDrummer *>(glfwGetWindowUserPointer(window));

    if (app != nullptr) app->onResize(width, height);
}

void AppDrummer::onResize(int width, int height)
{
    this->_display_w = width;
    this->_display_h = height;

    glViewport(0, 0, width, height);
}

bool AppDrummer::Setup()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    io.ConfigDockingWithShift = false;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsDark();
    ImGui::GetStyle().TabRounding = 2.0f;
    ImGui::GetStyle().FrameRounding = 2.0f;

    io.Fonts->Clear();
    ImFont *font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    if (font != nullptr)
    {
        io.FontDefault = font;
    }
    else
    {
        io.Fonts->AddFontDefault();
    }
    io.Fonts->Build();

    _state._banks->RescanForBanks();
    _state._banks->LoadBank(_state._currentBank);

    _stepper.Setup();
    _state._activeTrack = 0;

    _effectUi.Setup();
    _libraryUi.Setup();
    _mixerUi.Setup();
    _sequencerUi.Setup();

    return true;
}

void AppDrummer::Tick()
{
    _stepper.Tick();
}

void PianoRollEditor(AppState &_state)
{
    if (ImGui::Begin("Pianoroll editor"))
    {
        if (_state._activeTrack < 0 || _state._activeTrack >= NUM_MIXER_TRACKS)
        {
            ImGui::End();
            return;
        }

        if (_state._activePattern < 0 || !_state._sequencer.DoesPatternExistAtIndex(_state._activeTrack, _state._activePattern))
        {
            ImGui::End();
            return;
        }

        auto &region = _state._sequencer.GetPattern(_state._activeTrack, _state._activePattern);
        unsigned int maxvalue = 50;
        for (int c = 0; c < 88; c++)
        {
            for (size_t i = 0; i < region.valuesOfValues[c].size(); i++)
            {
                if (region.valuesOfValues[c][i].values[1] + 10 > maxvalue)
                    maxvalue = static_cast<unsigned int>(std::ceil(region.valuesOfValues[c][i].values[1] / 10) * 10 + 10);
            }
        }
        const float elapsedTime = static_cast<float>((static_cast<unsigned>(_state._stepper->_totalTimeInMs)) % (maxvalue * 1000)) / 1000.f;

        static struct timelineEvent *selectedEvent = nullptr;
        if (ImGui::BeginTimelines("MyTimeline", maxvalue, 0, 88))
        {
            for (int c = 0; c < 88; c++)
            {
                char id[32];
                sprintf(id, "%4s%d", NoteNames[(107 - c) % NoteNameCount], (107 - c) / NoteNameCount - 1);
                ImGui::TimelineStart(id, false);
                for (size_t i = 0; i < region.valuesOfValues[c].size(); i++)
                {
                    bool selected = (&(region.valuesOfValues[c][i]) == selectedEvent);
                    if (ImGui::TimelineEvent(region.valuesOfValues[c][i].values, &selected))
                    {
                        selectedEvent = &(region.valuesOfValues[c][i]);
                    }
                }
                float new_values[2];
                if (ImGui::TimelineEnd(new_values))
                {
                    timelineEvent e{
                        std::fmin(new_values[0], new_values[1]),
                        std::fmax(new_values[0], new_values[1])};

                    region.valuesOfValues[c].push_back(e);
                    selectedEvent = &(region.valuesOfValues[c].back());
                }
            }
        }
        ImGui::EndTimelines(maxvalue / 10, elapsedTime);
    }
    ImGui::End();
}

void AppDrummer::Render()
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

    ImGuiPlayback();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
    _effectUi.Render();
    _libraryUi.Render();
    _mixerUi.Render();
    _sequencerUi.Render();
    ImGui::PopStyleVar();

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

void AppDrummer::onKeyAction(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{
}

void AppDrummer::ImGuiPlayback()
{
    ImGui::Begin("Playback");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 5));

        ImGui::SameLine();

        ImGui::Checkbox("Library", &_state._showLibrary);

        ImGui::SameLine();

        ImGui::Checkbox("Mixer", &_state._showMixer);

        ImGui::SameLine();

        ImGui::Checkbox("Step Editor", &_state._showEditor);

        ImGui::SameLine();

        if (ImGui::Button("Stop", ImVec2(32, 32)))
        {
            _stepper.Stop();
        }

        ImGui::SameLine();

        bool isPlaying = _stepper.IsPlaying();
        if (ImGui::Button(isPlaying ? "Pause" : "Play", ImVec2(32, 32)))
        {
            _stepper.PlayPause();
        }

        ImGui::SameLine();

        ImGui::PopStyleVar();

        ImGui::Spacing();

        ImGui::SameLine();

        auto bpm = _stepper.Bpm();
        ImGui::PushItemWidth(100);
        if (ImGui::InputInt("##bpm", &bpm))
        {
            _stepper.Bpm(bpm);
        }

        ImGui::SameLine();

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0 / static_cast<double>(ImGui::GetIO().Framerate), static_cast<double>(ImGui::GetIO().Framerate));

        ImGui::End();
    }
}

void AppDrummer::Cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
