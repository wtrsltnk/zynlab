#include "app.threedee.h"

#include "stb_image.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <examples/imgui_impl_glfw.h>
#include <examples/imgui_impl_opengl3.h>
#include <imgui_Timeline.h>
#include <imgui_checkbutton.h>
#include <imgui_knob.h>
#include <iterator>
#include <map>
#include <zyn.common/WavFileWriter.h>
#include <zyn.mixer/Track.h>
#include <zyn.nio/AudioOutputManager.h>
#include <zyn.nio/MidiInputManager.h>
#include <zyn.nio/WavEngine.h>
#include <zyn.seq/ArpModes.h>
#include <zyn.seq/Chords.h>
#include <zyn.seq/NotesGenerator.h>
#include <zyn.serialization/LibraryManager.h>
#include <zyn.serialization/SaveToFileSerializer.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/SampleNoteParams.h>

char const *const NoteNames[] = {
    "B",
    "A#",
    "A",
    "G#",
    "G",
    "F#",
    "F",
    "E",
    "D#",
    "D",
    "C#",
    "C",
};

unsigned int NoteNameCount = 12;

char const *const SnappingModes[] = {
    "Bar",
    "Beat",
    "Division",
};

unsigned int SnappingModeCount = 3;

timestep SnappingModeValues[] = {
    1024,
    1024 / 4,
    (1024 / 4) / 4,
};

static ImVec4 clear_color = ImColor(90, 90, 100);

enum class ToolbarTools
{
    Workspace,
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
    Regions,
    Piano,
    Effect,
    COUNT,
};

static char const *const toolbarIconFileNames[] = {
    "workspace.png",
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
    "notation.png",
    "piano.png",
    "effect.png"};

AppThreeDee::AppThreeDee(GLFWwindow *window, Mixer *mixer, ILibraryManager *library)
    : _state(mixer, library), _adNoteUI(&_state), _effectUi(&_state), _libraryUi(&_state),
      _mixerUi(&_state), _padNoteUi(&_state), _subNoteUi(&_state), _smplNoteUi(&_state), _oscilGenUi(&_state), _dialogs(&_state),
      _window(window),
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

    io.IniFilename = nullptr;

    _state._library->RefreshLibraries();

    _state._currentTrack = 0;

    _mixerUi.Setup();
    _libraryUi.Setup();
    _adNoteUI.Setup();
    _subNoteUi.Setup();
    _smplNoteUi.Setup();
    _padNoteUi.Setup();
    _effectUi.Setup();
    _oscilGenUi.Setup();
    _dialogs.Setup();

    LoadToolbarIcons();

    _state._playTime = 0.0f;

    TrackRegion region;
    region.startAndEnd[0] = std::chrono::milliseconds(0).count();
    region.startAndEnd[1] = std::chrono::milliseconds(1200).count();

    TrackRegionEvent a, b;

    a.note = 65;
    a.values[0] = std::chrono::milliseconds(200).count();
    a.values[1] = std::chrono::milliseconds(600).count();
    a.velocity = 100;
    region.eventsByNote->push_back(a);

    b.note = 75;
    b.values[0] = std::chrono::milliseconds(400).count();
    b.values[1] = std::chrono::milliseconds(800).count();
    b.velocity = 100;
    region.eventsByNote->push_back(b);

    _state._regions.AddRegion(0, region);

    return true;
}

void AppThreeDee::TickRegion(TrackRegion &region, unsigned char trackIndex, float prevPlayTime, float currentPlayTime, int repeat)
{
    auto track = _state._mixer->GetTrack(trackIndex);
    auto regionSize = region.startAndEnd[1] - region.startAndEnd[0];
    auto regionStart = region.startAndEnd[0] + repeat * regionSize;
    auto regionEnd = region.startAndEnd[1] + repeat * regionSize;

    if (regionStart < prevPlayTime && regionEnd < prevPlayTime)
    {
        return;
    }
    if (regionStart > currentPlayTime && regionEnd > currentPlayTime)
    {
        return;
    }

    MidiEvent ev;
    ev.type = MidiEventTypes::M_NOTE;
    ev.channel = track->Prcvchn;

    for (unsigned char noteIndex = 0; noteIndex < NUM_MIDI_NOTES; noteIndex++)
    {
        ev.num = noteIndex;
        for (auto &event : region.eventsByNote[noteIndex])
        {
            auto start = (regionStart + event.values[0]);
            auto end = (regionStart + event.values[1]);
            if (start >= prevPlayTime && start < currentPlayTime)
            {
                ev.value = 100;
                MidiInputManager::Instance().PutEvent(ev);
            }
            if (end >= prevPlayTime && end < currentPlayTime)
            {
                ev.value = 0;
                MidiInputManager::Instance().PutEvent(ev);
            }
        }
    }
}

