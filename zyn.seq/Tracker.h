#ifndef TRACKER_H
#define TRACKER_H

#include "Track.h"
#include <vector>
#include <zyn.common/globals.h>
#include <zyn.nio/MidiInputManager.h>

struct TrackerMidiEvent
{
    MidiEvent event;
    unsigned int trackerFrame;
};

class Tracker : public IMidiHook
{
    Track _tracks[NUM_MIDI_CHANNELS];
    SystemSettings *_settings;
    unsigned int _lastFrame;
    bool _isPlaying;
    unsigned int _currentStep;
    unsigned int _bpm;
    unsigned int _framesPerStep;
    unsigned int _lastFramesInStep;
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

    virtual void Trigger(unsigned int frameStart, unsigned int frameStop);
};

#endif // TRACKER_H
