#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "Pattern.h"
#include <chrono>
#include <map>
#include <zyn.common/globals.h>

class TrackPatternNote
{
public:
    TrackPatternNote();
    TrackPatternNote(unsigned char note, int step, float length);

    bool operator<(TrackPatternNote const &other) const;
    unsigned char _note;
    int _step;
    float _length;
};

class TrackPattern
{
public:
    TrackPattern();
    TrackPattern(std::string const &name, float hue);

    std::string _name;
    float _hue;
    std::set<TrackPatternNote> _notes;

    bool IsStepCovered(unsigned char note, int step);
};

class Note
{
public:
    Note();
    Note(Note const &note);
    Note(unsigned char note, unsigned char velocity, float length);
    virtual ~Note();

    unsigned char _note;
    unsigned char _velocity;
    float _length;
};

class ISteppable
{
public:
    virtual ~ISteppable();

    virtual std::vector<Note> GetNote(unsigned char trackIndex, int patternIndex, int stepIndex) = 0;
    virtual int CountSongLengthInPatterns() = 0;
};

int const maxNotes = 255;

class Stepper
{
protected:
    ISteppable *_steppable;
    IMixer *_mixer;
    bool isPlaying;
    std::chrono::milliseconds::rep _lastSequencerTimeInMs;
    std::chrono::milliseconds::rep _playerTimeInMs;
    std::chrono::milliseconds::rep _stepTimeInMs;
    int _bpm;
    int _currentStep;
    std::chrono::milliseconds::rep _activeNotes[NUM_MIXER_CHANNELS][maxNotes];

    void Step(int step);

public:
    Stepper(ISteppable *steppable, IMixer *mixer);
    virtual ~Stepper();

    void Setup();
    virtual void Tick();
    void HitNote(unsigned char chan, unsigned char note, unsigned char velocity, int durationInMs);

    bool IsPlaying();
    void Stop();
    void PlayPause();

    int Bpm() const;
    void Bpm(int bpm);
};

class Sequencer : public ISteppable
{
    std::map<int, TrackPattern> tracksOfPatterns[NUM_MIXER_CHANNELS];
    int _activeInstrument;
    int _activePattern;

public:
    Sequencer();
    virtual ~Sequencer();

    int CountSongLengthInPatterns();
    void AddPattern(int trackIndex, int patternIndex, char const *label);
    bool DoesPatternExistAtIndex(int trackIndex, int patternIndex);
    TrackPattern &GetPattern(int trackIndex, int patternIndex);
    void SetPattern(int trackIndex, int patternIndex, TrackPattern const &pattern);
    void RemoveActivePattern();
    void MovePatternLeftIfPossible();
    void MovePatternLeftForced();
    void SwitchPatternLeft();
    void MovePatternRightIfPossible();
    void MovePatternRightForced();
    void SwitchPatternRight();
    void SwitchPatterns(int firstKey, int secondKey);
    void SelectFirstPatternInTrack();
    void SelectLastPatternInTrack();
    void SelectPreviousPattern();
    void SelectNextPattern();
    int LastPatternIndex(int trackIndex);
    int PatternStepCount(int trackIndex, int patternIndex);

    int ActiveInstrument() const;
    void ActiveInstrument(int newActiveInstrument);
    int ActivePattern() const;
    void ActivePattern(int newActivePattern);

    // IStepper implementation
    virtual std::vector<Note> GetNote(unsigned char trackIndex, int patternIndex, int stepIndex);
};

#endif // SEQUENCER_H
