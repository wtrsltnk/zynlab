#ifndef TRACKER_H
#define TRACKER_H

#include "Track.h"
#include <vector>
#include <zyn.common/globals.h>
#include <zyn.nio/MidiInputManager.h>

struct TrackerMidiEvent
{
    MidiEvent event;
    float trackerFrame;
};

class Tracker
{
    SystemSettings *_settings;
    Track _tracks[NUM_MIDI_CHANNELS];
    unsigned int _lastFrame;
    bool _isPlaying;
    unsigned int _currentStep;
    unsigned int _bpm;
    float _stepTime;

    std::vector<TrackerMidiEvent> _trackerEvents;

    void NextStep();

public:
    Tracker(SystemSettings *settings);
    virtual ~Tracker();

    void SetBpm(unsigned int bpm);

    void Play();
    void Pause();
    void Stop();
    bool IsPlaying() const;
    bool IsPaused() const;
    bool IsStoped() const;
    unsigned int CurrentStep() const;
    unsigned int maxPatternCount();

    Track *Tracks();
};

#endif // TRACKER_H