void AppThreeDee::TempNoteOn(unsigned int channel, unsigned int note, unsigned int length)
{
    MidiEvent ev;
    ev.type = MidiEventTypes::M_NOTE;
    ev.channel = channel;
    ev.value = 100;
    ev.num = note;

    MidiInputManager::Instance().PutEvent(ev);

    for (auto &tn : _state._tempnotes)
    {
        if (tn.note == note && tn.channel == channel)
        {
            tn.done = false;
            tn.playUntil = _lastSequencerTimeInMs + length;
            return;
        }
    }

    tempnote n;
    n.playUntil = _lastSequencerTimeInMs + length;
    n.note = note;
    n.channel = channel;
    n.done = false;
    _state._tempnotes.push_back(n);
}

void AppThreeDee::Tick()
{
    std::chrono::milliseconds::rep currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    auto deltaTime = currentTime - _lastSequencerTimeInMs;

    for (auto &tn : _state._tempnotes)
    {
        if (tn.done) continue;
        if (tn.playUntil < currentTime)
        {
            tn.done = true;
            MidiEvent ev;
            ev.type = MidiEventTypes::M_NOTE;
            ev.channel = tn.channel;
            ev.value = 0;
            ev.num = tn.note;

            MidiInputManager::Instance().PutEvent(ev);
        }
    }

    _lastSequencerTimeInMs = currentTime;

    if (_state._isPlaying)
    {
        float bpmValue = float(_state._bpm) / 60.0f;
        deltaTime *= bpmValue;

        auto prevPlayTime = (_state._playTime);
        _state._playTime += deltaTime;
        auto currentPlayTime = _state._playTime;

        for (unsigned char trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; trackIndex++)
        {
            auto *track = _state._mixer->GetTrack(trackIndex);
            if (!track->Penabled)
            {
                continue;
            }
            for (auto &region : _state._regions.GetRegionsByTrack(trackIndex))
            {
                for (int i = 0; i <= region.repeat; i++)
                {
                    TickRegion(region, trackIndex, prevPlayTime, currentPlayTime, i);
                }
            }
        }

        if (_state._playTime >= _state._maxPlayTime)
        {
            _state._playTime = 0;
        }
    }
}

