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
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/MidiInputManager.h>
#include <zyn.nio/Nio.h>
#include <zyn.serialization/LibraryManager.h>

#include "IconsFontaudio.h"
#include "IconsForkAwesome.h"

Application::Application()
    : _sampleIndex(0),
      _monofont(nullptr),
      _fkFont(nullptr),
      _fadFont(nullptr)
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

    _session._mixer = new Mixer();
    _session._mixer->Init();
    _session._mixer->SetNoteSource(this);

    _session._library = new LibraryManager();

    _session._library->AddLibraryLocation("C:\\Program Files (x86)\\ZynAddSubFX\\banks\\");
    //*
    for (int i = 0; i < MAX_BANK_ROOT_DIRS; i++)
    {
        if (Config::Current().cfg.bankRootDirList[i].size() == 0)
        {
            continue;
        }
        _session._library->AddLibraryLocation(Config::Current().cfg.bankRootDirList[i]);
    }
    /*/
    _session._library->RefreshLibraries();
    //*/

    Nio::preferedSampleRate(SystemSettings::Instance().samplerate);

    if (!Nio::Start(_session._mixer, _session._mixer))
    {
        return false;
    }

    Nio::SelectSink("PA");
    Nio::SelectSource("RT");

    _session._song = new Song();

    _session._song->AddPattern();
    auto pattern = _session._song->GetPattern(0);
    pattern->Rename("Intro");

    for (int i = 0; i < 16; i++)
    {
        pattern->Notes(0)[i * 4]._note = 60 + i;
        pattern->Notes(0)[i * 4]._length = 64;
        pattern->Notes(0)[i * 4]._velocity = 100;
    }

    _playerControlsPanel.SetUp(&_session);
    _patternEditor.SetUp(&_session, _monofont);
    _instruments.SetUp(&_session);
    _effectsEditor.SetUp(&_session);
    _patternsManager.SetUp(&_session);
    _automationEditor.SetUp(&_session);
    _synthEditor.SetUp(&_session);

    return true;
}

