#include "Sequencer.h"
#include <cmath>

TrackPatternNote::TrackPatternNote() {}

TrackPatternNote::TrackPatternNote(unsigned char note, int step, float length)
    : _note(note), _step(step), _length(length)
{}

bool TrackPatternNote::operator<(TrackPatternNote const &other) const
{
    return (_note < other._note) || (_note == other._note && _step < other._step);
}

TrackPattern::TrackPattern()
{}

TrackPattern::TrackPattern(std::string const &name, float hue)
    : _name(name), _hue(hue)
{}

bool TrackPattern::IsStepCovered(unsigned char note, int step)
{
    for (auto &tnote : _notes)
    {
        if (tnote._note == note && tnote._step <= step && (tnote._step +tnote._length) >= step) return true;
    }
    return false;
}

Note::Note() = default;

Note::Note(Note const &note)
    : _note(note._note), _velocity(note._velocity), _length(note._length)
{}

Note::Note(unsigned char note, unsigned char velocity, float length)
    : _note(note), _velocity(velocity), _length(length)
{}

Note::~Note() = default;

ISteppable::~ISteppable() = default;

Stepper::Stepper(ISteppable *steppable, IMixer *mixer)
    : _steppable(steppable), _mixer(mixer)
{}

Stepper::~Stepper() = default;

std::chrono::milliseconds::rep calculateStepTime(int bpm)
{
    auto beatsPerSecond = static_cast<double>(bpm * 4) / 60.0;

    return static_cast<std::chrono::milliseconds::rep>(1000 / beatsPerSecond);
}

void Stepper::Setup()
{
    _lastSequencerTimeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    _playerTimeInMs = 0;
    _currentStep = 0;
    _bpm = 120;
    _stepTimeInMs = calculateStepTime(_bpm);
}

void Stepper::Tick()
{
    std::chrono::milliseconds::rep currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    auto deltaTime = currentTime - _lastSequencerTimeInMs;
    _lastSequencerTimeInMs = currentTime;

    for (int trackIndex = 0; trackIndex < NUM_MIXER_CHANNELS; trackIndex++)
    {
        for (int note = 0; note < maxNotes; note++)
        {
            if (_activeNotes[trackIndex][note] <= 0)
            {
                continue;
            }

            _activeNotes[trackIndex][note] -= deltaTime;
            if (_activeNotes[trackIndex][note] <= 0)
            {
                _mixer->NoteOff(static_cast<unsigned char>(trackIndex), static_cast<unsigned char>(note));
            }
        }
    }
    if (!IsPlaying())
    {
        return;
    }

    _playerTimeInMs += deltaTime;

    if (_playerTimeInMs > _stepTimeInMs)
    {
        _currentStep++;
        if (_currentStep >= (_steppable->CountSongLengthInPatterns() * 16))
        {
            _currentStep = 0;
        }
        Step(_currentStep);
        _playerTimeInMs -= _stepTimeInMs;
    }
}

void Stepper::HitNote(unsigned char chan, unsigned char note, unsigned char velocity, int durationInMs)
{
    _activeNotes[chan][note] = durationInMs;
    _mixer->NoteOn(chan, note, velocity);
}

int Stepper::Bpm() const
{
    return _bpm;
}

void Stepper::Bpm(int bpm)
{
    _bpm = bpm;
    _stepTimeInMs = calculateStepTime(_bpm);
}

bool Stepper::IsPlaying()
{
    return isPlaying;
}

void Stepper::Stop()
{
    isPlaying = false;
    _playerTimeInMs = 0;
}

void Stepper::PlayPause()
{
    isPlaying = !isPlaying;

    if (isPlaying)
    {
        Step(_currentStep);
    }
}

void Stepper::Step(int step)
{
    auto patternIndex = step / 16;
    auto stepIndex = step % 16;

    for (unsigned char trackIndex = 0; trackIndex < NUM_MIXER_CHANNELS; trackIndex++)
    {
        auto notes = _steppable->GetNote(trackIndex, patternIndex, stepIndex);
        for (auto note : notes)
        {
            HitNote(trackIndex, note._note, note._velocity, static_cast<int>(note._length * 1000.0f));
        }
    }
}

std::vector<Note> Sequencer::GetNote(unsigned char trackIndex, int patternIndex, int stepIndex)
{
    std::vector<Note> result;

    auto track = tracksOfPatterns[trackIndex];
    if (track.find(patternIndex) != track.end())
    {
        auto pattern = track[patternIndex];
        for (auto note : pattern._notes)
        {
            if (note._step == stepIndex)
            {
                result.push_back(Note(note._note, 200, 0.2f));
            }
        }
    }

    return result;
}

Sequencer::Sequencer() = default;

Sequencer::~Sequencer() = default;

