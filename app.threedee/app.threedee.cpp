#include "app.threedee.h"

#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnoteParams.h>

#include "examples/imgui_impl_glfw.h"
#include "examples/imgui_impl_opengl3.h"
#include "imgui_addons/imgui_checkbutton.h"
#include "imgui_addons/imgui_knob.h"
#include "stb_image.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <map>

const char *NoteNames[] = {
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

int NoteNameCount = 12;

static ImVec4 clear_color = ImColor(90, 90, 100);

static const float noteLabelWidth = 50.0f;
static const float rowHeight = 20.0f;
static const float stepWidth = 20.0f;

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer)
    : _state(mixer), _adNoteUI(&_state), _effectUi(&_state), _mixerUi(&_state), _padNoteUi(&_state), _subNoteUi(&_state),
      _window(window), _stepper(&_sequencer, mixer),
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

    ImGui::StyleColorsClassic();

    _state._mixer->GetBankManager()->RescanForBanks();
    _state._mixer->GetBankManager()->LoadBank(_state._currentBank);

    _stepper.Setup();
    _state._activeChannel = 0;

    _mixerUi.Setup();

    LoadToolbarIcons();

    return true;
}

enum class ToolbarTools
{
    Library,
    Inspector,
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

void AppThreeDee::HitNote(int trackIndex, int note, int velocity, int durationInMs)
{
    _stepper.HitNote(static_cast<unsigned char>(trackIndex),
                     static_cast<unsigned char>(note),
                     static_cast<unsigned char>(velocity),
                     durationInMs);
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
            auto trackEnabled = _state._mixer->GetChannel(trackIndex)->Penabled == 1;
            if (ImGui::Checkbox("##trackEnabled", &trackEnabled))
            {
                _state._mixer->GetChannel(trackIndex)->Penabled = trackEnabled ? 1 : 0;
            }
            ImGui::SameLine();

            float hue = trackIndex * 0.05f;
            char trackLabel[32] = {'\0'};
            sprintf(trackLabel, "%02d", trackIndex + 1);
            bool highLight = trackIndex == _state._activeChannel;
            if (highLight)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            }
            if (ImGui::Button(trackLabel, ImVec2(trackHeight, trackHeight)))
            {
                _state._activeChannel = trackIndex;
            }
            if (highLight)
            {
                ImGui::PopStyleColor();
            }
            ImGui::PopID();