void Application::Render2d()
{
    static bool _showStats = false;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowSize(ImVec2(playerControlsPanelWidth, playerControlsPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    _playerControlsPanel.Render2d();

    ImGui::SetNextWindowSize(ImVec2(playerControlsPanelWidth, Height() - playerControlsPanelHeight - effectsPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(0, playerControlsPanelHeight));
    _patternsManager.Render2d();

    ImGui::SetNextWindowSize(ImVec2(instrumentPanelWidth, Height()));
    ImGui::SetNextWindowPos(ImVec2(Width() - instrumentPanelWidth, 0));
    _instruments.Render2d();

    ImGui::SetNextWindowSize(ImVec2(Width() - instrumentPanelWidth, effectsPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(0, Height() - effectsPanelHeight));
    _effectsEditor.Render2d();

    ImGui::SetNextWindowSize(ImVec2(Width() - playerControlsPanelWidth - instrumentPanelWidth, tabbarPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(playerControlsPanelWidth, 0));

    ImGui::Begin("tabbar", nullptr, flags | ImGuiWindowFlags_NoTitleBar);
    {
        char buf[256] = {0};

        sprintf_s(buf, 256, "%s EDIT", ICON_FK_PENCIL);
        if (ImGui::Button(buf, ImVec2(120, 0)))
        {
            _session.selectedTab = SelectableTabs::PatternEditor;
            ImGui::SetWindowFocus(PatternEditor::ID);
        }

        ImGui::SameLine();

        sprintf_s(buf, 256, "%s MIXER", ICON_FK_SLIDERS);
        if (ImGui::Button(buf, ImVec2(120, 0)))
        {
            _session.selectedTab = SelectableTabs::Mixer;
            ImGui::SetWindowFocus("Mixer");
        }

        ImGui::SameLine();

        sprintf_s(buf, 256, "%s SYNTH", ICON_FAD_KEYBOARD);
        if (ImGui::Button(buf, ImVec2(120, 0)))
        {
            _session.selectedTab = SelectableTabs::Synth;
        }

        ImGui::SameLine();

        sprintf_s(buf, 256, "%s AUTOMATION", ICON_FAD_AUTOMATION_3P);
        if (ImGui::Button(buf, ImVec2(120, 0)))
        {
            _session.selectedTab = SelectableTabs::AutomationEditor;
        }

        ImGui::SameLine();

        sprintf_s(buf, 256, "%s STATS", ICON_FAD_AUTOMATION_3P);
        if (ImGui::Button(buf, ImVec2(120, 0)))
        {
            _showStats = true;
            ImGui::SetWindowFocus("Stats");
        }
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(
        Width() - playerControlsPanelWidth - instrumentPanelWidth,
        Height() - effectsPanelHeight - tabbarPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(playerControlsPanelWidth, tabbarPanelHeight));

    switch (_session.selectedTab)
    {
        case SelectableTabs::PatternEditor:
        {
            _patternEditor.Render2d();
            break;
        }
        case SelectableTabs::Mixer:
        {
            ImGui::ShowDemoWindow();
            break;
        }
        case SelectableTabs::Synth:
        {
            _synthEditor.Render2d();
            break;
        }
        case SelectableTabs::AutomationEditor:
        {
            _automationEditor.Render2d();
            break;
        }
        default:
        {
            //show Main Window
            ImGui::ShowDemoWindow();
            break;
        }
    }
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space)))
    {
        _session.TogglePlaying();
    }

    ImGui::Begin("Stats", &_showStats);
    {
        auto activeNoteCount = 0;
        for (unsigned int i = 0; i < _session._mixer->GetTrackCount(); i++)
        {
            Track *track = _session._mixer->GetTrack(i);
            if (track->Penabled == 0)
            {
                continue;
            }
            activeNoteCount += track->GetActiveNotes();
        }

        ImGui::Text("Active Notes: %d", activeNoteCount);
    }
    ImGui::End();
}

void Application::Cleanup()
{
    //ensure that everything has stopped with the mutex wait
    _session._mixer->Lock();
    _session._mixer->Unlock();

    Nio::Stop();

    delete _session._mixer;
    _session._mixer = nullptr;
}

std::vector<SimpleNote> Application::GetNotes(
    unsigned int frameCount,
    unsigned int sampleRate)
{
    PostRedraw();

    std::vector<SimpleNote> result;
    if (_session._playState == PlayStates::Stopped)
    {
        return result;
    }

    if (_session._playState == PlayStates::StartPlaying)
    {
        _session._playState = PlayStates::Playing;
        auto notes = GetCurrentStepNotes();
        result.insert(result.end(), notes.begin(), notes.end());
    }

    _sampleIndex += frameCount;

    auto samplesPerBeat = (unsigned int)((sampleRate * 60) / _session._bpm);
    auto samplesPerStep = samplesPerBeat / 4;
    if (_sampleIndex > samplesPerStep)
    {
        _sampleIndex -= samplesPerStep;
        NextStep();
        auto notes = GetCurrentStepNotes();
        result.insert(result.end(), notes.begin(), notes.end());
    }

    return result;
}

void Application::NextStep()
{
    auto song = _session._song;
    auto pattern = song->GetPattern(song->currentPattern);

    if (pattern == nullptr)
    {
        return;
    }

    _session.currentRow++;
    if (_session.currentRow >= pattern->Length())
    {
        song->currentPattern++;
        _session.currentRow = 0;

        if (song->currentPattern >= song->GetPatternCount())
        {
            song->currentPattern = 0;
        }
    }
    _patternEditor.keepRowInFocus = true;
}

std::vector<SimpleNote> Application::GetCurrentStepNotes()
{
    auto pattern = _session._song->GetPattern(_session._song->currentPattern);

    if (pattern == nullptr)
    {
        return std::vector<SimpleNote>();
    }

    std::vector<SimpleNote> result;

    for (int t = 0; t < _session._mixer->GetTrackCount(); t++)
    {
        auto notes = pattern->Notes(t);
        if (notes[_session.currentRow]._note != 0)
        {
            SimpleNote n(
                notes[_session.currentRow]._note,
                notes[_session.currentRow]._velocity,
                notes[_session.currentRow]._length,
                t);
            result.push_back(n);
        }
    }

    return result;
}

extern IApplication *CreateApplication();

IApplication *CreateApplication()
{
    return new Application();
}
