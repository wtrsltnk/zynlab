#include "Tracker.h"
#include <algorithm>

Tracker::Tracker(SystemSettings *settings)
    : _settings(settings), _lastFrame(0), _isPlaying(false), _currentStep(0), _framesPerStep(0), _lastFramesInStep(0)
{}

Tracker::~Tracker() = default;

void Tracker::Trigger(unsigned int frameStart, unsigned int frameStop)
{
    if (!_isPlaying)
    {
        return;
    }

    _lastFramesInStep += (frameStop - frameStart);
    while (_lastFramesInStep > _framesPerStep)
    {
        NextStep();
        _lastFramesInStep -= _framesPerStep;
    }

    _lastFrame += (frameStop - frameStart);
}

unsigned int Tracker::maxPatternCount()
{
    // Find track track height
    unsigned int maxPatterns = 0;
    for (int trackIndex = 0; trackIndex < NUM_MIDI_CHANNELS; trackIndex++)
    {
        auto patterns = _tracks[trackIndex]._patterns.size() + _tracks[trackIndex]._patternsStart;

        if (maxPatterns < patterns)
        {
            maxPatterns = patterns;
        }
    }

    return maxPatterns * NUM_PATTERN_EVENTS;
}

bool myfunction(TrackerMidiEvent &i, TrackerMidiEvent &j) { return (i.trackerFrame > j.trackerFrame); }

void Tracker::NextStep()
{
    _currentStep++;

    if (_currentStep >= maxPatternCount())
    {
        _currentStep = 0;
        _lastFrame = 0;
    }

    for (unsigned int i = 0; i < NUM_MIDI_CHANNELS; i++)
    {
        auto event = _tracks[i].GetPatternEventByStep(_currentStep);

        if (event == nullptr)
        {
            continue;
        }

        TrackerMidiEvent e1;
        e1.trackerFrame = _currentStep * _framesPerStep;
        e1.event.channel = i;
        e1.event.num = event->_note;
        e1.event.time = 0;
        e1.event.value = event->_velocity;
        _trackerEvents.push_back(e1);

        TrackerMidiEvent e2;
        e2.trackerFrame = (_currentStep * _framesPerStep) + _framesPerStep;
        e2.event.channel = i;
        e2.event.num = event->_note;
        e2.event.time = 0;
        e2.event.value = 0;
        _trackerEvents.push_back(e2);

        std::sort(_trackerEvents.begin(), _trackerEvents.end(), myfunction);
    }

    while (!_trackerEvents.empty())
    {
        if (_trackerEvents.back().trackerFrame > _lastFrame)
        {
            break;
        }

        MidiInputManager::Instance().PutEvent(_trackerEvents.back().event);
        _trackerEvents.pop_back();
    }
}

void Tracker::SetBpm(unsigned int bpm)
{
    _bpm = bpm;
    auto stepsPerSecond = float(_bpm) / 60.0f;
    _framesPerStep = static_cast<unsigned int>(float(_settings->samplerate) / stepsPerSecond);
}

void Tracker::Play()
{
    _isPlaying = true;
}

void Tracker::Pause()
{
    _isPlaying = false;
}

void Tracker::Stop()
{
    _isPlaying = false;
    _lastFrame = 0;
    _currentStep = 0;
}

bool Tracker::IsPlaying() const
{
    return _isPlaying;
}

bool Tracker::IsPaused() const
{
    return !_isPlaying;
}

bool Tracker::IsStoped() const
{
    return !_isPlaying && _currentStep == 0;
}

unsigned int Tracker::CurrentStep() const
{
    return _currentStep;
}

Track *Tracker::Tracks()
{
    return _tracks;
}