void AppThreeDee::PianoRollEditor()
{
    bool regionIsModified = false;
    static struct TrackRegionEvent *selectedEvent = nullptr;
    static bool drumMode = false;

    auto track = _state._mixer->GetTrack(_state._currentTrack);

    ImGui::SetNextWindowSize(ImVec2(400, 400));
    if (ImGui::BeginChild("Pianoroll editor", ImVec2(), false) && track != nullptr && _state._regions.DoesRegionExist(_state._currentTrack, _state._currentPattern))
    {
        auto &region = _state._regions.GetRegion(_state._currentTrack, _state._currentPattern);
        auto maxvalue = region.startAndEnd[1] - region.startAndEnd[0];

        if (ImGui::Button("Toggle DrumMode"))
        {
            drumMode = !drumMode;
        }

        ImGui::Text("Zoom");
        ImGui::SameLine();
        ImGui::PushItemWidth(120);
        ImGui::SliderInt("##horizontalZoom", &(_state._pianoRollEditorHorizontalZoom), 100, 300, "horizontal %d");

        ImGui::SameLine();

        static unsigned int current_snapping_mode = 2;

        ImGui::PushItemWidth(100);
        if (ImGui::BeginCombo("Snapping mode", SnappingModes[current_snapping_mode]))
        {
            for (unsigned int n = 0; n < SnappingModeCount; n++)
            {
                bool is_selected = (current_snapping_mode == n);
                if (ImGui::Selectable(SnappingModes[n], is_selected))
                {
                    current_snapping_mode = n;
                }
            }

            ImGui::EndCombo();
        }
        ImGui::ShowTooltipOnHover("Set the Snap value for the Piano Roll Editor");

        timestep elapsedTime = (static_cast<unsigned>(_state._playTime)) - region.startAndEnd[0];

        static unsigned int _baseNote = 65;

        if (ImGui::BeginChild("##timelinechild", ImVec2(0, -60)))
        {
            auto hue = _state._currentTrack * 0.05f;
            auto tintColor = ImColor::HSV(hue, 0.6f, 0.6f);

            unsigned int from = NUM_MIDI_NOTES - 1, to = 0;

            if (drumMode)
            {
                from = SAMPLE_NOTE_MAX;
                to = SAMPLE_NOTE_MIN - 1;
            }

            if (ImGui::BeginTimelines("MyTimeline", &maxvalue, 20, _state._pianoRollEditorHorizontalZoom, from - to + 4, SnappingModeValues[current_snapping_mode]))
            {
                ImGui::TimelineSetVar(ImGui::TimelineVars::ShowAddRemoveButtons, 0);
                ImGui::TimelineSetVar(ImGui::TimelineVars::ShowMuteSoloButtons, 0);

                for (unsigned int c = from; c > to; c--)
                {
                    char id[32];
                    if (drumMode)
                    {
                        sprintf(id, "%s", zyn::ui::SampleNote::NoteToString(c).c_str());
                    }
                    else
                    {
                        sprintf(id, "%4s%d", NoteNames[(107 - c) % NoteNameCount], (107 - c) / NoteNameCount - 1);
                    }
                    ImGui::TimelineStart(id, drumMode);
                    if (ImGui::IsItemClicked())
                    {
                        TempNoteOn(track->Prcvchn, c, 400);
                    }

                    for (size_t i = 0; i < region.eventsByNote[c].size(); i++)
                    {
                        bool selected = (&(region.eventsByNote[c][i]) == selectedEvent);
                        if (ImGui::TimelineEvent(region.eventsByNote[c][i].values, 0, tintColor, &selected))
                        {
                            regionIsModified = true;
                            selectedEvent = &(region.eventsByNote[c][i]);
                            _baseNote = c;
                        }
                    }
                    timestep new_values[2];
                    if (ImGui::TimelineEnd(new_values))
                    {
                        regionIsModified = true;
                        TrackRegionEvent e{
                            {
                                std::min(new_values[0], new_values[1]),
                                std::max(new_values[0], new_values[1]),
                            },
                            static_cast<unsigned char>(c),
                            100,
                        };

                        region.eventsByNote[c].push_back(e);
                        selectedEvent = &(region.eventsByNote[c].back());
                        _baseNote = c;
                    }
                }
            }
            ImGui::EndTimelines(&elapsedTime);

            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Delete)) && selectedEvent != nullptr)
            {
                if (_state._regions.DoesRegionExist(_state._currentTrack, _state._currentPattern))
                {
                    _state._regions.RemoveRegionEvent(_state._currentTrack, _state._currentPattern, *selectedEvent);
                    regionIsModified = true;
                }
            }
        }
        ImGui::EndChild();

        if (ImGui::Button("Clear selected") && selectedEvent != nullptr)
        {
            _state._regions.ClearSelectedInRegion(region, *selectedEvent);
            regionIsModified = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Clear all notes"))
        {
            _state._regions.ClearAllNotesInRegion(region);
            regionIsModified = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Clear all but selected") && selectedEvent != nullptr)
        {
            _state._regions.ClearAllButSelectedInRegion(region, *selectedEvent);
            regionIsModified = true;
        }

        static unsigned char selectedArpMode = 0;
        static unsigned char selectedChord = 0;
        static int space = 3;
        if (ImGui::Button("Generate Notes from selection"))
        {
            NotesGeneratorOptions options = {
                ArpModes::ToEnum(selectedArpMode),
                Chords::ToEnum(selectedChord),
                space,
            };
            NotesGenerator generator(options);
            generator.Generate(&(_state._regions), _state._currentTrack, _state._currentPattern, *selectedEvent);
            regionIsModified = true;
        }

        ImGui::SameLine();

        ImGui::PushItemWidth(200);
        ImGui::DropDown("##ArpMode", selectedArpMode, &(ArpModes::Names[0]), ArpModes::Enum::Count, "Arpeggio Mode");

        ImGui::SameLine();

        ImGui::PushItemWidth(200);
        ImGui::DropDown("##Chord", selectedChord, &(Chords::Names[0]), (Chords::Enum::Count), "Chord");

        ImGui::SameLine();

        ImGui::PushItemWidth(200);
        ImGui::SliderInt("Space", &space, 0, 16);

        if (regionIsModified)
        {
            UpdatePreviewImage(region);
        }
    }
    ImGui::EndChild();

    if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Delete)))
    {
    }
}

