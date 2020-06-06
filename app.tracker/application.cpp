#include <algorithm>
#include <application.h>
#include <imgui.h>
#include <iostream>
#include <map>
#include <zyn.common/Config.h>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/IMidiEventHandler.h>
#include <zyn.common/globals.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/MidiInputManager.h>
#include <zyn.nio/Nio.h>
#include <zyn.serialization/LibraryManager.h>

ImVec2 operator+(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x + b.x, a.y + b.y);
}

ImVec2 operator-(ImVec2 const &a, ImVec2 const &b)
{
    return ImVec2(a.x - b.x, a.y - b.y);
}

char const *NoteToString(
    unsigned int note);

char const *ValueToString(
    unsigned int note);

namespace ImGui
{
    void MoveCursorPos(
        ImVec2 delta)
    {
        SetCursorPos(GetCursorPos() + delta);
    }
} // namespace ImGui

class Note
{
public:
    unsigned int _note;
    unsigned int _length;
    unsigned int _velocity;
};

class Pattern
{
    unsigned int _length;
    std::vector<Note> _notes[NUM_MIXER_TRACKS];

public:
    Pattern(
        unsigned int length = 64);

    unsigned int Length() const;
    std::vector<Note> &Notes(
        unsigned int trackIndex);
};

Pattern::Pattern(
    unsigned int length)
    : _length(length)
{
    for (unsigned int t = 0; t < NUM_MIXER_TRACKS; t++)
    {
        _notes[t].resize(_length);
    }
}

unsigned int Pattern::Length() const
{
    return _length;
}

std::vector<Note> &Pattern::Notes(
    unsigned int trackIndex)
{
    return _notes[trackIndex];
}

