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
#include "application.h"

Application::Application()
    : _mixer(nullptr),
      _library(nullptr),
      _sampleIndex(0),
      _playState(PlayStates::Stopped),
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
    _fadFont = io.Fonts->AddFontFromFileTTF("fonts/fontaudio.ttf", 13.0f, &config, fontaudio_icon_ranges);

    static const ImWchar forkawesome_icon_ranges[] = {ICON_MIN_FK, ICON_MAX_FK, 0};
    _fkFont = io.Fonts->AddFontFromFileTTF("fonts/forkawesome-webfont.ttf", 12.0f, &config, forkawesome_icon_ranges);

    _monofont = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\SourceCodePro-Bold.ttf", 14.0f);

    io.Fonts->Build();
    Config::init();

    SystemSettings::Instance().samplerate = Config::Current().cfg.SampleRate;
    SystemSettings::Instance().buffersize = Config::Current().cfg.SoundBufferSize;
    SystemSettings::Instance().oscilsize = Config::Current().cfg.OscilSize;
    SystemSettings::Instance().alias();

    _mixer = new Mixer();

    _mixer->Init();

    _mixer->SetNoteSource(this);

    _library = new LibraryManager();

    for (int i = 0; i < MAX_BANK_ROOT_DIRS; i++)
    {
        if (Config::Current().cfg.bankRootDirList[i].size() == 0)
        {
            continue;
        }
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

    _patternEditor.SetUp(&_session, _mixer, _monofont);
    _instruments.SetUp(&_session, _mixer, _library);
    _effectsEditor.SetUp(&_session, _mixer);

    return true;
}

