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

class ApplicationSession
{
public:
    ApplicationSession();

    Mixer *_mixer;
    ILibraryManager *_library;

    PlayStates _playState = PlayStates::Stopped;
    Song *_song;
    unsigned int _bpm = 138;
    unsigned int currentTrack = 0;
    unsigned int currentRow = 0;
    unsigned int currentProperty = 0;
    unsigned int skipRowStepSize = 4;

    void StopPlaying();
    void StartPlaying();
    void TogglePlaying();
};

#endif // APPLICATIONSESSION_H
