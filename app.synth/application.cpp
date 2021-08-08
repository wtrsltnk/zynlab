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
};

class TrackPattern
{
public:
    TrackPattern();
    virtual ~TrackPattern();

private:
    unsigned char _note;
    BarTypes _numberOfbars = BarTypes::FourBar;

    // a collection of start+end time tuples for when to hit a note.
    // All time values must be between 0.0f and 1.0f which is relative
    // to the size of the total pattern, determined by the number of bars.
    std::vector<std::tuple<float, float>> _pattern;

    // a collection of tuples containing the start and end of a region where
    // this pattern is active. These numbers represent the bar number a region
    // starts and ends.
    std::vector<std::tuple<long, long>> _activeRegions;
};

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

    return true;
}

void Application::Render2d()
{
    _synthEditor.Render2d(ImGuiWindowFlags_NoCollapse);

    ImGui::ShowDemoWindow();
    ImGui::Begin("PatternEditor");

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

std::vector<SimpleNote> Application::GetNotes(
    unsigned int frameCount,
    unsigned int sampleRate)
{
    PostRedraw();

    std::vector<SimpleNote> result;

    return result;
}

extern IApplication *CreateApplication();

IApplication *CreateApplication()
{
    return new Application();
}