void Application::Render2d()
{
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

    ImGui::SetNextWindowSize(ImVec2(playerControlsPanelWidth, playerControlsPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::Begin(
        "PlayerControls",
        nullptr,
        flags | ImGuiWindowFlags_NoTitleBar);
    {
        bool playing = _playState != PlayStates::Stopped;
        if (!playing)
        {
            if (ImGui::Button(ICON_FAD_PLAY, ImVec2(0, 0)))
            {
                StartPlaying();
            }
        }
        else
        {
            if (ImGui::Button(ICON_FAD_PAUSE, ImVec2(0, 0)))
            {
                StopPlaying();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FAD_STOP, ImVec2(0, 0)))
        {
            StopPlaying();
            _session.currentRow = 0;
            if (_patternEditor.IsRecording())
            {
                _patternEditor.ToggleRecording();
            }
        }
        ImGui::SameLine();

        bool isRecording = _patternEditor.IsRecording();
        if (isRecording)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255, 0, 0, 155));
        }
        if (ImGui::Button(ICON_FAD_RECORD, ImVec2(0, 0)))
        {
            ImGui::SetWindowFocus("PatternEditor");
            _patternEditor.ToggleRecording();
        }
        if (isRecording)
        {
            ImGui::PopStyleColor();
        }
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(playerControlsPanelWidth, Height() - playerControlsPanelHeight - effectsPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(0, playerControlsPanelHeight));
    ImGui::Begin(
        "patterns",
        nullptr,
        flags);
    {
        ImGui::BeginChild("PatternsContainer");
        {
            if (ImGui::Button(ICON_FK_PLUS, ImVec2(0, 0)))
            {
                _patternEditor.CurrentSong()->AddPattern();
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_MINUS, ImVec2(0, 0)))
            {
                _patternEditor.CurrentSong()->RemovePattern(_patternEditor.CurrentSong()->currentPattern);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_ARROW_UP, ImVec2(0, 0)))
            {
                _patternEditor.CurrentSong()->MovePattern(_patternEditor.CurrentSong()->currentPattern, -1);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_ARROW_DOWN, ImVec2(0, 0)))
            {
                _patternEditor.CurrentSong()->MovePattern(_patternEditor.CurrentSong()->currentPattern, 1);
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_FILE_O, ImVec2(0, 0)))
            {
                _patternEditor.CurrentSong()->DuplicatePattern(_patternEditor.CurrentSong()->currentPattern);
            }

            ImGui::BeginChild("patterns", ImVec2(0, -100));
            {
                for (unsigned int i = 0; i < _patternEditor.CurrentSong()->GetPatternCount(); i++)
                {
                    ImGui::PushID(i);

                    auto pattern = _patternEditor.CurrentSong()->GetPattern(i);
                    char buf[256] = {0};
                    sprintf_s(buf, 256, "%02d : %s", int(i), pattern->Name().c_str());
                    ImGui::Selectable(buf, i == _patternEditor.CurrentSong()->currentPattern);
                    if (ImGui::IsItemClicked())
                    {
                        _patternEditor.CurrentSong()->currentPattern = i;
                    }

                    ImGui::PopID();
                }
            }
            ImGui::EndChild();

            ImGui::BeginChild("selectedpattern");
            {
                ImGui::Text("Name");
                auto name = _patternEditor.CurrentSong()->GetPattern(_patternEditor.CurrentSong()->currentPattern)->Name();
                char text[128] = {0};
                strcpy_s(text, 128, name.c_str());
                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                ImGui::InputText("##name", text, 128, ImGuiInputTextFlags_EnterReturnsTrue);

                ImGui::Text("Length");
                int len = _patternEditor.CurrentSong()->GetPattern(_patternEditor.CurrentSong()->currentPattern)->Length();
                ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth());
                if (ImGui::InputInt("##length", &len, 4))
                {
                    _patternEditor.CurrentSong()->GetPattern(_patternEditor.CurrentSong()->currentPattern)->Resize(len);
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(instrumentPanelWidth, Height()));
    ImGui::SetNextWindowPos(ImVec2(Width() - instrumentPanelWidth, 0));
    _instruments.Render2d();

    ImGui::SetNextWindowSize(ImVec2(Width() - instrumentPanelWidth, effectsPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(0, Height() - effectsPanelHeight));
    _effectsEditor.Render2d();

    ImGui::SetNextWindowSize(ImVec2(Width() - playerControlsPanelWidth - instrumentPanelWidth, tabbarPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(playerControlsPanelWidth, 0));

    static int selectedTab = 0;
    ImGui::Begin("tabbar", nullptr, flags | ImGuiWindowFlags_NoTitleBar);
    {
        char buf[256] = {0};

        sprintf_s(buf, 256, "%s Edit", ICON_FK_PENCIL);
        if (ImGui::Button(buf))
        {
            selectedTab = 0;
            ImGui::SetWindowFocus("PatternEditor");
        }

        ImGui::SameLine();

        sprintf_s(buf, 256, "%s Synth", ICON_FAD_KEYBOARD);
        if (ImGui::Button(buf))
        {
            selectedTab = 1;
        }

        ImGui::SameLine();

        sprintf_s(buf, 256, "%s Automation", ICON_FAD_AUTOMATION_3P);
        if (ImGui::Button(buf))
        {
            selectedTab = 2;
        }
    }
    ImGui::End();

    ImGui::SetNextWindowSize(ImVec2(
        Width() - playerControlsPanelWidth - instrumentPanelWidth,
        Height() - effectsPanelHeight - tabbarPanelHeight));
    ImGui::SetNextWindowPos(ImVec2(playerControlsPanelWidth, tabbarPanelHeight));

    switch (selectedTab)
    {
        case 0:
        {
            _patternEditor.Render2d();
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
        TogglePlaying();
    }
}

void Application::Cleanup()
{
    //ensure that everything has stopped with the mutex wait
    _mixer->Lock();
    _mixer->Unlock();

    Nio::Stop();

    delete _mixer;
    _mixer = nullptr;
}

void Application::StopPlaying()
{
    _playState = PlayStates::Stopped;
    for (int t = 0; t < NUM_MIXER_TRACKS; t++)
    {
        _mixer->GetTrack(t)
            ->RelaseAllKeys();
    }
    _patternEditor.keepRowInFocus = true;
}

void Application::StartPlaying()
{
    _playState = PlayStates::StartPlaying;
}

void Application::TogglePlaying()
{
    if (_playState == PlayStates::Stopped)
    {
        StartPlaying();
    }
    else
    {
        StopPlaying();
    }
}

std::vector<SimpleNote> Application::GetNotes(
    unsigned int frameCount,
    unsigned int sampleRate)
{
    PostRedraw();

    std::vector<SimpleNote> result;
    if (_playState == PlayStates::Stopped)
    {
        return result;
    }

    if (_playState == PlayStates::StartPlaying)
    {
        _playState = PlayStates::Playing;
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
    auto song = _patternEditor.CurrentSong();
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
    auto pattern = _patternEditor.CurrentSong()->GetPattern(_patternEditor.CurrentSong()->currentPattern);

    if (pattern == nullptr)
    {
        return std::vector<SimpleNote>();
    }

    std::vector<SimpleNote> result;

    for (int t = 0; t < NUM_MIXER_TRACKS; t++)
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
