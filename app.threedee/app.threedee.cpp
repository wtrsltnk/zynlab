#include "app.threedee.h"

#include <zyn.mixer/Channel.h>
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

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _state(mixer, &_stepper), _adNoteUI(&_state), _effectUi(&_state), _libraryUi(&_state),
      _mixerUi(&_state), _padNoteUi(&_state), _sequencerUi(&_state), _subNoteUi(&_state),
      _window(window), _stepper(&_state._sequencer, mixer),
      _toolbarIconsAreLoaded(false),
      _display_w(800), _display_h(600)
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

    ImGui::StyleColorsDark();

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

    _state._mixer->GetBankManager()->RescanForBanks();
    _state._mixer->GetBankManager()->LoadBank(_state._currentBank);

    _stepper.Setup();
    _state._activeChannel = 0;

    _mixerUi.Setup();
    _libraryUi.Setup();
    _adNoteUI.Setup();
    _subNoteUi.Setup();
    _padNoteUi.Setup();
    _effectUi.Setup();
    _sequencerUi.Setup();

    LoadToolbarIcons();

    return true;
}

void AppThreeDee::Tick()
{
    _stepper.Tick();
}

struct timelineEvent
{
    float values[2];
};

static std::vector<struct timelineEvent> valuesOfValues[88];

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

    ImGuiPlayback();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
    _effectUi.Render();
    _libraryUi.Render();
    _mixerUi.Render();
    _sequencerUi.Render();
    if (_state._showSmartControls)
    {
        _adNoteUI.Render();
        _padNoteUi.Render();
        _subNoteUi.Render();
    }
    ImGui::PopStyleVar();

    unsigned int maxvalue = 50;
    for (int c = 0; c < 88; c++)
    {
        for (size_t i = 0; i < valuesOfValues[c].size(); i++)
        {
            if (valuesOfValues[c][i].values[1] + 10 > maxvalue)
                maxvalue = static_cast<unsigned int>(std::ceil(valuesOfValues[c][i].values[1] / 10) * 10 + 10);
        }
    }
    const float elapsedTime = static_cast<float>((static_cast<unsigned>(ImGui::GetTime() * 1000)) % (maxvalue * 1000)) / 1000.f;

    static struct timelineEvent *selectedEvent = nullptr;
    ImGui::Begin("timeline window");
    if (ImGui::BeginTimelines("MyTimeline", maxvalue, 0, 88))
    {
        for (int c = 0; c < 88; c++)
        {
            char id[32];
            sprintf(id, "%d", 107 - c);
            ImGui::TimelineStart(id, false);
            for (size_t i = 0; i < valuesOfValues[c].size(); i++)
            {
                bool selected = (&(valuesOfValues[c][i]) == selectedEvent);
                if (ImGui::TimelineEvent(valuesOfValues[c][i].values, &selected))
                {
                    selectedEvent = &(valuesOfValues[c][i]);
                }
            }
            float new_values[2];
            if (ImGui::TimelineEnd(new_values))
            {
                timelineEvent e{
                    std::fmin(new_values[0], new_values[1]),
                    std::fmax(new_values[0], new_values[1])};

                valuesOfValues[c].push_back(e);
                selectedEvent = &(valuesOfValues[c].back());
            }
        }
    }
    ImGui::EndTimelines(maxvalue / 10, elapsedTime);

    ImGui::End();

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

enum class ToolbarTools
{
    Library,
    Inspector,
    SmartControls,
    Mixer,
    Editor,
    Rewind,
    FastForward,
    Stop,
    Play,
    Record,
    COUNT,
};

static char const *const toolbarIconFileNames[] = {
    "library.png",
    "inspector.png",
    "smart-controls.png",
    "mixer.png",
    "editor.png",
    "rewind.png",
    "fast-forward.png",
    "stop.png",
    "play.png",
    "record.png",
};

void AppThreeDee::LoadToolbarIcons()
{
    std::string rootDir = "./icons/";

    _toolbarIconsAreLoaded = false;

    _toolbarIcons.reserve(static_cast<size_t>(ToolbarTools::COUNT));

    for (size_t i = 0; i < static_cast<size_t>(ToolbarTools::COUNT); i++)
    {
        GLuint my_opengl_texture;
        glGenTextures(1, &my_opengl_texture);

        auto filename = rootDir + toolbarIconFileNames[i];
        int x, y, n;
        unsigned char *data = stbi_load(filename.c_str(), &x, &y, &n, 0);
        if (data == nullptr)
        {
            std::cout << "Failed to load instrument category " << i << " from file " << filename << std::endl;
            _toolbarIcons[i] = 0;
            continue;
        }
        _toolbarIcons[i] = my_opengl_texture;
        _toolbarIconsAreLoaded = true;

        glBindTexture(GL_TEXTURE_2D, my_opengl_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, n, x, y, 0, n == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
}

void AppThreeDee::ImGuiPlayback()
{
    ImGui::Begin("Playback");
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 5));

        ImGui::ImageToggleButton("toolbar_library", &_state._showLibrary, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Library)]), ImVec2(32, 32));

        ImGui::SameLine();

        ImGui::ImageToggleButton("toolbar_inspector", &_state._showInspector, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Inspector)]), ImVec2(32, 32));

        ImGui::SameLine();

        ImGui::ImageToggleButton("toolbar_smart_controls", &_state._showSmartControls, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::SmartControls)]), ImVec2(32, 32));

        ImGui::SameLine();

        ImGui::ImageToggleButton("toolbar_mixer", &_state._showMixer, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Mixer)]), ImVec2(32, 32));

        ImGui::SameLine();

        ImGui::ImageToggleButton("toolbar_editor", &_state._showEditor, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Editor)]), ImVec2(32, 32));

        ImGui::PopStyleVar();

        ImGui::SameLine();

        ImGui::Spacing();

        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 5));

        ImGui::SameLine();

        ImGui::ImageButton(reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Rewind)]), ImVec2(32, 32));

        ImGui::SameLine();

        ImGui::ImageButton(reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::FastForward)]), ImVec2(32, 32));

        ImGui::SameLine();

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Stop)]), ImVec2(32, 32)))
        {
            _stepper.Stop();
        }

        ImGui::SameLine();

        ImGui::SameLine();

        bool isPlaying = _stepper.IsPlaying();
        if (ImGui::ImageToggleButton("toolbar_play", &isPlaying, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Play)]), ImVec2(32, 32)))
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

void AppThreeDee::Cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
