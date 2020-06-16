#include <algorithm>
#include <application.h>
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
#include "patterneditor.h"

enum class PlayStates
{
    Stopped,
    StartPlaying,
    Playing,
};

class Application :
    public IApplication,
    public INoteSource
{
    Mixer *_mixer;
    ILibraryManager *_library;

    PatternEditor _patternEditor;
    unsigned int _bpm;
    unsigned int _sampleIndex;
    PlayStates _playState;

    ImFont *_monofont;

public:
    Application()
        : _mixer(nullptr),
          _library(nullptr),
          _bpm(138),
          _sampleIndex(0),
          _playState(PlayStates::Stopped),
          _monofont(nullptr)
    {}

    void StopPlaying()
    {
        _playState = PlayStates::Stopped;
        for (int t = 0; t < NUM_MIXER_TRACKS; t++)
        {
            _mixer->GetTrack(t)
                ->RelaseAllKeys();
        }
        _patternEditor.keepRowInFocus = true;
    }

    void StartPlaying()
    {
        _playState = PlayStates::StartPlaying;
    }

    virtual std::vector<SimpleNote> GetNotes(
        unsigned int frameCount,
        unsigned int sampleRate)
    {
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

        auto samplesPerBeat = (unsigned int)((sampleRate * 60) / _bpm);
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

    void NextStep()
    {
        auto song = _patternEditor.CurrentSong();
        auto pattern = song->GetPattern(song->currentPattern);

        if (pattern == nullptr)
        {
            return;
        }

        _patternEditor.currentRow++;
        if (_patternEditor.currentRow >= pattern->Length())
        {
            song->currentPattern++;
            _patternEditor.currentRow = 0;

            if (song->currentPattern >= song->GetPatternCount())
            {
                song->currentPattern = 0;
            }
        }
        _patternEditor.keepRowInFocus = true;
    }

    std::vector<SimpleNote> GetCurrentStepNotes()
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
            if (notes[_patternEditor.currentRow]._note != 0)
            {
                SimpleNote n(
                    notes[_patternEditor.currentRow]._note,
                    notes[_patternEditor.currentRow]._velocity,
                    notes[_patternEditor.currentRow]._length,
                    t);
                result.push_back(n);
            }
        }

        return result;
    }

    virtual bool Setup()
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

            ImFontConfig config;
            config.MergeMode = true;
            config.GlyphMinAdvanceX = 13.0f;
            static const ImWchar fontaudio_icon_ranges[] = {ICON_MIN_FAD, ICON_MAX_FAD, 0};
            io.Fonts->AddFontFromFileTTF("fonts/fontaudio.ttf", 13.0f, &config, fontaudio_icon_ranges);
            static const ImWchar forkawesome_icon_ranges[] = {ICON_MIN_FK, ICON_MAX_FK, 0};
            io.Fonts->AddFontFromFileTTF("fonts/forkawesome-webfont.ttf", 12.0f, &config, forkawesome_icon_ranges);
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

        _mixer->SetNoteSource(this);

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

        _patternEditor.SetUp(_mixer, _monofont);

        return true;
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

    const int instrumentPanelWidth = 370;
    const int effectsPanelHeight = 160;
    const int playerControlsPanelWidth = 153;
    const int playerControlsPanelHeight = 80;

    virtual void Render2d()
    {
        //show Main Window
        ImGui::ShowDemoWindow();

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings;

        ImGui::SetNextWindowSize(ImVec2(playerControlsPanelWidth, playerControlsPanelHeight));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::Begin(
            "PlayerControls",
            nullptr,
            flags);
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
                _patternEditor.currentRow = 0;
            }
            ImGui::SameLine();

            bool isRecording = _patternEditor.IsRecording();
            if (isRecording)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(255, 0, 0, 155));
            }
            if (ImGui::Button(ICON_FAD_RECORD, ImVec2(0, 0)))
            {
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
        ImGui::Begin(
            "instruments",
            nullptr,
            flags);
        {
            if (ImGui::CollapsingHeader("Instrument Tracks", ImGuiTreeNodeFlags_DefaultOpen))
            {
                ImGui::BeginChild("TracksContainer", ImVec2(0, 300));
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
                        _patternEditor.currentTrack = i;
                    }
                    ImGui::SameLine();

                    char buf[256] = {0};
                    sprintf_s(buf, 256, "%02d : %s", int(i), track->Pname);
                    ImGui::Selectable(buf, i == _patternEditor.currentTrack);
                    if (ImGui::IsItemClicked())
                    {
                        _patternEditor.currentTrack = i;
                    }

                    ImGui::PopID();
                }
                ImGui::EndChild();

                if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !_patternEditor.HandlePlayingNotes())
                {
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow)))
                    {
                        if (_patternEditor.currentTrack > 0)
                        {
                            _patternEditor.currentTrack--;
                        }
                    }
                    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow)))
                    {
                        if (_patternEditor.currentTrack < NUM_MIXER_TRACKS - 1)
                        {
                            _patternEditor.currentTrack++;
                        }
                    }
                }
            }

            if (ImGui::CollapsingHeader("Instrument Properties"))
            {
                ImGui::BeginChild("InstrumentProperties", ImVec2(0, 200));
                if (_patternEditor.currentTrack < NUM_MIXER_TRACKS)
                {
                    auto track = _mixer->GetTrack(_patternEditor.currentTrack);

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
                    ImGui::Text(
                        "By : %s", (track->info.Pauthor));
                    ImGui::TextWrapped(
                        "%s", track->info.Pcomments);
                }
                ImGui::EndChild();
            }

            if (ImGui::CollapsingHeader("Instrument Library"))
            {
                // todo : dont do this every frame
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
                        auto const &track = _mixer->GetTrack(_patternEditor.currentTrack);
                        track->Lock();
                        _library->LoadAsInstrument(selection, track);
                        track->Penabled = 1;
                        track->ApplyParameters();
                        track->Unlock();
                    }
                }
            }
        }
        ImGui::End();

        char const *const EffectNames[] = {
            "No effect",
            "Reverb",
            "Echo",
            "Chorus",
            "Phaser",
            "AlienWah",
            "Distortion",
            "Equalizer",
            "DynFilter",
        };

        static char const *const reverbPresetNames[] = {
            "Cathedral 1",
            "Cathedral 2",
            "Cathedral 3",
            "Hall 1",
            "Hall 2",
            "Room 1",
            "Room 2",
            "Basement",
            "Tunnel",
            "Echoed 1",
            "Echoed 2",
            "Very Long 1",
            "Very Long 2",
        };

        ImGui::SetNextWindowSize(ImVec2(Width() - instrumentPanelWidth, effectsPanelHeight));
        ImGui::SetNextWindowPos(ImVec2(0, Height() - effectsPanelHeight));
        ImGui::Begin(
            "Effects Editor",
            nullptr,
            flags);
        {
            auto track = _mixer->GetTrack(_patternEditor.currentTrack);
            for (int e = 0; e < NUM_TRACK_EFX; e++)
            {
                if (e > 0) ImGui::SameLine();
                ImGui::PushID(e);

                int item_current = track->partefx[e]->geteffect();
                ImGui::BeginChild("fx", ImVec2(320, 120), true);

                ImGui::SetNextItemWidth(100);
                if (ImGui::Combo("##effect", &item_current, EffectNames, IM_ARRAYSIZE(EffectNames)))
                {
                    track->partefx[e]->changeeffect(item_current);
                }

                ImGui::SameLine();

                int preset_current = track->partefx[e]->getpreset();
                ImGui::SetNextItemWidth(150);
                if (ImGui::Combo("preset", &preset_current, reverbPresetNames, IM_ARRAYSIZE(reverbPresetNames)))
                {
                    track->partefx[e]->changepreset(preset_current);
                }

                ImGui::EndChild();
                ImGui::PopID();
            }
        }
        ImGui::End();

        ImGui::SetNextWindowSize(ImVec2(Width() - playerControlsPanelWidth - instrumentPanelWidth, Height() - effectsPanelHeight));
        ImGui::SetNextWindowPos(ImVec2(playerControlsPanelWidth, 0));

        _patternEditor.Render2d();
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
