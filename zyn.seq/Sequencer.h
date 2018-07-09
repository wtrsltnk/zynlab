#ifndef SEQUENCER_H
#define SEQUENCER_H

#include <mutex>
#include <thread>
#include <vector>

enum class PlayingStates
{
    Stopped,
    Playing,
    Paused,
    Quit,
};

class Sequencer
{
    class IMixer *_mixer;
    std::thread _playThread;

    PlayingStates _playingState;
    std::mutex _changePlayingState;

    int _currentStep;
    int _prevStep;
    double _currentStepTime;
    std::mutex _changeCurrentStep;

    int _bpm;
    std::mutex _changeBpm;

    std::vector<class SequencerStrip *> _channels;
    std::mutex _changeChannels;

    void runThread();
    void doStep();

public:
    Sequencer(class IMixer *mixer);
    virtual ~Sequencer();

    PlayingStates currentState();
    int currentStep();

    void setBpm(int bpm);
    int bpm();

    void Start();
    void Pause();
    void Stop();
    void Quit();

    void setStep(int step, int note, double velocity);
    void clearStep(int step);
    bool isStepOn(int step);
};

#endif // SEQUENCER_H