class Application :
    public IApplication
{
    Mixer *_mixer;
    ILibraryManager *_library;
    Pattern *_pattern;

    ImFont *_monofont;

public:
    Application()
        : _mixer(nullptr),
          _library(nullptr),
          _pattern(nullptr),
          _monofont(nullptr)
    {}

    virtual bool Setup()
    {
        ImGuiIO &io = ImGui::GetIO();
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
        _monofont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\SourceCodePro-Bold.ttf", 14.0f);
        io.Fonts->Build();

        Config::init();

        SystemSettings::Instance().samplerate = Config::Current().cfg.SampleRate;
        SystemSettings::Instance().buffersize = Config::Current().cfg.SoundBufferSize;
        SystemSettings::Instance().oscilsize = Config::Current().cfg.OscilSize;
        SystemSettings::Instance().alias();

        _mixer = new Mixer();

        _mixer->Init();

        _library = new LibraryManager();

        for (int i = 0; i < MAX_BANK_ROOT_DIRS; i++)
        {
            if (Config::Current().cfg.bankRootDirList[i].size() == 0)
            {
                continue;
            }
            std::cout << Config::Current().cfg.bankRootDirList[i] << std::endl;
            _library->AddLibraryLocation(Config::Current().cfg.bankRootDirList[i]);
        }

        _library->RefreshLibraries();

        Nio::preferedSampleRate(SystemSettings::Instance().samplerate);

        if (!Nio::Start(_mixer, _mixer))
        {
            return false;
        }

        Nio::SelectSink("PA");
        Nio::SelectSource("RT");

        _pattern = new Pattern(64);

        _pattern->Notes(0)[0]._note = 50;
        _pattern->Notes(0)[0]._length = 64;

        return true;
    }

    virtual void Render3d()
    {}

    const unsigned int numTracks = NUM_MIXER_TRACKS;
    unsigned int currentRow = 0;
    unsigned int currentTrack = 0;
    unsigned int currentProperty = 0;
    unsigned int skipRowStepSize = 4;

    const char *emptyCellNote = "---";
    const char *emptyCellParameter = "..";
    const char *emptyCellFx = "000";

    const int headerHeight = 85;
    const int footerHeight = 20;
    const int rowIndexColumnWidth = 30;
    const int scrollbarHeight = 20;

    std::map<char, unsigned int> _charToNoteMap{
        {'Z', 60}, // C-4
        {'S', 61}, // C#4
        {'X', 62}, // D-4
        {'D', 63}, // D#4
        {'C', 64}, // E-4
        {'V', 65}, // F-4
        {'G', 66}, // F#4
        {'B', 67}, // G-4
        {'H', 68}, // G#4
        {'N', 69}, // A-4
        {'J', 70}, // A#4
        {'M', 71}, // B-4

        {'Q', 72}, // C-5
        {'2', 73}, // C#5
        {'W', 74}, // D-5
        {'3', 75}, // D#5
        {'E', 76}, // E-5
        {'R', 77}, // F-5
        {'5', 78}, // F#5
        {'T', 79}, // G-5
        {'6', 80}, // G#5
        {'Y', 81}, // A-5
        {'7', 82}, // A#5
        {'U', 83}, // B-5

        {'I', 84}, // C-6
        {'9', 85}, // C#6
        {'O', 86}, // D-6
        {'0', 87}, // D#6
        {'P', 88}, // E-6
    };

    bool HandleKeyboardNotes()
    {
        if (currentProperty == 0)
        {
            for (auto p : _charToNoteMap)
            {
                if (ImGui::IsKeyPressed((ImWchar)p.first))
                {
                    _pattern->Notes(currentTrack)[currentRow]._note = p.second;
                    MoveCurrentRowDown(true);
                    return true;
                }
            }
        }
        else if (currentProperty == 1)
        {
        }

        return false;
    }

    bool HandleKeyboardNavigation()
    {
        bool result = false;

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
        {
            MoveCurrentRowUp(ImGui::GetIO().KeyCtrl);

            result = true;
        }

        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
        {
            MoveCurrentRowDown(ImGui::GetIO().KeyCtrl);

            result = true;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow)))
        {
            MoveToNextProperty();

            result = true;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow)))
        {
            MoveToPreviousProperty();

            result = true;
        }
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Tab)))
        {
            ChangeCurrentTrack(ImGui::GetIO().KeyShift);

            result = true;
        }

        return result;
    }

    void MoveCurrentRowUp(
        bool largeStep)
    {
        unsigned int step = largeStep ? 4 : 1;
        if (currentRow >= step)
        {
            currentRow -= step;
        }
        else
        {
            currentRow = _pattern->Length() - 1;
        }
    }

    void MoveCurrentRowDown(
        bool largeStep)
    {
        unsigned int step = largeStep ? skipRowStepSize : 1;

        currentRow += step;

        if (currentRow >= _pattern->Length()) currentRow = 0;
    }

    void ChangeCurrentTrack(
        bool moveLeft)
    {
        if (moveLeft)
        {
            if (currentTrack > 0)
            {
                currentTrack--;
            }
            else
            {
                currentTrack = numTracks - 1;
            }
        }
        else
        {
            currentTrack++;
            if (currentTrack >= numTracks) currentTrack = 0;
        }
    }

    void MoveToPreviousProperty()
    {
        if (currentProperty == 0)
        {
            currentProperty = 4;
            if (currentTrack > 0)
            {
                currentTrack--;
            }
            else
            {
                currentTrack = numTracks - 1;
            }
        }
        else
        {
            currentProperty--;
        }
    }

    void MoveToNextProperty()
    {
        currentProperty++;
        if (currentProperty >= 4)
        {
            currentProperty = 0;
            currentTrack++;
            if (currentTrack >= numTracks) currentTrack = 0;
        }
    }

    ILibraryItem *LibraryTree(
        ILibrary *library)
    {
        ILibraryItem *result = nullptr;

        if (library->GetChildren().empty() && library->GetItems().empty())
        {
            if (ImGui::TreeNodeEx(library->GetName().c_str(), ImGuiTreeNodeFlags_Leaf))
            {
                ImGui::TreePop();
            }
        }
        else
        {
            if (ImGui::TreeNode(library->GetName().c_str()))
            {
                for (auto level : library->GetChildren())
                {
                    auto tmp = LibraryTree(level);
                    if (tmp != nullptr && result == nullptr)
                    {
                        result = tmp;
                    }
                }

                for (auto item : library->GetItems())
                {
                    if (ImGui::TreeNodeEx(item->GetName().c_str(), ImGuiTreeNodeFlags_Leaf))
                    {
                        ImGui::TreePop();
                    }

                    if (ImGui::IsItemClicked())
                    {
                        result = item;
                    }
                }
                ImGui::TreePop();
            }
        }

        return result;
    }

    virtual void Render2d()
    {
        auto selectionColor = ImColor(20, 180, 20, 255);
        auto selectedRowBackgroundColor = ImColor(20, 220, 20, 55);

        //show Main Window
        ImGui::ShowDemoWindow();

        ImGui::Begin(
            "instruments");
        {
            for (unsigned int i = 0; i < NUM_MIXER_TRACKS; i++)
            {
                auto track = _mixer->GetTrack(i);
                ImGui::PushID(i);

                bool v = track->Penabled == 1;
                if (ImGui::Checkbox("##enabled", &v))
                {
                    track->Penabled = v ? 1 : 0;
                }
                if (ImGui::IsItemClicked())
                {
                    currentTrack = i;
                }
                ImGui::SameLine();

                char buf[256] = {0};
                sprintf_s(buf, 256, "%02d : %s", int(i), track->Pname);
                ImGui::Selectable(buf, i == currentTrack);
                if (ImGui::IsItemClicked())
                {
                    currentTrack = i;
                }

                ImGui::PopID();
            }
        }
        ImGui::End();

        ImGui::Begin(
            "Instrument Properties");
        {
            if (currentTrack >= 0 && currentTrack < NUM_MIXER_TRACKS)
            {
                auto track = _mixer->GetTrack(currentTrack);

                bool v = track->Penabled == 1;
                if (ImGui::Checkbox("Enabled", &v))
                {
                    track->Penabled = v ? 1 : 0;
                }

                ImGui::SameLine();
                ImGui::SetNextItemWidth(80);

                int midiChannel = track->Prcvchn;
                if (ImGui::SliderInt(
                        "MIDI Channel",
                        &midiChannel,
                        1, NUM_MIDI_CHANNELS))
                {
                    track->Prcvchn = midiChannel;
                }

                ImGui::InputText(
                    "Name",
                    (char *)(track->Pname),
                    IM_ARRAYSIZE(track->Pname));
                ImGui::InputText(
                    "Author",
                    (char *)(track->info.Pauthor),
                    IM_ARRAYSIZE(track->info.Pauthor));
                ImGui::InputTextMultiline(
                    "Comments",
                    (char *)(track->info.Pcomments),
                    IM_ARRAYSIZE(track->info.Pcomments));
            }
        }
        ImGui::End();

        ImGui::Begin(
            "Instrument Library");
        {
            auto libs = _library->GetTopLevelLibraries();
            std::vector<ILibrary *> slibs(libs.begin(), libs.end());
            std::sort(slibs.begin(), slibs.end(), [](ILibrary *a, ILibrary *b) {
                return a->GetName() < b->GetName();
            });
            for (auto topLevel : slibs)
            {
                auto selection = LibraryTree(topLevel);
                if (selection != nullptr)
                {
                    auto const &track = _mixer->GetTrack(currentTrack);
                    track->Lock();
                    _library->LoadAsInstrument(selection, track);
                    track->Penabled = 1;
                    track->ApplyParameters();
                    track->Unlock();
                }
            }
        }
        ImGui::End();

        ImGui::Begin(
            "TracksContainer",
            nullptr,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            ImGui::PushFont(_monofont);
            auto content = ImGui::GetContentRegionAvail() - ImVec2(rowIndexColumnWidth, 0);
            float tracksScrollx = 0, tracksScrolly = 0, lineHeight = 30;

            auto spaceWidth = ImGui::CalcTextSize(" ");
            auto cellNoteWidth = ImGui::CalcTextSize(emptyCellNote);
            auto cellParameterWidth = ImGui::CalcTextSize(emptyCellParameter);
            auto cellFxWidth = ImGui::CalcTextSize(emptyCellFx);
            auto columnWidth = cellNoteWidth +
                               spaceWidth + cellParameterWidth +
                               spaceWidth + cellParameterWidth +
                               spaceWidth + cellFxWidth;
            auto fullWidth = (columnWidth.x + 15) * numTracks;

            auto contentTop = ImGui::GetCursorPosY();
            ImGui::MoveCursorPos(ImVec2(rowIndexColumnWidth, headerHeight));
            ImGui::BeginChild(
                "container",
                ImVec2(content.x, -(footerHeight + scrollbarHeight)));
            {
                lineHeight = ImGui::GetTextLineHeightWithSpacing();

                ImGui::BeginChild(
                    "tracks",
                    ImVec2(fullWidth, lineHeight * _pattern->Length()));
                {
                    auto tracksPos = ImGui::GetWindowContentRegionMin();
                    auto tracksMax = ImGui::GetWindowContentRegionMax();

                    auto selectionRowMin = ImVec2(tracksPos.x, tracksPos.y + currentRow * lineHeight);
                    auto selectionRowMax = ImVec2(tracksMax.x, tracksPos.y + (currentRow + 1) * lineHeight);

                    auto drawList = ImGui::GetWindowDrawList();

                    for (unsigned int i = 0; i < _pattern->Length(); i += 4)
                    {
                        auto highlightRowMin = ImVec2(tracksPos.x, tracksPos.y + i * lineHeight);
                        auto highlightRowMax = ImVec2(tracksMax.x, tracksPos.y + (i + 1) * lineHeight);

                        drawList->AddRectFilled(
                            ImGui::GetWindowPos() + highlightRowMin,
                            ImGui::GetWindowPos() + highlightRowMax,
                            ImColor(120, 120, 120, 55));
                    }

                    // SELECTED ROW

                    drawList->AddRectFilled(
                        ImGui::GetWindowPos() + selectionRowMin,
                        ImGui::GetWindowPos() + selectionRowMax,
                        selectedRowBackgroundColor);

                    // MAKE ROOM FOR THE HEADERS

                    ImGui::Columns(numTracks);
                    for (unsigned int i = 0; i < numTracks; i++)
                    {
                        ImGui::SetColumnWidth(i, columnWidth.x + 15);
                    }

                    // ALL TRACKS AND CELLS

                    for (unsigned int r = 0; r < _pattern->Length(); r++)
                    {
                        for (unsigned int i = 0; i < numTracks; i++)
                        {
                            auto notes = _pattern->Notes(i);
                            auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                            if (i == currentTrack && r == currentRow)
                            {
                                auto cursorWidth = cellNoteWidth.x;
                                auto min = markerPos + ImVec2(2, 0) - ImVec2(ImGui::GetScrollX(), ImGui::GetScrollY());
                                if (currentProperty > 0)
                                {
                                    min.x += cellNoteWidth.x + spaceWidth.x - 1;
                                    cursorWidth = cellParameterWidth.x;
                                }
                                if (currentProperty > 1)
                                {
                                    min.x += cellParameterWidth.x + spaceWidth.x - 1;
                                    cursorWidth = cellParameterWidth.x;
                                }
                                if (currentProperty > 2)
                                {
                                    min.x += cellParameterWidth.x + spaceWidth.x;
                                    cursorWidth = cellFxWidth.x;
                                }

                                drawList->AddRectFilled(
                                    min - ImVec2(4, 0),
                                    min + ImVec2(cursorWidth, lineHeight),
                                    selectionColor);
                            }

                            char const *cellNote = emptyCellNote;
                            char const *cellParameter1 = emptyCellParameter;
                            char const *cellParameter2 = emptyCellParameter;
                            char const *cellFx = emptyCellFx;
                            if (notes[r]._note != 0)
                            {
                                cellNote = NoteToString(notes[r]._note);
                                if (notes[r]._length != 0)
                                {
                                    cellParameter1 = ValueToString(notes[r]._length);
                                }
                                if (notes[r]._velocity != 0)
                                {
                                    cellParameter2 = ValueToString(notes[r]._velocity);
                                }
                            }
                            ImGui::Text("%s %s %s %s", cellNote, cellParameter1, cellParameter2, cellFx);
                            ImGui::NextColumn();
                        }
                    }
                }
                ImGui::EndChild();

                // Events

                if (HandleKeyboardNotes())
                {
                    ImGui::SetScrollY((currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
                }
                else if (HandleKeyboardNavigation())
                {
                    ImGui::SetScrollY((currentRow - (content.y / lineHeight) / 2 + 1) * lineHeight);
                    ImGui::SetScrollX((currentTrack - (content.x / columnWidth.x) / 2 + 1) * columnWidth.x);
                }

                tracksScrollx = ImGui::GetScrollX();
                tracksScrolly = ImGui::GetScrollY();
            }
            ImGui::EndChild();

            // FOOTERS

            ImGui::MoveCursorPos(ImVec2(rowIndexColumnWidth, 0));
            ImGui::BeginChild(
                "footerscontainer",
                ImVec2(content.x, footerHeight));
            {
                ImGui::SetScrollX(tracksScrollx);

                ImGui::BeginChild(
                    "footers",
                    ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, footerHeight));
                {
                    ImGui::Columns(numTracks);
                    for (unsigned int i = 0; i < numTracks; i++)
                    {
                        ImGui::PushID(i);
                        auto drawList = ImGui::GetWindowDrawList();
                        auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                        if (i == currentTrack)
                        {
                            drawList->AddLine(
                                markerPos + ImVec2(-4, 2),
                                markerPos + ImVec2(-4, 16),
                                selectionColor,
                                2);
                            drawList->AddLine(
                                markerPos + ImVec2(-4, 16),
                                markerPos + ImVec2(10, 16),
                                selectionColor,
                                3);
                            drawList->AddLine(
                                markerPos + ImVec2(columnWidth.x + 3, 2),
                                markerPos + ImVec2(columnWidth.x + 3, 16),
                                selectionColor,
                                2);
                            drawList->AddLine(
                                markerPos + ImVec2(columnWidth.x + 4, 16),
                                markerPos + ImVec2(columnWidth.x - 11, 16),
                                selectionColor,
                                3);
                        }

                        auto w = ImGui::CalcTextSize("footer 00").x / 2.0f;
                        ImGui::SetColumnWidth(i, columnWidth.x + 15);
                        ImGui::MoveCursorPos(ImVec2((ImGui::GetContentRegionAvailWidth() / 2.0f) - w, 0));
                        ImGui::Text("footer %02d", i + 1);
                        ImGui::PopID();
                        ImGui::NextColumn();
                    }
                    ImGui::Columns(1);
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();

            // SCROLLBAR

            ImGui::MoveCursorPos(ImVec2(rowIndexColumnWidth, 0));
            ImGui::BeginChild(
                "scrollbar",
                ImVec2(content.x, scrollbarHeight),
                false,
                ImGuiWindowFlags_AlwaysHorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
            {
                ImGui::SetScrollX(tracksScrollx);

                ImGui::BeginChild(
                    "scrollbarcontent ",
                    ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, scrollbarHeight));
                {}
                ImGui::EndChild();
            }
            ImGui::EndChild();

            // ROWINDICES

            ImGui::SetCursorPosY(contentTop);
            ImGui::MoveCursorPos(ImVec2(0, headerHeight));
            ImGui::BeginChild(
                "rowinedicescontainer",
                ImVec2(rowIndexColumnWidth, -(footerHeight + scrollbarHeight)),
                false,
                ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::SetScrollY(tracksScrolly);

                ImGui::BeginChild(
                    "rowindices",
                    ImVec2(rowIndexColumnWidth, lineHeight * _pattern->Length()));
                {
                    auto drawList = ImGui::GetWindowDrawList();

                    for (unsigned int r = 0; r < _pattern->Length(); r++)
                    {
                        auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();
                        if (r % 4 == 0)
                        {
                            drawList->AddRectFilled(
                                markerPos,
                                markerPos + ImVec2(rowIndexColumnWidth, lineHeight),
                                ImColor(120, 120, 120, 55));
                        }
                        if (r == currentRow)
                        {
                            drawList->AddRectFilled(
                                markerPos,
                                markerPos + ImVec2(rowIndexColumnWidth, lineHeight),
                                selectedRowBackgroundColor);
                        }
                        ImGui::Text("%02d", r);
                    }
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();

            // HEADERS

            ImGui::SetCursorPosY(contentTop);
            ImGui::MoveCursorPos(ImVec2(rowIndexColumnWidth, 0));
            ImGui::BeginChild(
                "headerscontainer",
                ImVec2(content.x, -40));
            {
                ImGui::SetScrollX(tracksScrollx);
                ImGui::BeginChild(
                    "headers",
                    ImVec2(fullWidth + ImGui::GetStyle().ScrollbarSize, headerHeight));
                {
                    ImGui::Columns(numTracks);

                    auto drawList = ImGui::GetWindowDrawList();
                    for (unsigned int i = 0; i < numTracks; i++)
                    {
                        ImGui::PushID(i);
                        auto markerPos = ImGui::GetWindowPos() + ImGui::GetCursorPos();

                        if (i == currentTrack)
                        {
                            drawList->AddLine(
                                markerPos + ImVec2(-4, 0),
                                markerPos + ImVec2(-4, 14),
                                selectionColor,
                                2);
                            drawList->AddLine(
                                markerPos + ImVec2(-4, 0),
                                markerPos + ImVec2(10, 0),
                                selectionColor,
                                3);
                            drawList->AddLine(
                                markerPos + ImVec2(columnWidth.x + 3, 0),
                                markerPos + ImVec2(columnWidth.x + 3, 14),
                                selectionColor,
                                2);
                            drawList->AddLine(
                                markerPos + ImVec2(columnWidth.x + 3, 0),
                                markerPos + ImVec2(columnWidth.x - 11, 0),
                                selectionColor,
                                3);
                        }

                        auto hue = i * 0.05f;
                        drawList->AddRectFilled(
                            markerPos + ImVec2(0, 4),
                            markerPos + ImVec2(columnWidth.x, 8),
                            ImColor::HSV(hue, 0.9f, 0.7f));

                        ImGui::SetColumnWidth(i, columnWidth.x + 15);
                        auto w = ImGui::CalcTextSize("Track 00").x / 2.0f;
                        ImGui::MoveCursorPos(ImVec2((ImGui::GetContentRegionAvailWidth() / 2.0f) - w, 9));
                        ImGui::SetNextWindowSize(ImVec2(columnWidth.x + 15, headerHeight));
                        ImGui::Text("Track %02d", i + 1);
                        bool v = _mixer->GetTrack(i)->Penabled == 1;
                        if (ImGui::Checkbox("##enabled", &v))
                        {
                            _mixer->GetTrack(i)->Penabled = v ? 1 : 0;
                            if (v)
                            {
                                currentTrack = i;
                                currentProperty = 0;
                            }
                        }
                        ImGui::SameLine();
                        ImGui::Button("edit", ImVec2(-1, 0));

                        ImGui::Button("m", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));
                        ImGui::SameLine();
                        ImGui::Button("s", ImVec2(ImGui::GetFrameHeight(), ImGui::GetFrameHeight()));

                        ImGui::PopID();
                        ImGui::NextColumn();
                    }
                    ImGui::Columns(1);
                }
                ImGui::EndChild();
            }
            ImGui::EndChild();

            ImGui::PopFont();
        }
        ImGui::End();
    }

    virtual void Cleanup()
    {
        //ensure that everything has stopped with the mutex wait
        _mixer->Lock();
        _mixer->Unlock();

        Nio::Stop();

        delete _mixer;
        _mixer = nullptr;
    }
};

extern IApplication *CreateApplication();

IApplication *CreateApplication()
{
    return new Application();
}

char const *NoteToString(
    unsigned int note)
{
    switch (note)
    {
        case 127:
            return "G-9";
        case 126:
            return "F#9";
        case 125:
            return "F-9";
        case 124:
            return "E-9";
        case 123:
            return "D#9";
        case 122:
            return "D-9";
        case 121:
            return "C#9";
        case 120:
            return "C-9";
        case 119:
            return "B-8";
        case 118:
            return "A#8";
        case 117:
            return "A-8";
        case 116:
            return "G#8";
        case 115:
            return "G-8";
        case 114:
            return "F#8";
        case 113:
            return "F-8";
        case 112:
            return "E-8";
        case 111:
            return "D#8";
        case 110:
            return "D-8";
        case 109:
            return "C#8";
        case 108:
            return "C-8";
        case 107:
            return "B-7";
        case 106:
            return "A#7";
        case 105:
            return "A-7";
        case 104:
            return "G#7";
        case 103:
            return "G-7";
        case 102:
            return "F#7";
        case 101:
            return "F-7";
        case 100:
            return "E-7";
        case 99:
            return "D#7";
        case 98:
            return "D-7";
        case 97:
            return "C#7";
        case 96:
            return "C-7";
        case 95:
            return "B-6";
        case 94:
            return "A#6";
        case 93:
            return "A-6";
        case 92:
            return "G#6";
        case 91:
            return "G-6";
        case 90:
            return "F#6";
        case 89:
            return "F-6";
        case 88:
            return "E-6";
        case 87:
            return "D#6";
        case 86:
            return "D-6";
        case 85:
            return "C#6";
        case 84:
            return "C-6";
        case 83:
            return "B-5";
        case 82:
            return "A#5";
        case 81:
            return "A-5";
        case 80:
            return "G#5";
        case 79:
            return "G-5";
        case 78:
            return "F#5";
        case 77:
            return "F-5";
        case 76:
            return "E-5";
        case 75:
            return "D#5";
        case 74:
            return "D-5";
        case 73:
            return "C#5";
        case 72:
            return "C-5";
        case 71:
            return "B-4";
        case 70:
            return "A#4";
        case 69:
            return "A-4";
        case 68:
            return "G#4";
        case 67:
            return "G-4";
        case 66:
            return "F#4";
        case 65:
            return "F-4";
        case 64:
            return "E-4";
        case 63:
            return "D#4";
        case 62:
            return "D-4";
        case 61:
            return "C#4";
        case 60:
            return "C-4";
        case 59:
            return "B-3";
        case 58:
            return "A#3";
        case 57:
            return "A-3";
        case 56:
            return "G#3";
        case 55:
            return "G-3";
        case 54:
            return "F#3";
        case 53:
            return "F-3";
        case 52:
            return "E-3";
        case 51:
            return "D#3";
        case 50:
            return "D-3";
        case 49:
            return "C#3";
        case 48:
            return "C-3";
        case 47:
            return "B-2";
        case 46:
            return "A#2";
        case 45:
            return "A-2";
        case 44:
            return "G#2";
        case 43:
            return "G-2";
        case 42:
            return "F#2";
        case 41:
            return "F-2";
        case 40:
            return "E-2";
        case 39:
            return "D#2";
        case 38:
            return "D-2";
        case 37:
            return "C#2";
        case 36:
            return "C-2";
        case 35:
            return "B-1";
        case 34:
            return "A#1";
        case 33:
            return "A-1";
        case 32:
            return "G#1";
        case 31:
            return "G-1";
        case 30:
            return "F#1";
        case 29:
            return "F-1";
        case 28:
            return "E-1";
        case 27:
            return "D#1";
        case 26:
            return "D-1";
        case 25:
            return "C#1";
        case 24:
            return "C-1";
        case 23:
            return "B-0";
        case 22:
            return "A#0";
        case 21:
            return "A-0";
    }
    return "---";
}

char const *ValueToString(
    unsigned int value)
{
    switch (value)
    {
        case 1:
            return "1";
        case 2:
            return "2";
        case 3:
            return "3";
        case 4:
            return "4";
        case 5:
            return "5";
        case 6:
            return "6";
        case 7:
            return "7";
        case 8:
            return "8";
        case 9:
            return "9";
        case 10:
            return "a";
        case 11:
            return "b";
        case 12:
            return "c";
        case 13:
            return "d";
        case 14:
            return "e";
        case 15:
            return "f";
        case 16:
            return "10";
        case 17:
            return "11";
        case 18:
            return "12";
        case 19:
            return "13";
        case 20:
            return "14";
        case 21:
            return "15";
        case 22:
            return "16";
        case 23:
            return "17";
        case 24:
            return "18";
        case 25:
            return "19";
        case 26:
            return "1a";
        case 27:
            return "1b";
        case 28:
            return "1c";
        case 29:
            return "1d";
        case 30:
            return "1e";
        case 31:
            return "1f";
        case 32:
            return "20";
        case 33:
            return "21";
        case 34:
            return "22";
        case 35:
            return "23";
        case 36:
            return "24";
        case 37:
            return "25";
        case 38:
            return "26";
        case 39:
            return "27";
        case 40:
            return "28";
        case 41:
            return "29";
        case 42:
            return "2a";
        case 43:
            return "2b";
        case 44:
            return "2c";
        case 45:
            return "2d";
        case 46:
            return "2e";
        case 47:
            return "2f";
        case 48:
            return "30";
        case 49:
            return "31";
        case 50:
            return "32";
        case 51:
            return "33";
        case 52:
            return "34";
        case 53:
            return "35";
        case 54:
            return "36";
        case 55:
            return "37";
        case 56:
            return "38";
        case 57:
            return "39";
        case 58:
            return "3a";
        case 59:
            return "3b";
        case 60:
            return "3c";
        case 61:
            return "3d";
        case 62:
            return "3e";
        case 63:
            return "3f";
        case 64:
            return "40";
        case 65:
            return "41";
        case 66:
            return "42";
        case 67:
            return "43";
        case 68:
            return "44";
        case 69:
            return "45";
        case 70:
            return "46";
        case 71:
            return "47";
        case 72:
            return "48";
        case 73:
            return "49";
        case 74:
            return "4a";
        case 75:
            return "4b";
        case 76:
            return "4c";
        case 77:
            return "4d";
        case 78:
            return "4e";
        case 79:
            return "4f";
        case 80:
            return "50";
        case 81:
            return "51";
        case 82:
            return "52";
        case 83:
            return "53";
        case 84:
            return "54";
        case 85:
            return "55";
        case 86:
            return "56";
        case 87:
            return "57";
        case 88:
            return "58";
        case 89:
            return "59";
        case 90:
            return "5a";
        case 91:
            return "5b";
        case 92:
            return "5c";
        case 93:
            return "5d";
        case 94:
            return "5e";
        case 95:
            return "5f";
        case 96:
            return "60";
        case 97:
            return "61";
        case 98:
            return "62";
        case 99:
            return "63";
        case 100:
            return "64";
        case 101:
            return "65";
        case 102:
            return "66";
        case 103:
            return "67";
        case 104:
            return "68";
        case 105:
            return "69";
        case 106:
            return "6a";
        case 107:
            return "6b";
        case 108:
            return "6c";
        case 109:
            return "6d";
        case 110:
            return "6e";
        case 111:
            return "6f";
        case 112:
            return "70";
        case 113:
            return "71";
        case 114:
            return "72";
        case 115:
            return "73";
        case 116:
            return "74";
        case 117:
            return "75";
        case 118:
            return "76";
        case 119:
            return "77";
        case 120:
            return "78";
        case 121:
            return "79";
        case 122:
            return "7a";
        case 123:
            return "7b";
        case 124:
            return "7c";
        case 125:
            return "7d";
        case 126:
            return "7e";
        case 127:
            return "7f";
    }

    return "..";
}