ImVec4 GetRulerColor()
{
    return ImGui::GetStyle().Colors[ImGuiCol_Border] * ImVec4(0.4f, 0.8f, 1.0f, 0.7f);
}

void RegionTrack(
    AppState &state,
    int trackIndex,
    timestep max)
{
    ImGuiWindow const *const win = ImGui::GetCurrentWindow();

    ImVec4 const color2 = ImGui::GetStyle().Colors[ImGuiCol_Border] * ImVec4(1.0f, 1.0f, 1.0f, 0.3f);
    ImVec4 const color3 = ImGui::GetStyle().Colors[ImGuiCol_Button];
    ImVec4 const rulerColor = GetRulerColor();

    auto width = std::max(int(ImGui::GetWindowContentRegionWidth()), int(200 + (max / 1000)));
    auto regions = state._regions.GetRegionsByTrack(trackIndex);
    ImGui::BeginChild("TrackWindow", ImVec2(width, 100));

    win->DrawList->AddRectFilled(
        ImGui::GetWindowPos() + ImVec2(0, 0),
        ImGui::GetWindowPos() + ImVec2(198, ImGui::GetWindowSize().y - 2),
        ImGui::ColorConvertFloat4ToU32(color3));

    win->DrawList->AddRectFilled(
        ImGui::GetWindowPos() + ImVec2(200, 0),
        ImGui::GetWindowPos() + ImGui::GetWindowSize() - ImVec2(0, 2),
        ImGui::ColorConvertFloat4ToU32(color2));

    ImGui::Button("Track");

    auto regionWidth = ImGui::GetWindowContentRegionWidth();

    for (int i = 200; i < regionWidth; i += 100)
    {
        win->DrawList->AddLine(
            ImGui::GetWindowPos() + ImVec2(i, 0),
            ImGui::GetWindowPos() + ImVec2(i, ImGui::GetWindowSize().y),
            ImGui::ColorConvertFloat4ToU32(rulerColor));
    }

    for (unsigned int r = 0; r < regions.size(); r++)
    {
        ImGui::PushID(r);

        auto region = regions[r];
        auto hue = trackIndex * 0.05f;
        auto tintColor = ImColor::HSV(hue, 0.6f, 0.6f);
        auto tintBorderColor = ImColor::HSV(hue, 0.6f, 0.9f);
        auto min = ImGui::GetWindowPos() + ImVec2(200 + region.startAndEnd[0] / 10, 5);
        auto max = ImGui::GetWindowPos() + ImVec2(200 + region.startAndEnd[1] / 10, 90);

        ImGui::SetCursorPos(min - ImGui::GetWindowPos());
        if (ImGui::InvisibleButton("selectRegion", max - min))
        {
            state._currentTrack = trackIndex;
            state._currentPattern = int(r);
        }

        win->DrawList->AddRectFilled(
            min,
            max,
            ImGui::ColorConvertFloat4ToU32(tintBorderColor));

        if (region.previewImage > 0)
        {
            win->DrawList->AddImage(
                reinterpret_cast<ImTextureID>(region.previewImage),
                min + ImVec2(1, 1),
                max - ImVec2(1, 1),
                ImVec2(0, 0),
                ImVec2(1, 1),
                ImGui::ColorConvertFloat4ToU32(tintColor));
        }
        else
        {
            win->DrawList->AddRectFilled(
                min + ImVec2(1, 1),
                max - ImVec2(1, 1),
                ImGui::ColorConvertFloat4ToU32(tintColor));
        }

        ImGui::PopID();
    }

    ImGui::EndChild();
}

