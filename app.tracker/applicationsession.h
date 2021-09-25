#ifndef APPLICATIONSESSION_H
#define APPLICATIONSESSION_H

#include "song.h"
#include <zyn.common/ILibraryManager.h>
#include <zyn.common/globals.h>

enum class PlayStates
{
    Stopped,
    StartPlaying,
    Playing,
};

enum class SelectableTabs
{
    PatternEditor,
    Mixer,
    Synth,
};

class ApplicationSession
{
public:
    ApplicationSession();

    IMixer *_mixer = nullptr;
    ILibraryManager *_library = nullptr;

    PlayStates _playState = PlayStates::Stopped;
    bool _editMode = false;
    Song *_song = nullptr;
    unsigned int _bpm = 138;
    SelectableTabs selectedTab = SelectableTabs::Synth;

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
