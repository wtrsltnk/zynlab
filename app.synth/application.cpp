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

    // a collection of start+end time tuples for when to hit a note.
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
unsigned int bpm = 120;

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
    _trackPatterns[0].insert(std::make_pair(65, TrackPattern()));
    _trackPatterns[0][65]._pattern.push_back({0.0f, 0.1f});
    _trackPatterns[0][65]._pattern.push_back({0.25f, 0.1f});
    _trackPatterns[0][65]._pattern.push_back({0.5f, 0.1f});
    _trackPatterns[0][65]._pattern.push_back({0.75f, 0.1f});

    return true;
}

void Application::Render2d()
{
    _synthEditor.Render2d(ImGuiWindowFlags_NoCollapse);

    ImGui::ShowDemoWindow();
    ImGui::Begin("PatternEditor");
    ImGui::Text("_currentBar = %f", _currentBar);

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

float ProgressInBars(
    unsigned int frameCount,
    unsigned int sampleRate)
{
    auto barsPerSecond = 1.0f / (bpm / (60.0f * 4.0f));
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

                std::cout << offset << std::endl;
                std::cout << note.first << " - " << 100 << " - " << float(patternEvent.length * note.second._numberOfbars) << " - " << channel.first << std::endl;
                result.push_back(SimpleNote(note.first, 100, float(patternEvent.length * note.second._numberOfbars), channel.first));
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