void AppThreeDee::RegionEditor()
{
    ImGuiWindow const *const win = ImGui::GetCurrentWindow();

    ImGui::BeginChild("Regions", ImVec2(), false);

    auto regionWidth = ImGui::GetWindowContentRegionWidth();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2());
    ImGui::BeginChild("Region ruler", ImVec2(0, 30), false);

    for (int i = 200; i < regionWidth; i += 100)
    {
        win->DrawList->AddLine(
            ImGui::GetWindowPos() + ImVec2(i, 0),
            ImGui::GetWindowPos() + ImVec2(i, ImGui::GetWindowSize().y),
            ImGui::ColorConvertFloat4ToU32(GetRulerColor()));
    }

    ImGui::EndChild();

    if (ImGui::BeginChild("Region editor", ImVec2(0, 0), false))
    {
        timestep max = 40000;
        for (short int trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; trackIndex++)
        {
            auto regions = _state._regions.GetRegionsByTrack(trackIndex);
            for (auto region : regions)
            {
                if (max < region.startAndEnd[1])
                {
                    max = region.startAndEnd[1];
                }
            }
        }

        for (short int trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; trackIndex++)
        {
            ImGui::PushID(trackIndex);

            RegionTrack(_state, trackIndex, max);

            ImGui::PopID();
        }
        //        ImGui::ScrollbarEx();
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::EndChild();
    //    ImGui::SetNextWindowSize(ImVec2(400, 400));
    //    if (ImGui::BeginChild("Region editor", ImVec2(), false))
    //    {
    //        ImGui::Text("Zoom");
    //        ImGui::SameLine();
    //        ImGui::PushItemWidth(120);
    //        ImGui::SliderInt("##horizontalZoom", &(_state._sequencerHorizontalZoom), 10, 100, "horizontal %d");
    //        ImGui::SameLine();
    //        ImGui::PushItemWidth(120);
    //        ImGui::SliderInt("##verticalZoom", &(_state._sequencerVerticalZoom), 50, 100, "vertical %d");
    //        ImGui::SameLine();
    //        ImGui::PushItemWidth(220);
    //        int maxPlayTime = int(_state._maxPlayTime / 1024);
    //        if (ImGui::SliderInt("##maxPlayTime", &maxPlayTime, 4, 200, "song length %d"))
    //        {
    //            _state._maxPlayTime = maxPlayTime * 1024;
    //        }

    //        timestep elapsedTimeSequencer = _state._playTime % _state._maxPlayTime;

    //        if (ImGui::BeginChild("##timeline2child", ImVec2(0, -30)))
    //        {
    //            timestep maxValue = _state._maxPlayTime;
    //            if (ImGui::BeginTimelines("MyTimeline2", &maxValue, _state._sequencerVerticalZoom, _state._sequencerHorizontalZoom, NUM_MIXER_TRACKS, 1024))
    //            {
    //                ImGui::TimelineSetVar(ImGui::TimelineVars::ShowAddRemoveButtons, 1);
    //                ImGui::TimelineSetVar(ImGui::TimelineVars::ShowMuteSoloButtons, 1);
    //                for (short int trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; trackIndex++)
    //                {
    //                    ImGui::PushID(trackIndex);
    //                    auto track = _state._mixer->GetTrack(trackIndex);
    //                    auto hue = trackIndex * 0.05f;
    //                    auto tintColor = ImColor::HSV(hue, 0.6f, 0.6f);

    //                    char id[32];
    //                    sprintf(id, "Track %d", trackIndex);
    //                    bool muted = !track->Penabled;
    //                    bool solo = trackIndex == _state._mixer->Psolotrack;
    //                    ImGui::TimelineStart(id, false, &muted, &solo);
    //                    if (solo)
    //                    {
    //                        _state._mixer->Psolotrack = trackIndex;
    //                        muted = false;
    //                    }
    //                    else if (!solo && trackIndex == _state._mixer->Psolotrack)
    //                    {
    //                        _state._mixer->Psolotrack = DISABLED_MIXER_SOLO;
    //                    }
    //                    track->Penabled = !muted;

    //                    if (ImGui::IsItemClicked())
    //                    {
    //                        _state._currentTrack = trackIndex;
    //                    }

    //                    auto &regions = _state._regions.GetRegionsByTrack(trackIndex);
    //                    for (size_t i = 0; i < regions.size(); i++)
    //                    {
    //                        bool selected = (trackIndex == _state._currentTrack && int(i) == _state._currentPattern);
    //                        if (ImGui::TimelineEvent(regions[i].startAndEnd, regions[i].previewImage, tintColor, &selected))
    //                        {
    //                            _state._currentTrack = trackIndex;
    //                            _state._currentPattern = int(i);
    //                            UpdatePreviewImage(regions[i]);
    //                        }
    //                        auto x = regions[i].startAndEnd[1] - regions[i].startAndEnd[0];
    //                        for (int j = 1; j <= regions[i].repeat; j++)
    //                        {
    //                            timestep repeat_values[2]{regions[i].startAndEnd[0] + (x * j), regions[i].startAndEnd[1] + (x * j)};
    //                            ImGui::TimelineReadOnlyEvent(repeat_values, regions[i].previewImage, tintColor);
    //                        }
    //                    }

    //                    TrackRegion newRegion;
    //                    if (ImGui::TimelineEnd(newRegion.startAndEnd))
    //                    {
    //                        _state._currentTrack = trackIndex;
    //                        _state._currentPattern = int(_state._regions.GetRegionsByTrack(trackIndex).size());

    //                        UpdatePreviewImage(newRegion);
    //                        _state._regions.AddRegion(trackIndex, newRegion);
    //                    }
    //                    ImGui::PopID();
    //                }
    //            }
    //            if (ImGui::EndTimelines(&elapsedTimeSequencer))
    //            {
    //                _state._maxPlayTime = maxValue;
    //            }
    //            _state._playTime = elapsedTimeSequencer;

    //            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && ImGui::IsKeyReleased(ImGui::GetKeyIndex(ImGuiKey_Delete)))
    //            {
    //                _state._regions.RemoveRegion(_state._currentTrack, _state._currentPattern);
    //            }
    //        }
    //        ImGui::EndChild();
    //    }
    //    ImGui::EndChild();
}

