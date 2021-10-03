#include "applicationsession.h"

#include <zyn.mixer/Track.h>

ApplicationSession::ApplicationSession()
{}

void ApplicationSession::StopPlaying()
{
    _playState = PlayStates::Stopped;
    currentRow = 0;
    for (unsigned int t = 0; t < _mixer->GetTrackCount(); t++)
    {
        _mixer->GetTrack(t)->RelaseAllKeys();
    }
}

void ApplicationSession::PausePlaying()
{
    _playState = PlayStates::Stopped;

    for (unsigned int t = 0; t < _mixer->GetTrackCount(); t++)
    {
        _mixer->GetTrack(t)->RelaseAllKeys();
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
