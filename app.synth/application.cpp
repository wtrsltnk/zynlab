#include "application.h"

#include <algorithm>
#include <iapplication.h>
#include <imgui.h>
#include <imgui_helpers.h>
#include <imgui_plot.h>
#include <iostream>
#include <map>
#include <zyn.common/Config.h>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/IMidiEventHandler.h>
#include <zyn.common/globals.h>
#include <zyn.nio/MidiInputManager.h>
#include <zyn.nio/Nio.h>
#include <zyn.serialization/LibraryManager.h>

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"

enum BarTypes
{
    FourBar = 4,
    EightBar = 8,
    SixteenBar = 16,
    ThirtyTwoBar = 32,
    SixtyFourBar = 64,
    OneHunderedTwentyEightBar = 128,
};

struct PatternNote
{
    PatternNote() = default;
    PatternNote(
        float s,
        float l,
        char no,
        unsigned char v)
    {
        start = s;
        length = l;
        noteOffset = no;
        velocity = v;
    }

    float start;
    float length;
    char noteOffset = 0; // +/- 12 notes
    unsigned char velocity = 100;
};

class TrackPattern
{
public:
    TrackPattern();
    virtual ~TrackPattern();

    BarTypes _numberOfbars = BarTypes::FourBar;

    // a collection of start+length time structs for when to hit a note.
    // All time values must be between 0.0f and 1.0f which is relative
    // to the size of the total pattern, determined by the number of bars.
    std::vector<PatternNote> _pattern;

    // a collection of tuples containing the start and end of a region where
    // this pattern is active. These numbers represent the bar number a region
    // starts and ends.
    std::vector<std::tuple<long, long>> _activeRegions;
};

std::map<int, std::map<unsigned char, TrackPattern>> _trackPatterns;
float _currentBar = 0;
int bpm = 138;
bool playing = false;

char const *NoteToString(
    unsigned int note);

TrackPattern::TrackPattern() = default;

TrackPattern::~TrackPattern() = default;

void EnsurePatterns(
    TrackPattern *trackPattern)
{
    if (trackPattern->_numberOfbars != trackPattern->_pattern.size())
    {
        trackPattern->_pattern.resize(trackPattern->_numberOfbars);
    }

    auto relativeBarLength = 1.0f / float(trackPattern->_numberOfbars);
    for (int i = 0; i < trackPattern->_numberOfbars; i++)
    {
        trackPattern->_pattern[i].start = relativeBarLength * i;
        trackPattern->_pattern[i].length = 0.5f / float(trackPattern->_numberOfbars);
    }
}

Application::Application()
    : _mixer(std::make_unique<Mixer>())
{}