void AppThreeDee::NewFile()
{
    _currentFileName = "";

    _state._regions = RegionsManager();
    _state._regions.AddRegion(0, 0, 4 * 1024);
    _state._currentPattern = 0;
    _state._currentTrack = 0;
}

void AppThreeDee::OpenFile()
{
    SaveToFileSerializer()
        .LoadWorkspace(_state._mixer, &_state._regions, _currentFileName);
}

void AppThreeDee::SaveFile()
{
    SaveToFileSerializer()
        .SaveWorkspace(_state._mixer, &_state._regions, _currentFileName);
}

void AppThreeDee::RenderDialogs()
{
    auto result = _dialogs.RenderSaveFileDialog();
    if (result == zyn::ui::DialogResults::Ok)
    {
        _currentFileName = _dialogs.GetSaveFileName();
        SaveFile();
        ImGui::CloseCurrentPopup();
    }
    if (result == zyn::ui::DialogResults::Cancel)
    {
        ImGui::CloseCurrentPopup();
    }

    result = _dialogs.RenderOpenFileDialog();
    if (result == zyn::ui::DialogResults::Ok)
    {
        _currentFileName = _dialogs.GetOpenFileName();
        OpenFile();
        ImGui::CloseCurrentPopup();
    }
    if (result == zyn::ui::DialogResults::Cancel)
    {
        ImGui::CloseCurrentPopup();
    }
}