            ImGui::PushStyleColor(ImGuiCol_Button, static_cast<ImVec4>(ImColor::HSV(hue, 0.6f, 0.6f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, static_cast<ImVec4>(ImColor::HSV(hue, 0.7f, 0.7f)));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, static_cast<ImVec4>(ImColor::HSV(hue, 0.8f, 0.8f)));

            //if (_state._mixer->GetChannel(trackIndex)->Pkitmode != 0)
            {
                ImGuiStepSequencer(trackIndex, trackHeight);
            }
            //else
            {
                //ImGuiPianoRollSequencer(trackIndex, trackHeight);
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
            //if (_state._activeInstrument() >= 0 && _state._mixer->GetChannel(_state._activeInstrument())->Pkitmode != 0)
            {
                ImGuiStepSequencerEventHandling();
            }
            //else
            {
                //ImGuiPianoRollSequencerEventHandling();
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
        bool isActive = (trackIndex == _state._activeChannel && patternIndex == _state._activePattern);
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
                _state._activeChannel = trackIndex;
                _state._activePattern = patternIndex;
            }
            if (ImGui::IsMouseDoubleClicked(0))
            {
                _state._showEditor = true;
            }
        }
        else if (_state._mixer->GetChannel(trackIndex)->Penabled)
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

    if (_state._mixer->GetChannel(trackIndex)->Penabled)
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
        _sequencer.RemoveActivePattern(_state._activeChannel, _state._activePattern);
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
    {
        if (io.KeyShift && !io.KeyCtrl)
        {
            _sequencer.MovePatternLeftForced(_state._activeChannel, _state._activePattern);
        }
        else if (!io.KeyShift && io.KeyCtrl)
        {
            _sequencer.SwitchPatternLeft(_state._activeChannel, _state._activePattern);
        }
        else
        {
            _sequencer.MovePatternLeftIfPossible(_state._activeChannel, _state._activePattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
    {
        if (io.KeyShift && !io.KeyCtrl)
        {
            _sequencer.MovePatternRightForced(_state._activeChannel, _state._activePattern);
        }
        else if (!io.KeyShift && io.KeyCtrl)
        {
            _sequencer.SwitchPatternRight(_state._activeChannel, _state._activePattern);
        }
        else
        {
            _sequencer.MovePatternRightIfPossible(_state._activeChannel, _state._activePattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Home)))
    {
        _sequencer.SelectFirstPatternInTrack(_state._activeChannel, _state._activePattern);
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_End)))
    {
        _sequencer.SelectLastPatternInTrack(_state._activeChannel, _state._activePattern);
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
    {
        _state._showEditor = true;
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
    {
        if (io.KeyShift)
        {
            _sequencer.SelectPreviousPattern(_state._activeChannel, _state._activePattern);
        }
        else
        {
            _sequencer.SelectNextPattern(_state._activeChannel, _state._activePattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_C)) && io.KeyCtrl)
    {
        if (_sequencer.DoesPatternExistAtIndex(_state._activeChannel, _state._activePattern))
        {
            auto pattern = _sequencer.GetPattern(_state._activeChannel, _state._activePattern);
            _clipboardPatterns.push_back(pattern);
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_V)) && io.KeyCtrl)
    {
        if (!_clipboardPatterns.empty() && _sequencer.DoesPatternExistAtIndex(_state._activeChannel, _state._activePattern))
        {
            _sequencer.SetPattern(_state._activeChannel, _state._activePattern, _clipboardPatterns.back());
        }
    }
}

void AppThreeDee::ImGuiStepPatternEditorWindow()
{
    if (!_state._showEditor)
    {
        return;
    }

    ImGui::Begin("Pattern editor", &_state._showEditor);

    if (!_sequencer.DoesPatternExistAtIndex(_state._activeChannel, _state._activePattern))
    {
        _state._activePattern = -1;
        ImGui::End();
        return;
    }

    auto &style = ImGui::GetStyle();
    auto &selectedPattern = _sequencer.GetPattern(_state._activeChannel, _state._activePattern);

    char tmp[256];
    strcpy(tmp, selectedPattern._name.c_str());
    if (ImGui::InputText("pattern name", tmp, 256))
    {
        selectedPattern._name = tmp;
    }

    ImGui::BeginChild("Notes");
    auto width = ImGui::GetWindowWidth() - noteLabelWidth - (style.ItemSpacing.x * 2) - style.ScrollbarSize;
    auto itemWidth = (width / 16) - (style.ItemSpacing.x);
    for (int i = 0; i < 88; i++)
    {
        if (i % NoteNameCount == 0)
        {
            ImGui::Separator();
        }
        ImGui::PushID(i);
        if (ImGui::Button(NoteNames[i % NoteNameCount], ImVec2(noteLabelWidth, rowHeight)))
        {
            HitNote(_state._activeChannel, i, 200, 200);
        }
        for (int j = 0; j < 16; j++)
        {
            ImGui::SameLine();
            ImGui::PushID(j);
            auto found = selectedPattern._notes.find(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j), 0.2f));
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
                    selectedPattern._notes.erase(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j), 0.2f));
                }
                else
                {
                    selectedPattern._notes.insert(TrackPatternNote(static_cast<unsigned char>(i), static_cast<unsigned char>(j), 0.2f));
                }
                HitNote(_state._activeChannel, i, 200, 200);
            }
            ImGui::PopStyleColor();
            ImGui::PopID();
        }
        ImGui::PopID();
    }
    ImGui::EndChild();
    ImGui::End();
}

