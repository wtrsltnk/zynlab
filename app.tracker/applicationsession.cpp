#include "applicationsession.h"

ApplicationSession::ApplicationSession()
{}

void ApplicationSession::StopPlaying()
{
    _playState = PlayStates::Stopped;
    currentRow = 0;
    for (int t = 0; t < NUM_MIXER_TRACKS; t++)
    {
        _mixer->GetTrack(t)
            ->RelaseAllKeys();
    }
}

void ApplicationSession::StartPlaying()
{
    _playState = PlayStates::StartPlaying;
}

void ApplicationSession::TogglePlaying()
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

bool ApplicationSession::IsRecording() const
{
    return _editMode;
}

void ApplicationSession::ToggleRecording()
{
    _editMode = !_editMode;
}
