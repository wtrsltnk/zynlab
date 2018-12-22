#ifndef SEQUENCER_H
#define SEQUENCER_H

#include "Pattern.h"
#include <chrono>
#include <map>
#include <zyn.common/globals.h>

class Note
{
public:
    Note();
    Note(Note const &note);
    Note(unsigned char note, unsigned char velocity);
    virtual ~Note();

    unsigned char _note;
    unsigned char _velocity;
};

class ISteppable
{
public:
    virtual ~ISteppable();

    virtual std::vector<Note> GetNote(unsigned char trackIndex, int patternIndex, int stepIndex) = 0;
    virtual int CountSongLength() = 0;
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
    void HitNote(unsigned char chan, unsigned char note, int durationInMs);

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

    int CountSongLength();
    void AddPattern(int trackIndex, int patternIndex, char const *label);
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
    bool DoesPatternExistAtIndex(int trackIndex, int patternIndex);
    TrackPattern &GetPattern(int trackIndex, int patternIndex);

    int ActiveInstrument() const;
    void ActiveInstrument(int newActiveInstrument);
    int ActivePattern() const;
    void ActivePattern(int newActivePattern);

    // IStepper implementation
    virtual std::vector<Note> GetNote(unsigned char trackIndex, int patternIndex, int stepIndex);
};

#endif // SEQUENCER_H