bool Application::Setup()
{
    auto &style = ImGui::GetStyle();

    ImGui::StyleColorsDark();

    style.TabRounding = 4;
    style.GrabRounding = 0;
    style.ChildRounding = 0;
    style.PopupRounding = 0;
    style.ScrollbarRounding = 0;
    style.WindowRounding = 0;
    style.Colors[ImGuiCol_Border] = ImVec4(30 / 255.0f, 30 / 255.0f, 30 / 255.0f, 240 / 255.0f);
    style.Colors[ImGuiCol_TitleBg] = ImVec4(21 / 255.0f, 37 / 255.0f, 61 / 255.0f, 240 / 255.0f);
    style.FramePadding = ImVec2(8, 8);
    style.ItemSpacing = ImVec2(8, 8);
    style.ItemInnerSpacing = ImVec2(4, 4);

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

    ImFontConfig config;
    config.MergeMode = true;
    config.GlyphMinAdvanceX = 13.0f;
    config.GlyphOffset = ImVec2(0, 3.0f);

    static const ImWchar fontaudio_icon_ranges[] = {ICON_MIN_FAD, ICON_MAX_FAD, 0};
    _fadFont = io.Fonts->AddFontFromFileTTF("fonts/fontaudio.ttf", 18.0f, &config, fontaudio_icon_ranges);

    config.GlyphOffset = ImVec2(0.0f, 0.0f);
    static const ImWchar forkawesome_icon_ranges[] = {ICON_MIN_FK, ICON_MAX_FK, 0};
    _fkFont = io.Fonts->AddFontFromFileTTF("fonts/forkawesome-webfont.ttf", 12.0f, &config, forkawesome_icon_ranges);

    _monofont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\consola.ttf", 14.0f);

    io.Fonts->Build();

    Config::init();

    SystemSettings::Instance().samplerate = Config::Current().cfg.SampleRate;
    SystemSettings::Instance().buffersize = Config::Current().cfg.SoundBufferSize;
    SystemSettings::Instance().oscilsize = Config::Current().cfg.OscilSize;
    SystemSettings::Instance().alias();

    _mixer->Init();
    _mixer->SetNoteSource(this);

    _library = std::make_unique<LibraryManager>();

    _library->AddLibraryLocation("C:\\Program Files (x86)\\zyn-fusion-windows-64bit-3.0.3-demo\\banks\\");
    //*
    for (int i = 0; i < MAX_BANK_ROOT_DIRS; i++)
    {
        if (Config::Current().cfg.bankRootDirList[i].size() == 0)
        {
            continue;
        }
        _library->AddLibraryLocation(Config::Current().cfg.bankRootDirList[i]);
    }
    _library->RefreshLibraries();
    //*/

    Nio::preferedSampleRate(SystemSettings::Instance().samplerate);

    if (!Nio::Start(_mixer.get(), _mixer.get()))
    {
        return false;
    }

    Nio::SelectSink("PA");
    Nio::SelectSource("RT");

    _synthEditor.SetUp(_mixer.get(), _library.get());

    _trackPatterns.insert(std::make_pair(0, std::map<unsigned char, TrackPattern>()));
    _trackPatterns[0].insert(std::make_pair(45, TrackPattern()));
    _trackPatterns[0][45]._pattern.emplace_back(0.0f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.15f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.2f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.25f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.4f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.45f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.5f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.65f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.7f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.75f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.9f, 0.02f, 0, 100);
    _trackPatterns[0][45]._pattern.emplace_back(0.95f, 0.02f, 0, 100);

    return true;
}

ImVector<ImVec2> ConvertToPoints(
    const TrackPattern &p,
    int w)
{
    ImVector<ImVec2> points;

    for (auto &t : p._pattern)
    {
        points.push_back(ImVec2(t.start * w, 10));
        points.push_back(ImVec2((t.start + t.length) * w, 10));
    }

    return points;
}

