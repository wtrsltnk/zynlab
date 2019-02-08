#include "app.threedee.h"

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

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer, IBankManager *banks)
    : _state(mixer, &_stepper, banks), _adNoteUI(&_state), _effectUi(&_state), _libraryUi(&_state),
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;   // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; // Enable Multi-Viewport / Platform Windows
    io.ConfigDockingWithShift = false;

    ImGui_ImplGlfw_InitForOpenGL(_window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    ImGui::StyleColorsDark();
    ImGui::GetStyle().TabRounding = 2.0f;
    ImGui::GetStyle().FrameRounding = 2.0f;

    ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(0.71f, 0.7f, 0.7f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] = ImVec4(0.51f, 0.5f, 0.5f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_Tab] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TabActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TabHovered] = ImVec4(0.45f, 0.49f, 0.48f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.14f, 0.14f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.15f, 0.14f, 0.14f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.15f, 0.14f, 0.14f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_TabUnfocusedActive] = ImGui::GetStyle().Colors[ImGuiCol_TabActive];
    ImGui::GetStyle().Colors[ImGuiCol_TabUnfocused] = ImGui::GetStyle().Colors[ImGuiCol_Tab];
    ImGui::GetStyle().Colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.15f, 0.14f, 0.14f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.15f, 0.14f, 0.14f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_Separator] = ImVec4(0.24f, 0.24f, 0.27f, 1.0f);
    ImGui::GetStyle().Colors[ImGuiCol_ResizeGrip] = ImVec4(0.15f, 0.14f, 0.14f, 1.0f);

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

    _mixerUi.Setup();
    _libraryUi.Setup();
    _adNoteUI.Setup();
    _subNoteUi.Setup();
    _padNoteUi.Setup();
    _effectUi.Setup();
    _sequencerUi.Setup();

    LoadToolbarIcons();

    _state._playTime = 0.0f;

    return true;
}

void AppThreeDee::Tick()
{
    _stepper.Tick();

    std::chrono::milliseconds::rep currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    auto deltaTime = currentTime - _lastSequencerTimeInMs;
    _lastSequencerTimeInMs = currentTime;

    if (_state._stepper->IsPlaying())
    {
        auto prevPlayTime = (_state._playTime / 1000.0f);
        _state._playTime += deltaTime;
        auto currentPlayTime = (_state._playTime / 1000.0f);

        for (unsigned char trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; trackIndex++)
        {
            auto *track = _state._mixer->GetTrack(trackIndex);
            if (!track->Penabled)
            {
                continue;
            }
            for (auto &region : _state.regionsByTrack[trackIndex])
            {
                if (region.startAndEnd[0] < prevPlayTime && region.startAndEnd[1] < prevPlayTime)
                {
                    continue;
                }
                if (region.startAndEnd[0] > currentPlayTime && region.startAndEnd[1] > currentPlayTime)
                {
                    continue;
                }

                for (unsigned char noteIndex = 0; noteIndex < NUM_MIDI_NOTES; noteIndex++)
                {
                    for (auto &event : region.eventsByNote[noteIndex])
                    {
                        auto start = (region.startAndEnd[0] + event.values[0]);
                        auto end = (region.startAndEnd[0] + event.values[1]);
                        if (start >= prevPlayTime && start < currentPlayTime)
                        {
                            _state._mixer->NoteOn(trackIndex, noteIndex, 100);
                        }
                        if (end >= prevPlayTime && end < currentPlayTime)
                        {
                            _state._mixer->NoteOff(trackIndex, noteIndex);
                        }
                    }
                }
            }
        }
    }
}

static char const *snapping_modes[] = {
    "Bar",
    "Beat",
    "Division",
};
static int snapping_mode_count = 3;

static float snapping_mode_values[] = {
    4.0f,
    1.0f,
    1.0f / 4.0f,
};