void AppThreeDee::ChangeAppMode(AppMode appMode)
{
    _state._uiState._activeMode = appMode;
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
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_MenuBar;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    bool sf = false, of = false;
    ImGui::Begin("TestDockspace", nullptr, window_flags);
    {
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("New"))
                {
                    NewFile();
                }
                if (ImGui::MenuItem("Open"))
                {
                    of = true;
                }
                if (ImGui::MenuItem("Save"))
                {
                    if (_currentFileName == "")
                    {
                        sf = true;
                    }
                    else
                    {
                        SaveFile();
                    }
                }
                if (ImGui::MenuItem("Save As..."))
                {
                    sf = true;
                }

                ImGui::Separator();
                if (ImGui::MenuItem("Quit"))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (sf)
        {
            _dialogs.SaveFileDialog(zyn::ui::Dialogs::SAVEFILE_DIALOG_ID);
        }
        if (of)
        {
            _dialogs.OpenFileDialog(zyn::ui::Dialogs::OPENFILE_DIALOG_ID);
        }

        RenderDialogs();

        ImGui::BeginChild("ActivityBar", ImVec2(42, -50), false);
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10, 10));

            bool regions = _state._uiState._activeMode == AppMode::Regions;
            if (ImGui::ImageToggleButton("Activity_Regions", &regions, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Regions)]), ImVec2(32, 32)) && regions)
            {
                ChangeAppMode(AppMode::Regions);
            }
            ImGui::ShowTooltipOnHover("Show/Hide Regions");

            bool editor = _state._uiState._activeMode == AppMode::Editor;
            if (ImGui::ImageToggleButton("Activity_Editor", &editor, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Editor)]), ImVec2(32, 32)) && editor)
            {
                ChangeAppMode(AppMode::Editor);
            }
            ImGui::ShowTooltipOnHover("Show/Hide Editor");

            bool instrument = _state._uiState._activeMode == AppMode::Instrument;
            if (ImGui::ImageToggleButton("Activity_Instrument", &instrument, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Piano)]), ImVec2(32, 32)) && instrument)
            {
                ChangeAppMode(AppMode::Instrument);
            }
            ImGui::ShowTooltipOnHover("Show/Hide Instrument");

            bool mixer = _state._uiState._activeMode == AppMode::Mixer;
            if (ImGui::ImageToggleButton("Activity_Mixer", &mixer, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Mixer)]), ImVec2(32, 32)) && mixer)
            {
                ChangeAppMode(AppMode::Mixer);
            }
            ImGui::ShowTooltipOnHover("Show/Hide Mixer");

            bool effect = _state._uiState._activeMode == AppMode::Effect;
            if (ImGui::ImageToggleButton("Activity_Effect", &effect, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Effect)]), ImVec2(32, 32)) && effect)
            {
                ChangeAppMode(AppMode::Effect);
            }
            ImGui::ShowTooltipOnHover("Show/Hide Effect");

            ImGui::PopStyleVar();

            ImGui::EndChild();
        }

        ImGui::SameLine();

        if (_state._uiState._activeMode != AppMode::Mixer)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(5, 10));

            _mixerUi.RenderTrack(_state._currentTrack);

            ImGui::PopStyleVar();

            ImGui::SameLine();
        }

        switch (_state._uiState._activeMode)
        {
            case AppMode::Mixer:
            {
                _mixerUi.Render();
                break;
            }
            case AppMode::Editor:
            {
                PianoRollEditor();
                break;
            }
            case AppMode::Regions:
            {
                RegionEditor();
                break;
            }
            case AppMode::Instrument:
            {
                ImGui::BeginChild("Instrument", ImVec2(), false);
                static int visibleInstrument = 0;

                bool instrument = visibleInstrument == 0;
                bool checked = _state._mixer->GetTrack(_state._currentTrack)->Instruments[_state._currentTrackInstrument].Padenabled != 0;
                auto changed = ImGui::ToggleButtonWithCheckbox("Add", &instrument, &checked, ImVec2(96, 32));
                if (instrument || changed)
                {
                    visibleInstrument = 0;
                    _state._mixer->GetTrack(_state._currentTrack)->Instruments[_state._currentTrackInstrument].Padenabled = checked ? 1 : 0;
                }
                ImGui::ShowTooltipOnHover("Show/Hide Add synth");

                ImGui::SameLine();

                instrument = visibleInstrument == 1;
                checked = _state._mixer->GetTrack(_state._currentTrack)->Instruments[_state._currentTrackInstrument].Psubenabled != 0;
                changed = ImGui::ToggleButtonWithCheckbox("Sub", &instrument, &checked, ImVec2(96, 32));
                if (instrument || changed)
                {
                    visibleInstrument = 1;
                    _state._mixer->GetTrack(_state._currentTrack)->Instruments[_state._currentTrackInstrument].Psubenabled = checked ? 1 : 0;
                }
                ImGui::ShowTooltipOnHover("Show/Hide Subtractive synth");

                ImGui::SameLine();

                instrument = visibleInstrument == 2;
                checked = _state._mixer->GetTrack(_state._currentTrack)->Instruments[_state._currentTrackInstrument].Ppadenabled != 0;
                changed = ImGui::ToggleButtonWithCheckbox("Pad", &instrument, &checked, ImVec2(96, 32));
                if (instrument || changed)
                {
                    visibleInstrument = 2;
                    _state._mixer->GetTrack(_state._currentTrack)->Instruments[_state._currentTrackInstrument].Ppadenabled = checked ? 1 : 0;
                }
                ImGui::ShowTooltipOnHover("Show/Hide Pad synth");

                ImGui::SameLine();

                instrument = visibleInstrument == 3;
                checked = _state._mixer->GetTrack(_state._currentTrack)->Instruments[_state._currentTrackInstrument].Psmplenabled != 0;
                changed = ImGui::ToggleButtonWithCheckbox("Smplr", &instrument, &checked, ImVec2(96, 32));
                if (instrument || changed)
                {
                    visibleInstrument = 3;
                    _state._mixer->GetTrack(_state._currentTrack)->Instruments[_state._currentTrackInstrument].Psmplenabled = checked ? 1 : 0;
                }
                ImGui::ShowTooltipOnHover("Show/Hide Sampler");

                if (visibleInstrument == 0)
                {
                    _adNoteUI._OscilGen = &_oscilGenUi;
                    _adNoteUI.Render();
                }
                if (visibleInstrument == 1) _subNoteUi.Render();
                if (visibleInstrument == 2) _padNoteUi.Render();
                if (visibleInstrument == 3) _smplNoteUi.Render();
                ImGui::EndChild();
                break;
            }
            case AppMode::Effect:
            {
                _effectUi.Render();
                break;
            }
        }

        //        ImGuiPlayback();

        //        _libraryUi.Render();

        //        _mixerUi.RenderInspector();

        //        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(15, 10));
        //        _mixerUi.Render();

        //        _adNoteUI.Render();
        //        _padNoteUi.Render();
        //        _subNoteUi.Render();
        //        _smplNoteUi.Render();
        //        _oscilGenUi.Render();
        //        _effectUi.Render();

        //        ImGui::PopStyleVar();

        //        PianoRollEditor();

        //        RegionEditor();
    }
    ImGui::End();
    ImGui::PopStyleVar(2);

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

