#ifndef APPLICATIONSESSION_H
#define APPLICATIONSESSION_H

#include "song.h"
#include <zyn.mixer/Mixer.h>

enum class PlayStates
{
    Stopped,
    StartPlaying,
    Playing,
};

enum class ActiveSynths
{
    Add,
    Sub,
    Pad,
    Smpl,
};

enum class SelectableTabs
{
    PatternEditor,
    Mixer,
    Synth,
    AutomationEditor,
};

class ApplicationSession
{
public:
    ApplicationSession();

    Mixer *_mixer = nullptr;
    ILibraryManager *_library = nullptr;

    PlayStates _playState = PlayStates::Stopped;
    bool _editMode = false;
    Song *_song = nullptr;
    unsigned int _bpm = 138;
    SelectableTabs selectedTab = SelectableTabs::Synth;

    // Synth state
    unsigned int currentTrack = 0;
    unsigned int currentTrackInstrument = 0;
    unsigned int currentVoice = NUM_VOICES;
    ActiveSynths currentSynth = ActiveSynths::Add;

    // Pattern state
    unsigned int currentRow = 0;
    unsigned int currentProperty = 0;
    unsigned int skipRowStepSize = 4;

    void StopPlaying();
    void StartPlaying();
    void TogglePlaying();

    bool IsRecording() const;
    void ToggleRecording();
};

#endif // APPLICATIONSESSION_H