void AppThreeDee::ImGuiPianoRollSequencer(int trackIndex, float trackHeight)
{
    auto lastIndex = _sequencer.LastPatternIndex(trackIndex);
    for (int patternIndex = 0; patternIndex <= lastIndex; patternIndex++)
    {
        auto count = _sequencer.PatternStepCount(trackIndex, patternIndex);

        ImGui::SameLine();
        ImGui::PushID(patternIndex + trackIndex * 1000);
        bool isActive = trackIndex == _state._activeChannel && patternIndex == _state._activePattern;
        if (isActive)
        {
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 2.0f);
        }

        if (_sequencer.DoesPatternExistAtIndex(trackIndex, patternIndex))
        {
            auto &pattern = _sequencer.GetPattern(trackIndex, patternIndex);
            if (ImGui::Button(pattern._name.c_str(), ImVec2(((count - 1) / 16 + 1) * 120.0f, trackHeight)))
            {
                _state._activeChannel = trackIndex;
                _state._activePattern = patternIndex;
            }
            if (ImGui::IsMouseDoubleClicked(0))
            {
                _state._showEditor = true;
            }
        }
        else if (_state._mixer->GetChannel(trackIndex)->Penabled)
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
    if (_state._mixer->GetChannel(trackIndex)->Penabled)
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

void AppThreeDee::ImGuiPianoRollSequencerEventHandling()
{
}

void AppThreeDee::ImGuiPianoRollPatternEditorWindow()
{
    if (!_state._showEditor)
    {
        return;
    }

    if (!_sequencer.DoesPatternExistAtIndex(_state._activeChannel, _state._activePattern))
    {
        _state._activePattern = -1;
        return;
    }

    auto &style = ImGui::GetStyle();
    auto &selectedPattern = _sequencer.GetPattern(_state._activeChannel, _state._activePattern);

    ImGui::Begin("Piano roll editor", &_state._showEditor);
    char tmp[256];
    strcpy(tmp, selectedPattern._name.c_str());
    if (ImGui::InputText("pattern name", tmp, 256))
    {
        selectedPattern._name = tmp;
    }

    ImGui::BeginChild("Notes");
    for (int i = 0; i < 88; i++)
    {
        auto mousey = ImGui::GetMousePos().y - ImGui::GetWindowPos().y + ImGui::GetScrollY();
        auto mousex = ImGui::GetMousePos().x - ImGui::GetWindowPos().x + ImGui::GetScrollX();

        if (i % NoteNameCount == 0)
        {
            ImGui::Separator();
        }
        ImGui::PushID(i);

        auto min = ImGui::GetCursorPos();
        if (ImGui::Button(NoteNames[i % NoteNameCount], ImVec2(noteLabelWidth, rowHeight)))
        {
            HitNote(_state._activeChannel, i, 200, 200);
        }

        for (auto &note : selectedPattern._notes)
        {
            if (note._note != i) continue;

            ImGui::SetCursorPos(ImVec2(noteLabelWidth + style.ItemSpacing.x + note._step * stepWidth, min.y));
            ImGui::Button("", ImVec2(note._length * stepWidth, rowHeight));
        }

        auto step = mousex - (noteLabelWidth + style.ItemSpacing.x);
        if (ImGui::IsWindowFocused() && mousey >= min.y &&
            mousey <= (min.y + ImGui::GetTextLineHeight()) && step > 0 &&
            !selectedPattern.IsStepCovered(static_cast<unsigned char>(i), static_cast<int>(step)))
        {
            min.x = std::floor((noteLabelWidth + style.ItemSpacing.x + step) / stepWidth) * stepWidth;
            ImGui::SetCursorPos(min);
            if (ImGui::Button("+", ImVec2(stepWidth, rowHeight)))
            {
                selectedPattern._notes.insert(TrackPatternNote(static_cast<unsigned char>(i), static_cast<int>((step / stepWidth)), 0.4f));
            }
        }
        ImGui::PopID();
    }
    ImGui::EndChild();

    ImGui::End();
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

    ImGuiPlayback();

    _mixerUi.Render();
    _effectUi.Render();

    ImGuiSequencer();
    ImGuiStepPatternEditorWindow();
    ImGuiPianoRollPatternEditorWindow();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
    _adNoteUI.Render();
    _subNoteUi.Render();
    _padNoteUi.Render();
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

void AppThreeDee::onKeyAction(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
{
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