int Sequencer::CountSongLengthInPatterns()
{
    int maxStep = 0;
    for (int trackIndex = 0; trackIndex < NUM_MIXER_CHANNELS; trackIndex++)
    {
        auto &pattern = tracksOfPatterns[trackIndex];
        if (pattern.empty())
        {
            continue;
        }
        if (pattern.rbegin()->first >= maxStep)
        {
            maxStep = pattern.rbegin()->first;
        }
    }

    return maxStep + 1;
}

void Sequencer::AddPattern(int trackIndex, int patternIndex, char const *label)
{
    static int n = std::rand() % 255;
    float hue = n * 0.05f;
    _activeInstrument = trackIndex;
    _activePattern = patternIndex;
    tracksOfPatterns[trackIndex].insert(std::make_pair(patternIndex, TrackPattern(label, hue)));
    n = (n + std::rand()) % 255;
}

bool Sequencer::DoesPatternExistAtIndex(int trackIndex, int patternIndex)
{
    if (trackIndex < 0 || trackIndex >= NUM_MIXER_CHANNELS)
    {
        return false;
    }

    return tracksOfPatterns[trackIndex].find(patternIndex) != tracksOfPatterns[trackIndex].end();
}

TrackPattern &Sequencer::GetPattern(int trackIndex, int patternIndex)
{
    return tracksOfPatterns[trackIndex][patternIndex];
}

void Sequencer::SetPattern(int trackIndex, int patternIndex, TrackPattern const &pattern)
{
    _activeInstrument = trackIndex;
    _activePattern = patternIndex;
    tracksOfPatterns[trackIndex].erase(patternIndex);
    tracksOfPatterns[trackIndex].insert(std::make_pair(patternIndex, pattern));
}

void Sequencer::RemoveActivePattern()
{
    if (_activeInstrument < 0 || _activePattern < 0)
    {
        return;
    }

    tracksOfPatterns[_activeInstrument].erase(_activePattern);
    _activeInstrument = -1;
    _activePattern = -1;
}

void Sequencer::MovePatternLeftIfPossible()
{
    auto ap = tracksOfPatterns[_activeInstrument].find(_activePattern);

    if (_activeInstrument < 0 || _activePattern < 0 || ap == tracksOfPatterns[_activeInstrument].end())
    {
        return;
    }

    auto currentKey = ap->first;

    if (currentKey == 0)
    {
        return;
    }

    auto currentValue = ap->second;
    auto newKey = currentKey - 1;

    if (tracksOfPatterns[_activeInstrument].find(newKey) == tracksOfPatterns[_activeInstrument].end())
    {
        tracksOfPatterns[_activeInstrument].insert(std::make_pair(newKey, currentValue));
        tracksOfPatterns[_activeInstrument].erase(currentKey);
        _activePattern = newKey;
    }
}

void Sequencer::MovePatternLeftForced()
{
    auto ap = tracksOfPatterns[_activeInstrument].find(_activePattern);

    if (_activeInstrument < 0 || _activePattern < 0 || ap == tracksOfPatterns[_activeInstrument].end())
    {
        return;
    }

    if (tracksOfPatterns[_activeInstrument].begin()->first == 0)
    {
        return;
    }

    for (int i = tracksOfPatterns[_activeInstrument].begin()->first; i <= ap->first; i++)
    {
        auto itr = tracksOfPatterns[_activeInstrument].find(i);
        if (itr == tracksOfPatterns[_activeInstrument].end())
        {
            continue;
        }
        tracksOfPatterns[_activeInstrument].insert(std::make_pair(i - 1, itr->second));
        tracksOfPatterns[_activeInstrument].erase(i);
    }

    _activePattern = _activePattern - 1;
}

void Sequencer::SwitchPatternLeft()
{
    auto ap = tracksOfPatterns[_activeInstrument].find(_activePattern);

    if (_activeInstrument < 0 || _activePattern < 0 || ap == tracksOfPatterns[_activeInstrument].end())
    {
        return;
    }

    auto currentKey = ap->first;

    if (currentKey <= 0)
    {
        return;
    }

    auto currentValue = ap->second;
    auto newKey = currentKey - 1;

    tracksOfPatterns->erase(currentKey);

    if (tracksOfPatterns[_activeInstrument].find(newKey) != tracksOfPatterns[_activeInstrument].end())
    {
        auto tmpValue = tracksOfPatterns[_activeInstrument].find(newKey)->second;
        tracksOfPatterns[_activeInstrument].erase(newKey);
        tracksOfPatterns[_activeInstrument].insert(std::make_pair(currentKey, tmpValue));
    }

    tracksOfPatterns[_activeInstrument].insert(std::make_pair(newKey, currentValue));

    _activePattern = newKey;
}

