#include "application.h"

#include <algorithm>
#include <iapplication.h>
#include <imgui.h>
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
    OneBar = 1,
    TwoBar = 2,
    FourBar = 4,
    EightBar = 8,
    SixteenBar = 16,
    ThirtyTwoBar = 32,
};

struct PatternNote
{
    float start;
    float length;
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

TrackPattern::TrackPattern() = default;

TrackPattern::~TrackPattern() = default;

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

    static const ImWchar fontaudio_icon_ranges[] = {ICON_MIN_FAD, ICON_MAX_FAD, 0};
    _fadFont = io.Fonts->AddFontFromFileTTF("fonts/fontaudio.ttf", 18.0f, &config, fontaudio_icon_ranges);

    static const ImWchar forkawesome_icon_ranges[] = {ICON_MIN_FK, ICON_MAX_FK, 0};
    _fkFont = io.Fonts->AddFontFromFileTTF("fonts/forkawesome-webfont.ttf", 12.0f, &config, forkawesome_icon_ranges);

    _monofont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\SourceCodePro-Bold.ttf", 14.0f);

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
    _trackPatterns[0][45]._pattern.push_back({0.0f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.15f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.2f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.25f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.4f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.45f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.5f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.65f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.7f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.75f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.9f, 0.02f});
    _trackPatterns[0][45]._pattern.push_back({0.95f, 0.02f});

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
    ImGui::Text("_currentBar = %f", _currentBar);
    ImGui::SliderInt("bpm", &bpm, 50, 250);

    static TrackPattern *selectedTrackPattern = nullptr;
    static unsigned char deleteNote = 0;
    bool openPopupClick = false;
    auto &trackPattern = _trackPatterns[0];
    {
        static int nextPatternNote = 0;
        static int lastNote = 0;
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        for (auto &p : trackPattern)
        {
            ImGui::PushID(p.first);
            lastNote = p.first;
            ImGui::Text("[%d]", p.first);
            ImGui::SameLine();

            ImVec2 canvas_pos = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
            ImVec2 canvas_size = ImVec2(200, 21);            // ImGui::GetContentRegionAvail(); // Resize canvas to what's available
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
                auto a = ImVec2(canvas_pos.x + (pattern.start * canvas_size.x), canvas_pos.y + 10);
                auto b = ImVec2(canvas_pos.x + ((pattern.start + pattern.length) * canvas_size.x), canvas_pos.y + 10);
                draw_list->AddLine(a, b, IM_COL32(255, 255, 0, 255), 18.0f);
            }

            ImGui::SameLine();
            if (ImGui::Button("x"))
            {
                deleteNote = p.first;
            }
            ImGui::PopID();
        }

        if (nextPatternNote < lastNote)
        {
            nextPatternNote = lastNote + 1;
        }
        if (nextPatternNote < 128)
        {
            ImGui::SliderInt("next pattern", &nextPatternNote, lastNote, 128);
            ImGui::SameLine();
            if (ImGui::Button("+"))
            {
                trackPattern.insert(std::make_pair(nextPatternNote, TrackPattern()));
            }
        }

        if (deleteNote != 0)
        {
            trackPattern.erase(deleteNote);
            deleteNote = 0;
            selectedTrackPattern = nullptr;
        }
    }

    if (openPopupClick)
    {
        ImGui::OpenPopup("Save?");
    }
    if (ImGui::BeginPopupModal("Save?") && selectedTrackPattern != nullptr)
    {
        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        static ImVector<ImVec2> points;
        static bool adding_line = false;
        if (ImGui::Button("Clear")) selectedTrackPattern->_pattern.clear();
        ImGui::Text("Left-click and drag to add lines");

        // Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
        // But you can also draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
        // If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
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
                std::cout << start << "->" << len << std::endl;
                selectedTrackPattern->_pattern.push_back({start, len});

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
            auto a = ImVec2(canvas_pos.x + (pattern.start * canvas_size.x), canvas_pos.y + (canvas_size.y / 2));
            auto b = ImVec2(canvas_pos.x + ((pattern.start + pattern.length) * canvas_size.x), canvas_pos.y + (canvas_size.y / 2));
            draw_list->AddLine(a, b, IM_COL32(255, 255, 0, 255), canvas_size.y);
        }
        if (!points.empty())
        {
            draw_list->AddLine(points.front(), mouse_pos_in_canvas, IM_COL32(255, 155, 0, 255), canvas_size.y);
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
    //ensure that everything has stopped with the mutex wait
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
                result.push_back(SimpleNote(note.first, 100, len, channel.first));
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