void gen_random(char *s, const int len)
{
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i)
    {
        s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    s[len] = 0;
}

void AppThreeDee::ImGuiPlayback()
{
    /*    auto width = ImGui::GetWindowContentRegionWidth();

    ImGui::BeginChild("Playback", ImVec2(0, 60));
    {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2, 5));

        ImGui::BeginChild("Playback_Left", ImVec2(480, 60));
        {
            ImGui::ImageToggleButton("toolbar_library", &_state._showLibrary, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Library)]), ImVec2(32, 32));
            ImGui::ShowTooltipOnHover("Show/Hide Library");

            ImGui::SameLine();

            ImGui::ImageToggleButton("toolbar_inspector", &_state._showInspector, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Inspector)]), ImVec2(32, 32));
            ImGui::ShowTooltipOnHover("Show/Hide Inspector");

            ImGui::SameLine();

            ImGui::ImageToggleButton("toolbar_quick_help", &_state._showQuickHelp, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::QuickHelp)]), ImVec2(32, 32));
            ImGui::ShowTooltipOnHover("Show/Hide Quick Help");

            ImGui::SameLine(0.0f, 8.0f);

            ImGui::ImageToggleButton("toolbar_mixer", &_state._showMixer, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Mixer)]), ImVec2(32, 32));
            ImGui::ShowTooltipOnHover("Show/Hide Mixer");

            ImGui::SameLine();

            ImGui::ImageToggleButton("toolbar_editor", &_state._showEditor, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Editor)]), ImVec2(32, 32));
            ImGui::ShowTooltipOnHover("Show/Hide Editor");

            ImGui::SameLine();

            ImGui::ImageToggleButton("toolbar_smart_controls", &_state._showSmartControls, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::SmartControls)]), ImVec2(32, 32));
            ImGui::ShowTooltipOnHover("Show/Hide Smart Controls");

            ImGui::SameLine(0.0f, 8.0f);

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
                _state._isPlaying = false;
            }

            ImGui::SameLine();

            bool isPlaying = _state._isPlaying;
            if (ImGui::ImageToggleButton("toolbar_play", &isPlaying, reinterpret_cast<ImTextureID>(_toolbarIcons[int(ToolbarTools::Play)]), ImVec2(32, 32)))
            {
                _state._isPlaying = !_state._isPlaying;
            }

            ImGui::SameLine();
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("Playback_Right", ImVec2(0, 60));
        {
            ImGui::PushItemWidth(100);
            ImGui::InputInt("##bpm", &(_state._bpm));

            ImGui::SameLine();

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0 / static_cast<double>(ImGui::GetIO().Framerate), static_cast<double>(ImGui::GetIO().Framerate));
        }
        ImGui::EndChild();

        ImGui::PopStyleVar();
    }
    ImGui::EndChild();
*/
}

void AppThreeDee::Cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