void Sequencer::MovePatternRightIfPossible()
{
    auto ap = tracksOfPatterns[_activeInstrument].find(_activePattern);

    if (_activeInstrument < 0 || _activePattern < 0 || ap == tracksOfPatterns[_activeInstrument].end())
    {
        return;
    }

    auto currentKey = ap->first;
    auto currentValue = ap->second;
    auto newKey = currentKey + 1;

    ap++;
    auto nextKey = ap->first;
    if (ap == tracksOfPatterns[_activeInstrument].end() || newKey < nextKey)
    {
        tracksOfPatterns[_activeInstrument].insert(std::make_pair(newKey, currentValue));
        tracksOfPatterns[_activeInstrument].erase(currentKey);
        _activePattern = newKey;
    }
}

void Sequencer::MovePatternRightForced()
{
    auto ap = tracksOfPatterns[_activeInstrument].find(_activePattern);

    if (_activeInstrument < 0 || _activePattern < 0 || ap == tracksOfPatterns[_activeInstrument].end())
    {
        return;
    }

    for (int i = tracksOfPatterns[_activeInstrument].rbegin()->first; i >= ap->first; i--)
    {
        auto itr = tracksOfPatterns[_activeInstrument].find(i);
        if (itr == tracksOfPatterns[_activeInstrument].end())
        {
            continue;
        }
        tracksOfPatterns[_activeInstrument].insert(std::make_pair(i + 1, itr->second));
        tracksOfPatterns[_activeInstrument].erase(i);
    }

    _activePattern = _activePattern + 1;
}

void Sequencer::SwitchPatternRight()
{
    auto ap = tracksOfPatterns[_activeInstrument].find(_activePattern);

    if (_activeInstrument < 0 || _activePattern < 0 || ap == tracksOfPatterns[_activeInstrument].end())
    {
        return;
    }

    auto currentKey = ap->first;
    auto currentValue = ap->second;
    auto newKey = currentKey + 1;

    tracksOfPatterns->erase(currentKey);

    if (tracksOfPatterns[_activeInstrument].find(newKey) != tracksOfPatterns[_activeInstrument].end())
    {
        auto tmpValue = tracksOfPatterns[_activeInstrument].find(newKey)->second;
        tracksOfPatterns[_activeInstrument].erase(newKey);
        tracksOfPatterns[_activeInstrument].insert(std::make_pair(currentKey, tmpValue));
    }

    tracksOfPatterns[_activeInstrument].insert(std::make_pair(newKey, currentValue));

    _activePattern = newKey;
}

void Sequencer::SelectFirstPatternInTrack()
{
    if (_activeInstrument < 0)
    {
        return;
    }

    _activePattern = tracksOfPatterns[_activeInstrument].begin()->first;
}

void Sequencer::SelectLastPatternInTrack()
{
    if (_activeInstrument < 0)
    {
        return;
    }

    _activePattern = tracksOfPatterns[_activeInstrument].rbegin()->first;
}

void Sequencer::SelectPreviousPattern()
{
    if (_activeInstrument < 0)
    {
        return;
    }

    if (_activePattern <= 0)
    {
        return;
    }

    int newIndex = _activePattern - 1;
    while (newIndex >= 0)
    {
        if (DoesPatternExistAtIndex(_activeInstrument, newIndex))
        {
            _activePattern = newIndex;
            break;
        }
        newIndex--;
    }
}

void Sequencer::SelectNextPattern()
{
    if (_activeInstrument < 0)
    {
        return;
    }

    auto lastIndex = LastPatternIndex(_activeInstrument);
    if (_activePattern == lastIndex)
    {
        return;
    }

    int newIndex = _activePattern + 1;
    while (newIndex <= lastIndex)
    {
        if (DoesPatternExistAtIndex(_activeInstrument, newIndex))
        {
            _activePattern = newIndex;
            break;
        }
        newIndex++;
    }
}

int Sequencer::LastPatternIndex(int trackIndex)
{
    return tracksOfPatterns[trackIndex].empty() ? -1 : tracksOfPatterns[trackIndex].rbegin()->first;
}

int Sequencer::PatternStepCount(int trackIndex, int patternIndex)
{
    if (!DoesPatternExistAtIndex(trackIndex, patternIndex))
    {
        return 0;
    }

    auto pattern = GetPattern(trackIndex, patternIndex);

    int maxSize = 0;
    for (auto &note : pattern._notes)
    {
        auto end = note._step + std::ceil(note._length);
        if (maxSize < end)
        {
            maxSize = static_cast<int>(end);
        }
    }

    return maxSize;
}

int Sequencer::ActiveInstrument() const
{
    return _activeInstrument;
}

void Sequencer::ActiveInstrument(int newActiveInstrument)
{
    _activeInstrument = newActiveInstrument;
}

int Sequencer::ActivePattern() const
{
    return _activePattern;
}

void Sequencer::ActivePattern(int newActivePattern)
{
    _activePattern = newActivePattern;
}