void Application::Render2d()
{
    _synthEditor.Render2d(ImGuiWindowFlags_NoCollapse);

    ImGui::ShowDemoWindow();

    ImGui::Begin("PatternEditor");

    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();

        if (!playing)
        {
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            draw_list->AddRectFilled(
                ImVec2(canvas_pos.x, canvas_pos.y),
                ImVec2(canvas_pos.x + 34, canvas_pos.x + 34),
                ImGui::GetColorU32(ImGuiCol_NavWindowingHighlight));
        }

        if (ImGui::Button(ICON_FAD_STOP))
        {
            playing = false;
        }

        ImGui::SameLine();

        if (playing)
        {
            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
            draw_list->AddRectFilled(
                ImVec2(canvas_pos.x, canvas_pos.y),
                ImVec2(canvas_pos.x + 34, canvas_pos.x + 34),
                ImGui::GetColorU32(ImGuiCol_NavWindowingHighlight));
        }

        if (ImGui::Button(ICON_FAD_PLAY))
        {
            playing = true;
        }
    }

    ImGui::SetNextItemWidth(160.0f);
    ImGui::SliderInt("bpm", &bpm, 50, 250);

    static TrackPattern *selectedTrackPattern = nullptr;
    bool openPopupClick = false;
    for (unsigned int t = 0; t < _mixer->GetTrackCount(); t++)
    {
        if (!_mixer->GetTrack(t)->Penabled)
        {
            continue;
        }

        static int nextPatternNote = 0;

        if (_trackPatterns.find(t) == _trackPatterns.end())
        {
            _trackPatterns.insert(std::make_pair(t, std::map<unsigned char, TrackPattern>()));
        }

        ImGui::PushID(t);
        static unsigned char deleteNote = 0;
        auto &trackPattern = _trackPatterns[t];
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        for (auto &p : trackPattern)
        {
            ImGui::PushID(p.first);
            ImGui::BeginGroup();

            ImGui::Button(NoteToString(p.first));
            ImGui::SameLine(60.0f);
            ImGui::AlignTextToFramePadding();
            ImGui::Text("%d", p.second._numberOfbars);
            ImGui::SameLine(90.0f);

            ImVec2 canvas_pos = ImGui::GetCursorScreenPos();                                                       // ImDrawList API uses screen coordinates!
            ImVec2 canvas_size = ImVec2(200, ImGui::GetTextLineHeight() + (ImGui::GetStyle().FramePadding.y * 2)); // ImGui::GetContentRegionAvail(); // Resize canvas to what's available
            if (ImGui::InvisibleButton("Edit", canvas_size))
            {
                selectedTrackPattern = &(p.second);
                openPopupClick = true;
            }
            if (ImGui::IsItemHovered())
            {
                draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImGui::GetColorU32(ImGuiCol_ButtonHovered));
            }
            else if (ImGui::IsItemActivated())
            {
                draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImGui::GetColorU32(ImGuiCol_ButtonActive));
            }
            else
            {
                draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImGui::GetColorU32(ImGuiCol_Button));
            }

            for (auto &pattern : p.second._pattern)
            {
                draw_list->AddLine(
                    ImVec2(canvas_pos.x + (pattern.start * canvas_size.x), canvas_pos.y + (canvas_size.y / 2)),
                    ImVec2(canvas_pos.x + ((pattern.start + pattern.length) * canvas_size.x), canvas_pos.y + (canvas_size.y / 2)),
                    IM_COL32(0, 255, 255, 55),
                    canvas_size.y - 1);
            }

            ImGui::SameLine();
            if (ImGui::Button("x"))
            {
                deleteNote = p.first;
            }
            ImGui::EndGroup();
            ImGui::PopID();
        }

        ImGui::SliderInt("next pattern", &nextPatternNote, 21, 127);
        ImGui::SameLine();
        if (ImGui::Button("+"))
        {
            trackPattern.insert(std::make_pair(nextPatternNote, TrackPattern()));
        }
        if (deleteNote != 0)
        {
            trackPattern.erase(deleteNote);
            deleteNote = 0;
            selectedTrackPattern = nullptr;
        }
        ImGui::PopID();
    }

    if (openPopupClick)
    {
        ImGui::OpenPopup("UpdatePattern");
    }

    if (ImGui::BeginPopupModal("UpdatePattern") && selectedTrackPattern != nullptr)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        static ImVector<ImVec2> points;
        static bool adding_line = false;
        if (ImGui::Button("Clear")) selectedTrackPattern->_pattern.clear();

        int item_current_2 = (static_cast<int>(selectedTrackPattern->_numberOfbars) / 4) - 1;
        if (ImGui::Combo("Number of bars", &item_current_2, "4 \0 8 \0 16 \0 32 \0 64 \0 128 \0\0"))
        {
            selectedTrackPattern->_numberOfbars = static_cast<BarTypes>((1 + item_current_2) * 4);
            EnsurePatterns(selectedTrackPattern);
        }

        for (auto &p : selectedTrackPattern->_pattern)
        {
            ImGui::PushID((void *)&p);
            ImGui::BeginGroup();
            int v = p.noteOffset;
            if (ImGui::VSliderInt("##noteOffset", ImVec2(18, 160), &v, -12, 12))
            {
                p.noteOffset = v;
            }
            KnobUchar("##velocity", &(p.velocity), 0, 127, ImVec2(20, 20), "Velocity");
            ImGui::EndGroup();
            ImGui::SameLine();
            ImGui::PopID();
        }
        ImGui::Text("track");

        ImVec2 canvas_pos = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
        ImVec2 canvas_size = ImVec2(500, 50);            // ImGui::GetContentRegionAvail(); // Resize canvas to what's available

        draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(50, 50, 50, 255), IM_COL32(50, 50, 60, 255), IM_COL32(60, 60, 70, 255), IM_COL32(50, 50, 60, 255));
        draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(255, 255, 255, 255));

        ImGui::InvisibleButton("canvas", canvas_size);
        ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
        if (adding_line)
        {
            if (!ImGui::IsMouseDown(0))
            {
                auto start = points.back().x / canvas_size.x;
                auto len = (mouse_pos_in_canvas.x - points.back().x) / canvas_size.x;

                selectedTrackPattern->_pattern.push_back({start, len, 0, 100});

                adding_line = false;
                points.clear();
            }
        }
        if (ImGui::IsItemHovered())
        {
            if (!adding_line && ImGui::IsMouseClicked(0))
            {
                points.push_back(mouse_pos_in_canvas);
                adding_line = true;
            }
            if (ImGui::IsMouseClicked(1) && !points.empty())
            {
                adding_line = false;
            }
        }

        draw_list->PushClipRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), true); // clip lines within the canvas (if we resize it, etc.)
        for (auto &pattern : selectedTrackPattern->_pattern)
        {
            draw_list->AddLine(
                ImVec2(canvas_pos.x + (pattern.start * canvas_size.x), canvas_pos.y + (canvas_size.y / 2)),
                ImVec2(canvas_pos.x + ((pattern.start + pattern.length) * canvas_size.x), canvas_pos.y + (canvas_size.y / 2)),
                IM_COL32(0, 255, 255, 55),
                canvas_size.y);
        }

        auto step = int(canvas_size.x) / static_cast<int>(selectedTrackPattern->_numberOfbars);
        for (int i = 0; i < int(canvas_size.x); i += step)
        {
            draw_list->AddLine(
                ImVec2(canvas_pos.x + i, canvas_pos.y),
                ImVec2(canvas_pos.x + i, canvas_pos.y + canvas_size.y),
                IM_COL32(255, 55, 0, 255),
                1.0f);
        }

        draw_list->PopClipRect();
        if (ImGui::Button("Close", ImVec2(80, 0)))
        {
            selectedTrackPattern = nullptr;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    ImGui::End();

    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
    {
        //        _session.TogglePlaying();
    }
}