void PianoRollEditor(AppState &_state)
{
    if (ImGui::Begin("Pianoroll editor"))
    {
        if (_state._activeTrack < 0 || _state._activeTrack >= NUM_MIXER_TRACKS)
        {
            ImGui::End();
            return;
        }

        auto &trackRegions = _state.regionsByTrack[_state._activeTrack];

        if (_state._activePattern < 0 || size_t(_state._activePattern) >= trackRegions.size() || trackRegions.empty())
        {
            ImGui::End();
            return;
        }

        auto &region = trackRegions[size_t(_state._activePattern)];
        unsigned int maxvalue = static_cast<unsigned int>(region.startAndEnd[1] - region.startAndEnd[0]);

        static int horizontalZoom = 150;

        ImGui::Text("Zoom");
        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::SliderInt("##horizontalZoom", &horizontalZoom, 100, 300, "horizontal %d");

        ImGui::SameLine();

        static int current_snapping_mode = 2;

        ImGui::PushItemWidth(100);
        if (ImGui::BeginCombo("Snapping mode", snapping_modes[current_snapping_mode]))
        {
            for (int n = 0; n < snapping_mode_count; n++)
            {
                bool is_selected = (current_snapping_mode == n);
                if (ImGui::Selectable(snapping_modes[n], is_selected))
                {
                    current_snapping_mode = n;
                }
            }

            ImGui::EndCombo();
        }
        ImGui::ShowTooltipOnHover("Set the Snap value for the Piano Roll Editor");

        float elapsedTime = static_cast<float>((static_cast<unsigned>(_state._playTime))) / 1000.f - region.startAndEnd[0];

        if (ImGui::BeginChild("##timelinechild", ImVec2(0, -30)))
        {
            bool regionIsModified = false;
            static struct timelineEvent *selectedEvent = nullptr;
            if (ImGui::BeginTimelines("MyTimeline", maxvalue, 20, horizontalZoom, 88, snapping_mode_values[current_snapping_mode]))
            {
                for (int c = NUM_MIDI_NOTES - 1; c > 0; c--)
                {
                    char id[32];
                    sprintf(id, "%4s%d", NoteNames[(107 - c) % NoteNameCount], (107 - c) / NoteNameCount - 1);
                    ImGui::TimelineStart(id);
                    if (ImGui::IsItemClicked())
                    {
                        _state._stepper->HitNote(_state._activeTrack, c, 100, 200);
                    }
                    for (size_t i = 0; i < region.eventsByNote[c].size(); i++)
                    {
                        bool selected = (&(region.eventsByNote[c][i]) == selectedEvent);
                        if (ImGui::TimelineEvent(region.eventsByNote[c][i].values, 0, ImGui::ColorConvertFloat4ToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) & selected))
                        {
                            if (region.eventsByNote[c][i].values[0] + 0.2f > region.eventsByNote[c][i].values[1])
                            {
                                region.eventsByNote[c][i].values[1] = region.eventsByNote[c][i].values[0] + 0.2f;
                            }
                            regionIsModified = true;
                            selectedEvent = &(region.eventsByNote[c][i]);
                        }
                    }
                    float new_values[2];
                    if (ImGui::TimelineEnd(new_values))
                    {
                        regionIsModified = true;
                        timelineEvent e{
                            {std::fmin(new_values[0], new_values[1]),
                             std::fmax(new_values[0], new_values[1])},
                            static_cast<unsigned char>(c),
                            100};

                        if (e.values[0] + 0.2f >= e.values[1])
                        {
                            e.values[1] = e.values[0] + 0.2f;
                        }

                        region.eventsByNote[c].push_back(e);
                        selectedEvent = &(region.eventsByNote[c].back());
                    }
                }
            }
            ImGui::EndTimelines(&elapsedTime);

            if (regionIsModified)
            {
                region.UpdatePreviewImage();
            }

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Delete)) && selectedEvent != nullptr)
            {
                if (_state._activeTrack >= 0 && _state._activeTrack < NUM_MIXER_TRACKS)
                {
                    auto &trackRegions = _state.regionsByTrack[_state._activeTrack];
                    if (_state._activePattern >= 0 && size_t(_state._activePattern) < trackRegions.size())
                    {
                        auto &events = trackRegions[size_t(_state._activePattern)].eventsByNote[selectedEvent->note];
                        auto itr = events.begin();
                        while (&(*itr) != selectedEvent && itr != events.end())
                        {
                            itr++;
                        }
                        if (itr != events.end())
                        {
                            events.erase(itr);
                        }
                    }
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void RegionEditor(AppState &_state)
{
    if (ImGui::Begin("Region editor"))
    {
        static int horizontalZoom = 50;
        static int verticalZoom = 50;

        ImGui::Text("Zoom");
        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::SliderInt("##horizontalZoom", &horizontalZoom, 10, 100, "horizontal %d");
        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::SliderInt("##verticalZoom", &verticalZoom, 30, 100, "vertical %d");

        int maxvalueSequencer = 50;
        float elapsedTimeSequencer = static_cast<float>((static_cast<int>(_state._playTime)) % (maxvalueSequencer * 1000)) / 1000.f;

        if (ImGui::BeginChild("##timeline2child", ImVec2(0, -30)))
        {
            if (ImGui::BeginTimelines("MyTimeline2", maxvalueSequencer, verticalZoom, horizontalZoom, NUM_MIXER_TRACKS, 1.0f))
            {
                for (int trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; trackIndex++)
                {
                    auto hue = trackIndex * 0.05f;
                    auto tintColor = ImColor::HSV(hue, 0.6f, 0.6f);

                    auto &regions = _state.regionsByTrack[trackIndex];
                    char id[32];
                    sprintf(id, "Track %d", trackIndex);
                    ImGui::TimelineStart(id);
                    if (ImGui::IsItemClicked())
                    {
                        _state._activeTrack = trackIndex;
                    }

                    for (size_t i = 0; i < regions.size(); i++)
                    {
                        bool selected = (trackIndex == _state._activeTrack && int(i) == _state._activePattern);
                        if (ImGui::TimelineEvent(regions[i].startAndEnd, regions[i].previewImage, tintColor, &selected))
                        {
                            _state._activeTrack = trackIndex;
                            _state._activePattern = int(i);
                            if (std::fabs(regions[i].startAndEnd[0] - regions[i].startAndEnd[1]) < 1.0f)
                            {
                                regions[i].startAndEnd[1] = regions[i].startAndEnd[0] + 1.0f;
                            }
                            regions[i].UpdatePreviewImage();
                        }
                    }

                    TrackRegion newRegion;
                    if (ImGui::TimelineEnd(newRegion.startAndEnd))
                    {
                        _state._activeTrack = trackIndex;
                        _state._activePattern = int(_state.regionsByTrack[trackIndex].size());

                        if (newRegion.startAndEnd[1] - newRegion.startAndEnd[0] < 1.0f)
                        {
                            newRegion.startAndEnd[1] = newRegion.startAndEnd[0] + 1.0f;
                        }
                        newRegion.UpdatePreviewImage();
                        _state.regionsByTrack[trackIndex].push_back(newRegion);
                    }
                }
            }
            ImGui::EndTimelines(&elapsedTimeSequencer);
            _state._playTime = elapsedTimeSequencer * 1000;

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Delete)))
            {
                if (_state._activeTrack >= 0 && _state._activeTrack < NUM_MIXER_TRACKS)
                {
                    _state.regionsByTrack[_state._activeTrack].erase(_state.regionsByTrack[_state._activeTrack].begin() + _state._activePattern);
                }
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
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
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
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

    if (_state._showSmartControls)
    {
        _adNoteUI.Render();
        _padNoteUi.Render();
        _subNoteUi.Render();
    }
    ImGui::PopStyleVar();

    PianoRollEditor(_state);
    RegionEditor(_state);

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
    QuickHelp,
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
    "quick-help.png",
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

        ImGui::ImageToggleButton("toolbar_quick_help", &_state._showQuickHelp, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::QuickHelp)]), ImVec2(32, 32));

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

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Rewind)]), ImVec2(32, 32)))
        {
            _state._playTime = 0;
            _state._mixer->ShutUp();
        }

        ImGui::SameLine();

        ImGui::ImageButton(reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::FastForward)]), ImVec2(32, 32));

        ImGui::SameLine();

        if (ImGui::ImageButton(reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Stop)]), ImVec2(32, 32)))
        {
            _state._playTime = 0;
            _state._mixer->ShutUp();
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
