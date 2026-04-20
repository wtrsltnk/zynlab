#include <algorithm>
#include <application.h>
#include <imgui.h>
#include <iostream>
#include <numeric>
#include <zyn.common/Config.h>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/IMidiEventHandler.h>
#include <zyn.common/globals.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/MidiInputManager.h>
#include <zyn.nio/Nio.h>

const int ToolBarHeight = 50;
const int ClipLibraryWidth = 300;
const int StatusBarHeight = 40;

static ImGuiID ClipLibraryDockID = 345678;
static ImGuiID ArrangementDockID = 3745678;
static ImGuiID ClipEditorDockID = 93765;

class Application :
    public IApplication,
    public INoteSource
{
    Mixer *_mixer;

    struct Note
    {
        int pitch;        // 0-7 for C3 to C4
        int startBeat;    // 0-15 for the 16 beats
        int duration = 1; // Usually 1 for this simple app
    };

    class Clip
    {
    private:
        int type; // 0: blue, 1: pink, 2: green
    public:
        std::vector<Note> notes;

        Clip() : type(0) {}

        int getBeats() const { return 16; } // Fixed at 16 beats
        int getType() const { return type; }
        void setType(int t) { type = t; }
    };

    struct ClipInstance
    {
        Clip *clip = nullptr;
        int startBeat = 0;
        float startBeatF = 0.0f; // Floating-point position for accurate dragging
    };

    struct Track
    {
        std::vector<ClipInstance> clipInstances;
        bool muted;
        float volume;

        Track() : muted(false), volume(1.0f) {}
    };

    struct ClipToDelete
    {
        Clip *clipPtr = nullptr;
        size_t trackIndex = 0;
        int clipStartBeat = 0;
    };
    std::vector<Clip> clipLibrary;
    std::vector<Track> tracks;
    Clip *activeClip = nullptr;
    int bpm = 138;
    float playheadPosition = 0.0f;
    ClipToDelete clipToDelete = {nullptr, 0, 0};

    // UI state
    bool isPlaying = false;
    ImVec2 gridCellSize;
    int selectedTool = 0;
    int playContext = 0;
    int activeTrack = 0;

    float currentBeatF = 0.0f;
    float playLength = 16.0f;

public:
    virtual std::vector<SimpleNote> GetNotes(
        unsigned int frameCount,
        unsigned int sampleRate)
    {
        std::vector<SimpleNote> result;

        if (!isPlaying) return result;

        // Compute beat duration and the beat range we're covering this frame
        float secondsPerBeat = 60.0f / float(bpm);
        float durationInSec = static_cast<float>(frameCount) / sampleRate;
        float endBeatF = currentBeatF + durationInSec / secondsPerBeat;

        for (const auto &track : tracks)
        {
            if (track.muted) continue;

            for (const auto &clipInst : track.clipInstances)
            {
                if (!clipInst.clip) continue;

                for (const auto &note : clipInst.clip->notes)
                {
                    float absoluteNoteBeat = clipInst.startBeatF + note.startBeat;

                    // If note starts within current frame's beat window
                    if ((absoluteNoteBeat >= currentBeatF && absoluteNoteBeat < endBeatF) || ((absoluteNoteBeat + playLength) >= currentBeatF && (absoluteNoteBeat + playLength) < endBeatF))
                    {
                        SimpleNote evt;

                        evt.note = note.pitch + 60; // 48 = MIDI C3
                        evt.velocity = 100;         // Fixed velocity; could be dynamic
                        evt.lengthInSec = note.duration * secondsPerBeat;
                        evt.channel = 0; // All on channel 0 for simplicity

                        result.push_back(evt);
                    }
                }
            }
        }

        currentBeatF = endBeatF;

        if (currentBeatF > playLength) currentBeatF -= playLength;

        return result;
    }

    virtual bool Setup()
    {
        Config::Current().init();

        SystemSettings::Instance().samplerate = Config::Current().cfg.SampleRate;
        SystemSettings::Instance().buffersize = Config::Current().cfg.SoundBufferSize;
        SystemSettings::Instance().oscilsize = Config::Current().cfg.OscilSize;
        SystemSettings::Instance().swaplr = Config::Current().cfg.SwapStereo;
        SystemSettings::Instance().alias();

        _mixer = new Mixer();

        _mixer->Init();
        _mixer->EnableTrack(0, 1);

        _mixer->SetNoteSource(this);
        Nio::preferedSampleRate(SystemSettings::Instance().samplerate);

        std::cerr.precision(1);
        std::cerr << std::fixed;
        std::cerr << "\nSample Rate = \t\t" << SystemSettings::Instance().samplerate << std::endl;
        std::cerr << "Sound Buffer Size = \t" << SystemSettings::Instance().buffersize << " samples" << std::endl;
        std::cerr << "Internal latency = \t\t" << SystemSettings::Instance().buffersize_f * 1000.0f / SystemSettings::Instance().samplerate_f << " ms" << std::endl;
        std::cerr << "ADsynth Oscil.Size = \t" << SystemSettings::Instance().oscilsize << " samples" << std::endl;

        if (!Nio::Start(_mixer, _mixer))
        {
            return false;
        }

        Nio::SelectSink("PA");
        Nio::SelectSource("RT");

        return true;
    }

    virtual void Render3d()
    {}

    void HitKey(
        int octave,
        int key)
    {
        MidiEvent ev;
        ev.type = MidiEventTypes::M_NOTE;
        ev.channel = 0;
        ev.num = 30 + (octave * 12) + key;
        ev.value = 100;
        MidiInputManager::Instance().PutEvent(ev);
    }

    void ReleaseKey(
        int octave,
        int key)
    {
        MidiEvent ev;
        ev.type = MidiEventTypes::M_NOTE;
        ev.channel = 0;
        ev.num = 30 + (octave * 12) + key;
        ev.value = 0;
        MidiInputManager::Instance().PutEvent(ev);
    }

    virtual void Render2d()
    {
        // Main menu bar
        RenderMainMenuBar();

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar;
        window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PopStyleVar(3);

        // Toolbar
        RenderToolbar();

        // Main UI components
        RenderClipLibrary();
        RenderClipEditor();
        RenderArrangementView();

        // Status bar
        RenderStatusBar();

        if (false)
        {
            ImGui::Begin("Piano", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
            int n = 0;

            // white keys first
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor(255, 255, 255, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.4f, 1.0f, 0.6f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.4f, 1.0f, 1.0f, 1.0f));
            ImVec2 note(10, 30), btnSize(40, 220);
            for (int i = 0; i < 28; i++)
            {
                ImGui::SetCursorPosX(note.x);
                ImGui::SetCursorPosY(note.y);
                ImGui::PushID(n++);
                auto octave = i / 7;
                auto octaveNote = i % 7;
                if (ImGui::Button("##test", btnSize))
                {
                    switch (octaveNote)
                    {
                        case 0: // A
                        {
                            ReleaseKey(octave, 0);
                            break;
                        }
                        case 1: // B
                        {
                            ReleaseKey(octave, 2);
                            break;
                        }
                        case 2: // C
                        {
                            ReleaseKey(octave, 3);
                            break;
                        }
                        case 3: // D
                        {
                            ReleaseKey(octave, 5);
                            break;
                        }
                        case 4: // E
                        {
                            ReleaseKey(octave, 7);
                            break;
                        }
                        case 5: // F
                        {
                            ReleaseKey(octave, 8);
                            break;
                        }
                        case 6: // G
                        {
                            ReleaseKey(octave, 10);
                            break;
                        }
                    }
                }
                if (ImGui::IsItemClicked(0))
                {
                    switch (octaveNote)
                    {
                        case 0: // A
                        {
                            HitKey(octave, 0);
                            break;
                        }
                        case 1: // B
                        {
                            HitKey(octave, 2);
                            break;
                        }
                        case 2: // C
                        {
                            HitKey(octave, 3);
                            break;
                        }
                        case 3: // D
                        {
                            HitKey(octave, 5);
                            break;
                        }
                        case 4: // E
                        {
                            HitKey(octave, 7);
                            break;
                        }
                        case 5: // F
                        {
                            HitKey(octave, 8);
                            break;
                        }
                        case 6: // G
                        {
                            HitKey(octave, 10);
                            break;
                        }
                    }
                }

                ImGui::SetItemAllowOverlap();
                ImGui::PopID();

                note.x += 42;
            }
            ImGui::PopStyleColor(3);

            // black keys
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.1f, 0.0f, 0.0f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.1f, 0.0f, 0.0f, 1.0f));
            ImVec2 note1 = ImVec2(35, 30);
            ImVec2 btnSize1 = ImVec2(30, 140);
            for (int i = 0; i < 28; i++)
            {
                if (i % 7 == 0 || i % 7 == 2 || i % 7 == 3 || i % 7 == 5 || i % 7 == 6)
                {
                    ImGui::SetCursorPosX(note1.x);
                    ImGui::SetCursorPosY(note1.y);
                    ImGui::PushID(n++);
                    if (ImGui::Button("##test", btnSize1))
                    {
                        auto octave = i / 7;
                        auto octaveNote = i % 7;
                        switch (octaveNote)
                        {
                            case 0: // A#
                            {
                                HitKey(octave, 1);
                                break;
                            }
                            case 2: // C#
                            {
                                HitKey(octave, 4);
                                break;
                            }
                            case 3: // D#
                            {
                                HitKey(octave, 6);
                                break;
                            }
                            case 4: // F#
                            {
                                HitKey(octave, 9);
                                break;
                            }
                            case 5: // G#
                            {
                                HitKey(octave, 11);
                                break;
                            }
                        }
                    }
                    ImGui::PopID();
                }
                note1.x += 42;
            }
            ImGui::PopStyleColor(2);

            ImGui::End();
        }
    }

    virtual void Cleanup()
    {
        // ensure that everything has stopped with the mutex wait
        _mixer->Lock();
        _mixer->Unlock();

        Nio::Stop();

        delete _mixer;
        _mixer = nullptr;
    }

    void RenderMainMenuBar()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Create"))
                {
                }
                if (ImGui::MenuItem("Open", "Ctrl+O"))
                {
                }
                if (ImGui::MenuItem("Save", "Ctrl+S"))
                {
                }
                if (ImGui::MenuItem("Save as.."))
                {
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void RenderClipLibrary()
    {
        ImGui::SetNextWindowPos(ImVec2(0, ToolBarHeight + 20));
        ImGui::SetNextWindowSize(ImVec2(ClipLibraryWidth, ImGui::GetIO().DisplaySize.y - (ToolBarHeight + 20) - StatusBarHeight));

        if (ImGui::Begin("Clip Library", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
        {
            // Add new clip button
            if (ImGui::Button("New Clip", ImVec2(120, 30)))
            {
                clipLibrary.push_back(Clip());
            }

            ImGui::Spacing();

            // Display clips as selectable items with preview
            for (size_t i = 0; i < clipLibrary.size(); i++)
            {
                ImGui::PushID(static_cast<int>(i));

                // Draw clip preview
                ImVec2 previewSize(140, 60);
                ImVec2 cursorPos = ImGui::GetCursorScreenPos();
                ImDrawList *drawList = ImGui::GetWindowDrawList();

                // Draw the clip background
                drawList->AddRectFilled(
                    cursorPos,
                    ImVec2(cursorPos.x + previewSize.x, cursorPos.y + previewSize.y),
                    IM_COL32(74, 74, 88, 255),
                    3.0f);

                // Draw the piano roll background
                drawList->AddRectFilled(
                    ImVec2(cursorPos.x + 10, cursorPos.y + 10),
                    ImVec2(cursorPos.x + previewSize.x - 10, cursorPos.y + previewSize.y - 10),
                    IM_COL32(42, 42, 56, 255));

                // Draw some horizontal lines
                for (int j = 1; j < 4; j++)
                {
                    float y = cursorPos.y + 10 + (j * 10);
                    drawList->AddLine(
                        ImVec2(cursorPos.x + 10, y),
                        ImVec2(cursorPos.x + previewSize.x - 10, y),
                        IM_COL32(85, 85, 101, 255));
                }

                // Draw sample notes in the preview
                DrawClipPreviewNotes(drawList, clipLibrary[i], cursorPos, previewSize);

                ImGui::InvisibleButton("##clippreview", previewSize);

                bool isSelected = (activeClip == &clipLibrary[i]);
                if (ImGui::IsItemClicked())
                {
                    activeClip = &clipLibrary[i];
                }

                // Start drag and drop source for clips
                if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                {
                    // Set payload to carry the index of the clip
                    int clipIdx = static_cast<int>(i);
                    ImGui::SetDragDropPayload("CLIP_ITEM", &clipIdx, sizeof(int));

                    // Preview display while dragging
                    ImGui::Text("Clip %d", clipIdx + 1);

                    // Draw a smaller preview of the clip being dragged
                    ImVec2 dragPreviewSize(100, 40);
                    ImVec2 dragCursorPos = ImGui::GetCursorScreenPos();

                    drawList->AddRectFilled(
                        dragCursorPos,
                        ImVec2(dragCursorPos.x + dragPreviewSize.x, dragCursorPos.y + dragPreviewSize.y),
                        IM_COL32(74, 74, 88, 255),
                        3.0f);

                    // Draw a mini preview of the clip's notes
                    ImU32 noteColor = 0;
                    switch (clipLibrary[i].getType())
                    {
                        case 0:
                            noteColor = IM_COL32(106, 140, 175, 255);
                            break; // Blue
                        case 1:
                            noteColor = IM_COL32(175, 106, 140, 255);
                            break; // Pink
                        case 2:
                            noteColor = IM_COL32(140, 175, 106, 255);
                            break; // Green
                        default:
                            noteColor = IM_COL32(150, 150, 150, 255);
                            break;
                    }

                    drawList->AddRectFilled(
                        ImVec2(dragCursorPos.x + 5, dragCursorPos.y + 5),
                        ImVec2(dragCursorPos.x + dragPreviewSize.x - 5, dragCursorPos.y + dragPreviewSize.y - 5),
                        IM_COL32(42, 42, 56, 255));

                    DrawClipPreviewNotes(drawList, clipLibrary[i], dragCursorPos, dragPreviewSize);

                    ImGui::EndDragDropSource();
                }

                // Highlight if selected
                if (isSelected)
                {
                    drawList->AddRect(
                        cursorPos,
                        ImVec2(cursorPos.x + previewSize.x, cursorPos.y + previewSize.y),
                        IM_COL32(106, 140, 175, 255),
                        3.0f,
                        0,
                        2.0f);
                }

                ImGui::PopID();
                ImGui::Spacing();
            }
        }
        ImGui::End();
    }

    void RenderClipEditor()
    {
        ImGui::SetNextWindowPos(ImVec2(ClipLibraryWidth, ToolBarHeight + 20));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - ClipLibraryWidth, (ImGui::GetIO().DisplaySize.y - (ToolBarHeight + 20) - StatusBarHeight) / 2));

        if (ImGui::Begin("Clip Editor", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
        {
            if (!isPlaying && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                if (playContext != 1) playheadPosition = 0;

                playContext = 1;
                playLength = 16.0f;
            }

            if (!activeClip)
            {
                ImGui::Text("Select a clip to edit");
                ImGui::End();
                return;
            }

            // Calculate grid dimensions
            ImVec2 canvasPos = ImGui::GetCursorScreenPos();
            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            ImDrawList *drawList = ImGui::GetWindowDrawList();

            // Background
            drawList->AddRectFilled(
                canvasPos,
                ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
                IM_COL32(42, 42, 56, 255));

            // Draw beat numbers
            float beatWidth = canvasSize.x / 16.0f;
            for (int i = 0; i < 16; i++)
            {
                std::string beatLabel = std::to_string(i + 1);
                ImVec2 textSize = ImGui::CalcTextSize(beatLabel.c_str());
                ImGui::SetCursorScreenPos(ImVec2(canvasPos.x + i * beatWidth + beatWidth / 2 - textSize.x / 2, canvasPos.y));
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "%s", beatLabel.c_str());
            }

            // Note labels (C3 to C4)
            const char *noteLabels[] = {"C4", "B3", "A3", "G3", "F3", "E3", "D3", "C3"};
            float noteHeight = (canvasSize.y - 20) / 8.0f; // 20px for beat numbers
            ImGui::SetCursorScreenPos(ImVec2(canvasPos.x, canvasPos.y + 20));
            for (int i = 0; i < 8; i++)
            {
                ImGui::SetCursorScreenPos(ImVec2(canvasPos.x, canvasPos.y + 20 + i * noteHeight + noteHeight / 2));
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), "%s", noteLabels[i]);
            }

            // Grid start position (after labels)
            ImVec2 gridStartPos = ImVec2(canvasPos.x + 30, canvasPos.y + 20);
            ImVec2 gridSize = ImVec2(canvasSize.x - 30, canvasSize.y - 20);

            // Calculate cell size
            gridCellSize = ImVec2(gridSize.x / 16.0f, gridSize.y / 8.0f);

            // Draw grid lines
            for (int x = 0; x <= 16; x++)
            {
                float lineX = gridStartPos.x + x * gridCellSize.x;
                ImU32 lineColor = (x % 4 == 0) ? IM_COL32(74, 74, 88, 255) : IM_COL32(58, 58, 72, 255);
                drawList->AddLine(
                    ImVec2(lineX, gridStartPos.y),
                    ImVec2(lineX, gridStartPos.y + gridSize.y),
                    lineColor);
            }

            for (int y = 0; y <= 8; y++)
            {
                float lineY = gridStartPos.y + y * gridCellSize.y;
                drawList->AddLine(
                    ImVec2(gridStartPos.x, lineY),
                    ImVec2(gridStartPos.x + gridSize.x, lineY),
                    IM_COL32(58, 58, 72, 255));
            }

            // Draw notes
            if (activeClip)
            {
                for (const auto &note : activeClip->notes)
                {
                    float noteX = gridStartPos.x + note.startBeat * gridCellSize.x;
                    float noteY = gridStartPos.y + (7 - note.pitch) * gridCellSize.y; // 7-(0-7) for C3 to C4

                    // Draw the note rectangle
                    drawList->AddRectFilled(
                        ImVec2(noteX, noteY),
                        ImVec2(noteX + gridCellSize.x, noteY + gridCellSize.y),
                        IM_COL32(106, 140, 175, 255),
                        3.0f);
                }
            }

            // Draw playhead if playing
            if (playContext == 1)
            {
                float playheadX = gridStartPos.x + (playheadPosition + currentBeatF) * gridCellSize.x - (gridCellSize.y / 2.0f);
                drawList->AddLine(
                    ImVec2(playheadX, gridStartPos.y),
                    ImVec2(playheadX, gridStartPos.y + gridSize.y),
                    isPlaying ? IM_COL32(255, 85, 85, 255) : IM_COL32(85, 85, 85, 155),
                    2.0f);
            }

            // Handle mouse interaction for note editing
            if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(0))
            {
                ImVec2 mousePos = ImGui::GetMousePos();

                // Check if mouse is within grid
                if (mousePos.x >= gridStartPos.x && mousePos.x < gridStartPos.x + gridSize.x &&
                    mousePos.y >= gridStartPos.y && mousePos.y < gridStartPos.y + gridSize.y)
                {

                    // Calculate grid cell
                    int gridX = static_cast<int>((mousePos.x - gridStartPos.x) / gridCellSize.x);
                    int gridY = static_cast<int>((mousePos.y - gridStartPos.y) / gridCellSize.y);

                    // Add or remove note based on selected tool
                    if (selectedTool == 0)
                    { // Add note
                        Note newNote;
                        newNote.startBeat = gridX;
                        newNote.pitch = 7 - gridY; // Convert from grid Y to pitch
                        newNote.duration = 1;

                        // Check if note already exists and remove it if it does
                        bool noteExists = false;
                        for (auto it = activeClip->notes.begin(); it != activeClip->notes.end();)
                        {
                            if (it->startBeat == newNote.startBeat && it->pitch == newNote.pitch)
                            {
                                it = activeClip->notes.erase(it);
                                noteExists = true;
                            }
                            else
                            {
                                ++it;
                            }
                        }

                        // Add the note if it didn't exist
                        if (!noteExists)
                        {
                            activeClip->notes.push_back(newNote);
                        }
                    }
                }
            }

            // Need an invisible button to capture mouse events
            ImGui::SetCursorScreenPos(canvasPos);
            ImGui::InvisibleButton("canvas", canvasSize);
        }
        ImGui::End();
    }

    void RenderArrangementView()
    {
        ImGui::SetNextWindowPos(ImVec2(ClipLibraryWidth, ToolBarHeight + 20 + (ImGui::GetIO().DisplaySize.y - (ToolBarHeight + 20) - StatusBarHeight) / 2));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - ClipLibraryWidth, (ImGui::GetIO().DisplaySize.y - (ToolBarHeight + 20) - StatusBarHeight) / 2));

        if (ImGui::Begin("Arrangement", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
        {
            if (!isPlaying && ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows))
            {
                if (playContext != 2) playheadPosition = 0;

                playContext = 2;
                auto count_beats = [](int a, Clip b) {
                    return a + b.getBeats();
                };
                playLength = std::accumulate(clipLibrary.begin(), clipLibrary.end(), 0, count_beats);
            }

            ImVec2 canvasPos = ImGui::GetCursorScreenPos();
            ImVec2 canvasSize = ImGui::GetContentRegionAvail();
            ImDrawList *drawList = ImGui::GetWindowDrawList();

            // Track height
            float trackHeight = 70.0f;
            float trackLabelWidth = 40.0f;
            float beatWidth = 10.0f; // Width of one beat in pixels

            // Draw tracks
            for (size_t i = 0; i < tracks.size(); i++)
            {
                ImGui::PushID(i);

                float trackY = canvasPos.y + i * trackHeight;

                // Track label background
                drawList->AddRectFilled(
                    ImVec2(canvasPos.x, trackY),
                    ImVec2(canvasPos.x + trackLabelWidth, trackY + trackHeight),
                    (activeTrack == i) ? IM_COL32(114, 114, 178, 255) : IM_COL32(58, 58, 72, 255));

                // Track label
                std::string trackLabel = "T" + std::to_string(i + 1);
                ImVec2 textSize = ImGui::CalcTextSize(trackLabel.c_str());
                drawList->AddText(
                    ImVec2(canvasPos.x + trackLabelWidth / 2 - textSize.x / 2,
                           trackY + trackHeight / 2 - textSize.y / 2),
                    IM_COL32(255, 255, 255, 255),
                    trackLabel.c_str());

                ImGui::InvisibleButton(("trackLabel_" + std::to_string(i)).c_str(),
                                       ImVec2(trackLabelWidth, trackHeight));

                if (ImGui::IsItemClicked(0))
                {
                    activeTrack = i;
                }

                // Track background
                drawList->AddRectFilled(
                    ImVec2(canvasPos.x + trackLabelWidth, trackY),
                    ImVec2(canvasPos.x + canvasSize.x, trackY + trackHeight),
                    IM_COL32(34, 34, 48, 255));

                // Draw grid lines for beats in the track
                for (int beat = 0; beat <= 64; beat += 4)
                { // Assuming visible area covers 64 beats
                    float lineX = canvasPos.x + trackLabelWidth + beat * beatWidth;
                    ImU32 lineColor = (beat % 16 == 0) ? IM_COL32(80, 80, 100, 255) : IM_COL32(60, 60, 80, 150);
                    float lineThickness = (beat % 16 == 0) ? 1.0f : 0.5f;

                    if (lineX <= canvasPos.x + canvasSize.x)
                    { // Only draw visible lines
                        drawList->AddLine(
                            ImVec2(lineX, trackY),
                            ImVec2(lineX, trackY + trackHeight),
                            lineColor,
                            lineThickness);
                    }
                }

                int clipInstanceId = 0;
                // Draw clips on this track
                for (const auto &clipInstance : tracks[i].clipInstances)
                {
                    ImGui::PushID(clipInstanceId);
                    float clipX = canvasPos.x + trackLabelWidth + clipInstance.startBeat * beatWidth;
                    float clipWidth = clipInstance.clip->getBeats() * beatWidth;

                    ImU32 clipColor = 0;
                    switch (clipInstance.clip->getType())
                    {
                        case 0:
                            clipColor = IM_COL32(106, 140, 175, 255);
                            break; // Blue
                        case 1:
                            clipColor = IM_COL32(175, 106, 140, 255);
                            break; // Pink
                        case 2:
                            clipColor = IM_COL32(140, 175, 106, 255);
                            break; // Green
                        default:
                            clipColor = IM_COL32(150, 150, 150, 255);
                            break;
                    }

                    // Only draw visible clips
                    if (clipX + clipWidth > canvasPos.x + trackLabelWidth &&
                        clipX < canvasPos.x + canvasSize.x)
                    {

                        // Clip border
                        drawList->AddRect(
                            ImVec2(clipX, trackY + 10),
                            ImVec2(clipX + clipWidth, trackY + trackHeight - 10),
                            clipColor,
                            3.0f,
                            0,
                            2.0f);

                        // Clip content background
                        drawList->AddRectFilled(
                            ImVec2(clipX + 10, trackY + 20),
                            ImVec2(clipX + clipWidth - 10, trackY + trackHeight - 20),
                            IM_COL32(42, 42, 56, 255));

                        // Draw miniature notes
                        DrawClipMiniature(
                            drawList, *clipInstance.clip,
                            ImVec2(clipX + 10, trackY + 20),
                            ImVec2(clipWidth - 20, trackHeight - 40));

                        // Add clip label
                        auto clipIndex = std::distance(
                            clipLibrary.begin(),
                            std::find_if(
                                clipLibrary.begin(), clipLibrary.end(),
                                [&](const Clip &c) { return &c == clipInstance.clip; }));

                        std::string clipLabel = "Clip " + std::to_string(clipIndex + 1);
                        ImVec2 labelSize = ImGui::CalcTextSize(clipLabel.c_str());

                        // Only draw label if clip is wide enough
                        if (clipWidth > labelSize.x + 20)
                        {
                            drawList->AddText(
                                ImVec2(clipX + clipWidth / 2 - labelSize.x / 2, trackY + 25),
                                IM_COL32(200, 200, 200, 255),
                                clipLabel.c_str());
                        }

                        // Check for clip interactions
                        ImGui::SetCursorScreenPos(ImVec2(clipX, trackY + 10));
                        ImGui::InvisibleButton(("clip_" + std::to_string(i) + "_" + std::to_string(clipIndex)).c_str(),
                                               ImVec2(clipWidth, trackHeight - 20));

                        // Context menu for clips
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                        {
                            activeTrack = i;
                            ImGui::OpenPopup(("clip_context_" + std::to_string(i) + "_" + std::to_string(clipIndex)).c_str());
                        }

                        // Drag to reposition a clip
                        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
                        {
                            activeTrack = i;
                            ImVec2 dragDelta = ImGui::GetIO().MouseDelta;

                            // Convert pixel drag to beat positions
                            float beatDelta = dragDelta.x / beatWidth;

                            // Track index where mouse is currently
                            ImVec2 mousePos = ImGui::GetMousePos();
                            int currentTrackIdx = static_cast<int>((mousePos.y - canvasPos.y) / trackHeight);

                            // Only update if in valid range
                            if (currentTrackIdx >= 0 && currentTrackIdx < static_cast<int>(tracks.size()))
                            {
                                // Find the clip instance
                                for (auto it = tracks[i].clipInstances.begin(); it != tracks[i].clipInstances.end(); ++it)
                                {
                                    if (it->clip == clipInstance.clip && it->startBeat == clipInstance.startBeat)
                                    {
                                        // Remove from current track if track changed
                                        if (currentTrackIdx != static_cast<int>(i))
                                        {
                                            ClipInstance movedInstance = *it;
                                            movedInstance.startBeatF = std::max<float>(0.0f, movedInstance.startBeatF + beatDelta);
                                            movedInstance.startBeat = static_cast<int>(movedInstance.startBeatF);
                                            tracks[currentTrackIdx].clipInstances.push_back(movedInstance);
                                            tracks[i].clipInstances.erase(it);
                                            activeTrack = currentTrackIdx;
                                            break;
                                        }
                                        else
                                        {
                                            // Update floating-point position first
                                            it->startBeatF = std::max<float>(0.0f, it->startBeatF + beatDelta);
                                            // Then update integer position for rendering
                                            it->startBeat = static_cast<int>(it->startBeatF);
                                            break;
                                        }
                                    }
                                }
                            }
                        }

                        // Context menu implementation
                        if (ImGui::BeginPopup(("clip_context_" + std::to_string(i) + "_" + std::to_string(clipIndex)).c_str()))
                        {
                            if (ImGui::MenuItem("Delete"))
                            {
                                // Mark for deletion (don't delete during iteration)
                                clipToDelete.trackIndex = i;
                                clipToDelete.clipStartBeat = clipInstance.startBeat;
                                clipToDelete.clipPtr = clipInstance.clip;
                            }

                            if (ImGui::MenuItem("Edit"))
                            {
                                // Set as active clip for editing
                                activeClip = clipInstance.clip;
                            }

                            if (ImGui::MenuItem("Duplicate"))
                            {
                                ClipInstance newInstance = clipInstance;
                                newInstance.startBeat += clipInstance.clip->getBeats(); // Place after current clip
                                newInstance.startBeatF = newInstance.startBeat;
                                tracks[i].clipInstances.push_back(newInstance);
                            }

                            ImGui::EndPopup();
                        }
                    }
                    ImGui::PopID();
                    clipInstanceId++;
                }

                // Handle drop target for each track
                ImGui::SetCursorScreenPos(ImVec2(canvasPos.x + trackLabelWidth, trackY));
                ImGui::InvisibleButton(("track_" + std::to_string(i) + "_dropzone").c_str(),
                                       ImVec2(canvasSize.x - trackLabelWidth, trackHeight));

                if (ImGui::IsItemClicked(0))
                {
                    activeTrack = i;
                }

                // Highlight track when dragging over it
                if (ImGui::IsItemHovered() && ImGui::GetDragDropPayload() != nullptr)
                {
                    drawList->AddRect(
                        ImVec2(canvasPos.x + trackLabelWidth, trackY),
                        ImVec2(canvasPos.x + canvasSize.x, trackY + trackHeight),
                        IM_COL32(106, 140, 175, 180),
                        0.0f,
                        0,
                        2.0f);
                }

                // Accept drag and drop on this track
                if (ImGui::BeginDragDropTarget())
                {
                    // Show visual feedback - snap line at drop position
                    ImVec2 mousePos = ImGui::GetMousePos();
                    int snapBeat = static_cast<int>((mousePos.x - canvasPos.x - trackLabelWidth) / beatWidth);
                    float snapX = canvasPos.x + trackLabelWidth + snapBeat * beatWidth;

                    // Draw snap indicator line
                    drawList->AddLine(
                        ImVec2(snapX, trackY),
                        ImVec2(snapX, trackY + trackHeight),
                        IM_COL32(0, 255, 0, 200),
                        2.0f);

                    // Handle actual drop
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("CLIP_ITEM"))
                    {
                        int clipIndex = *(const int *)payload->Data;

                        // Add clip to track at snapped position
                        if (clipIndex >= 0 && clipIndex < static_cast<int>(clipLibrary.size()))
                        {
                            ClipInstance newInstance;
                            newInstance.clip = &clipLibrary[clipIndex];
                            newInstance.startBeatF = newInstance.startBeat = snapBeat;
                            tracks[i].clipInstances.push_back(newInstance);

                            // Sort clip instances by start beat for consistent rendering
                            std::sort(tracks[i].clipInstances.begin(), tracks[i].clipInstances.end(),
                                      [](const ClipInstance &a, const ClipInstance &b) {
                                          return a.startBeat < b.startBeat;
                                      });
                        }
                    }
                    ImGui::EndDragDropTarget();
                }

                ImGui::PopID();
            }

            // Draw playhead in arrangement view
            if (playContext == 2)
            {
                float playheadX = canvasPos.x + trackLabelWidth + playheadPosition * beatWidth;
                drawList->AddLine(
                    ImVec2(playheadX, canvasPos.y),
                    ImVec2(playheadX, canvasPos.y + tracks.size() * trackHeight),
                    isPlaying ? IM_COL32(255, 85, 85, 255) : IM_COL32(85, 85, 85, 155),
                    2.0f);
            }

            // Handle scheduled clip deletions
            if (clipToDelete.clipPtr != nullptr)
            {
                auto &clipInstances = tracks[clipToDelete.trackIndex].clipInstances;
                for (auto it = clipInstances.begin(); it != clipInstances.end(); ++it)
                {
                    if (it->clip == clipToDelete.clipPtr && it->startBeat == clipToDelete.clipStartBeat)
                    {
                        clipInstances.erase(it);
                        break;
                    }
                }
                clipToDelete = {nullptr, 0, 0}; // Reset deletion marker
            }

            // Add track button
            ImGui::SetCursorScreenPos(ImVec2(canvasPos.x, canvasPos.y + tracks.size() * trackHeight + 5));
            if (ImGui::Button("Add Track", ImVec2(100, 25)))
            {
                tracks.push_back(Track());
            }

            // Handle horizontal scrolling for the arrangement
            ImGui::SetCursorScreenPos(ImVec2(canvasPos.x, canvasPos.y + tracks.size() * trackHeight + 40));
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

            static float scrollX = 0.0f;
            ImGui::SetNextItemWidth(canvasSize.x);
            ImGui::SliderFloat("##hscroll", &scrollX, 0.0f, 100.0f, "");
            ImGui::PopStyleVar(2);
        }
        ImGui::End();
    }

    void RenderToolbar()
    {
        ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()));
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ToolBarHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));

        if (ImGui::Begin("##toolbar", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse))
        {

            ImGui::Text("Tools:");
            ImGui::SameLine();

            // Note tool
            bool noteToolSelected = (selectedTool == 0);
            if (ImGui::Checkbox("N", &noteToolSelected))
            {
                selectedTool = 0;
            }
            ImGui::SameLine();

            // Pencil tool
            bool pencilToolSelected = (selectedTool == 1);
            if (ImGui::Checkbox("Edit", &pencilToolSelected))
            {
                selectedTool = 1;
            }
            ImGui::SameLine();

            // Cut tool
            bool cutToolSelected = (selectedTool == 2);
            if (ImGui::Checkbox("Cut", &cutToolSelected))
            {
                selectedTool = 2;
            }
            ImGui::SameLine();

            ImGui::Separator(); // Ex(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();

            // BPM control
            ImGui::SetNextItemWidth(50);
            if (ImGui::DragInt("BPM", &bpm, 1, 40, 240))
            {
                // Update tempo
            }
            ImGui::SameLine();

            ImGui::Separator(); // Ex(ImGuiSeparatorFlags_Vertical);
            ImGui::SameLine();

            const char *playPauseButtonText = "";
            if (isPlaying)
            {
                switch (playContext)
                {
                    case 0:
                    {
                        playPauseButtonText = "Pause";
                        break;
                    }
                    case 1:
                    {
                        playPauseButtonText = "Pause Clip";
                        break;
                    }
                    case 2:
                    {
                        playPauseButtonText = "Pause Song";
                        break;
                    }
                }
            }
            else
            {
                switch (playContext)
                {
                    case 0:
                    {
                        playPauseButtonText = "Play";
                        break;
                    }
                    case 1:
                    {
                        playPauseButtonText = "Play Clip";
                        break;
                    }
                    case 2:
                    {
                        playPauseButtonText = "Play Song";
                        break;
                    }
                }
            }

            // Play/Pause button
            if (ImGui::Button(playPauseButtonText, ImVec2(0, 30)))
            {
                isPlaying = !isPlaying;
            }
            ImGui::SameLine();

            // Export button
            if (ImGui::Button("Export", ImVec2(60, 30)))
            {
                // Export functionality
            }
        }
        ImGui::End();
        ImGui::PopStyleVar(2);
    }

    void RenderStatusBar()
    {
        auto *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y + viewport->Size.y - StatusBarHeight));
        ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, StatusBarHeight));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);

        if (ImGui::Begin("##statusbar", nullptr,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoCollapse))
        {

            // Project info
            ImGui::Text("Project: My Composition - %d BPM - 4/4", bpm);

            // Right-aligned clip count and duration
            ImGui::SameLine(ImGui::GetWindowWidth() - 150);
            ImGui::Text("%zu clips - 00:01:24", clipLibrary.size());
        }
        ImGui::End();
        ImGui::PopStyleVar();
    }

    // Draw miniature notes in clip thumbnails
    void DrawClipMiniature(ImDrawList *drawList, const Clip &clip, const ImVec2 &pos, const ImVec2 &size)
    {
        ImU32 noteColor = 0;
        switch (clip.getType())
        {
            case 0:
                noteColor = IM_COL32(106, 140, 175, 255);
                break; // Blue
            case 1:
                noteColor = IM_COL32(175, 106, 140, 255);
                break; // Pink
            case 2:
                noteColor = IM_COL32(140, 175, 106, 255);
                break; // Green
            default:
                noteColor = IM_COL32(150, 150, 150, 255);
                break;
        }

        float beatWidth = size.x / 16.0f;
        float noteHeight = size.y / 8.0f;

        for (const auto &note : clip.notes)
        {
            float noteX = pos.x + note.startBeat * beatWidth;
            float noteY = pos.y + (7 - note.pitch) * noteHeight;

            drawList->AddRectFilled(
                ImVec2(noteX, noteY),
                ImVec2(noteX + beatWidth, noteY + noteHeight),
                noteColor,
                2.0f);
        }
    }

    // Draw clip preview in library
    void DrawClipPreviewNotes(ImDrawList *drawList, const Clip &clip, ImVec2 pos, ImVec2 size)
    {
        ImU32 noteColor = 0;
        switch (clip.getType())
        {
            case 0:
                noteColor = IM_COL32(106, 140, 175, 255);
                break; // Blue
            case 1:
                noteColor = IM_COL32(175, 106, 140, 255);
                break; // Pink
            case 2:
                noteColor = IM_COL32(140, 175, 106, 255);
                break; // Green
            default:
                noteColor = IM_COL32(150, 150, 150, 255);
                break;
        }

        const float previewContentX = pos.x + 10;
        const float previewContentY = pos.y + 10;
        const float previewContentWidth = size.x - 20;
        const float previewContentHeight = size.y - 20;

        for (const auto &note : clip.notes)
        {
            float noteX = previewContentX + (note.startBeat / 16.0f) * previewContentWidth;
            float noteY = previewContentY + (previewContentHeight - 5 - ((note.pitch / 8.0f) * previewContentHeight));

            drawList->AddRectFilled(
                ImVec2(noteX, noteY),
                ImVec2(noteX + 10, noteY + 5),
                noteColor);
        }
    }
};

extern IApplication *CreateApplication();

IApplication *CreateApplication()
{
    return new Application();
}