void Application::Cleanup()
{
    // ensure that everything has stopped with the mutex wait
    _mixer->Lock();
    _mixer->Unlock();

    Nio::Stop();
}

float BarsPerSecond() { return (bpm / 60.0f); }

float ProgressInBars(
    unsigned int frameCount,
    unsigned int sampleRate)
{
    auto barsPerSecond = BarsPerSecond();
    auto progressInSeconds = (1.0f / sampleRate) * frameCount;

    auto progressInBars = barsPerSecond * progressInSeconds;

    return progressInBars;
}

std::vector<SimpleNote> Application::GetNotes(
    unsigned int frameCount,
    unsigned int sampleRate)
{
    if (!playing)
    {
        return std::vector<SimpleNote>();
    }

    PostRedraw();

    auto progressInBars = ProgressInBars(frameCount, sampleRate);

    std::vector<SimpleNote> result;

    for (auto &channel : _trackPatterns)
    {
        for (auto &note : channel.second)
        {
            // todo: quickly exit this iteration when not
            // in an active region from note.second._activeRegions

            auto floorCurrentBar = int(std::floor(_currentBar));
            auto pastBars = (floorCurrentBar / note.second._numberOfbars) * note.second._numberOfbars;
            auto offset = _currentBar - pastBars;
            for (auto &patternEvent : note.second._pattern)
            {
                auto start = patternEvent.start * note.second._numberOfbars;
                // check if the pattern event start is within this block of bars (of note.second._numberOfbars size), or the next
                if ((start < offset || start > (offset + progressInBars)) && ((start + note.second._numberOfbars) < offset || (start + note.second._numberOfbars) > (offset + progressInBars)))
                {
                    continue;
                }

                // length of the note is multiplied by number of number of bars to make relative to a single bar
                // and then by barsPerSecond() to convert it seconds
                auto len = float(patternEvent.length * BarsPerSecond());
                result.emplace_back(note.first + patternEvent.noteOffset, patternEvent.velocity, len, channel.first);
            }
        }
    }

    _currentBar += progressInBars;

    return result;
}

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
