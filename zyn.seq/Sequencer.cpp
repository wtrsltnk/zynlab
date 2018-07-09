#include "Sequencer.h"
#include "SequencerStrip.h"
#include <zyn.mixer/Mixer.h>

#include <chrono>
#include <ctime>
#include <iostream>

Sequencer::Sequencer(class IMixer *mixer)
    : _mixer(mixer), _playThread([this]() { this->runThread(); }),
      _playingState(PlayingStates::Stopped),
      _currentStep(0), _prevStep(0), _currentStepTime(0.0),
      _bpm(100)
{
    this->_playThread.detach();
}

Sequencer::~Sequencer()
{}

void Sequencer::runThread()
{
    std::clock_t start = std::clock();
    std::clock_t lastCheck = start;

    while (this->currentState() != PlayingStates::Quit)
    {
        std::clock_t check = std::clock();
        if (this->currentState() == PlayingStates::Playing)
        {
            double stepTime = (60.0 / double(this->bpm())) / 4.0;
            //            std::cout << stepTime << std::endl;

            bool doStepAction = false;
            this->_changeCurrentStep.lock();
            this->_currentStepTime += ((check - lastCheck) / double(CLOCKS_PER_SEC));
            if (this->_currentStepTime >= stepTime)
            {
                this->_currentStepTime -= stepTime;
                this->_prevStep = this->_currentStep;
                this->_currentStep++;
                doStepAction = true;
            }
            this->_changeCurrentStep.unlock();

            if (doStepAction) this->doStep();
        }
        lastCheck = check;

        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

PlayingStates Sequencer::currentState()
{
    this->_changePlayingState.lock();
    auto result = this->_playingState;
    this->_changePlayingState.unlock();

    return result;
}

int Sequencer::currentStep()
{
    this->_changeCurrentStep.lock();
    auto result = this->_currentStep;
    this->_changeCurrentStep.unlock();

    return result;
}

void Sequencer::doStep()
{
    this->_mixer->Lock();
    for (SequencerStrip *strip : this->_channels)
    {
        auto prev = strip->_steps.find(this->_prevStep % 16);
        if (prev != strip->_steps.end())
        {
            this->_mixer->NoteOff(strip->_targetMixerChannel, prev->second._note);
        }

        auto curr = strip->_steps.find(this->_currentStep % 16);
        if (curr != strip->_steps.end())
        {
            this->_mixer->NoteOn(strip->_targetMixerChannel, curr->second._note, int(curr->second._velocity));
        }
    }
    this->_mixer->Unlock();
}

void Sequencer::setBpm(int bpm)
{
    this->_changeBpm.lock();
    this->_bpm = bpm;
    this->_changeBpm.unlock();
}

int Sequencer::bpm()
{
    this->_changeBpm.lock();
    auto result = this->_bpm;
    //    std::cout << result << std::endl;
    this->_changeBpm.unlock();

    return result;
}

void Sequencer::Start()
{
    auto state = this->currentState();

    // We do not change anymore when we are quitting.
    if (state == PlayingStates::Quit) return;

    // No need to change when we are already started
    if (state == PlayingStates::Playing) return;

    // Simply enable playing again when we were paused or stopped
    if (state == PlayingStates::Paused || state == PlayingStates::Stopped)
    {
        this->_changePlayingState.lock();
        this->_playingState = PlayingStates::Playing;
        this->_changePlayingState.unlock();
    }
}

void Sequencer::Pause()
{
    auto state = this->currentState();

    // We do not change anymore when we are quitting.
    if (state == PlayingStates::Quit) return;

    // No need to change when we are already paused
    if (state == PlayingStates::Paused) return;

    if (state == PlayingStates::Stopped || state == PlayingStates::Playing)
    {
        this->_changePlayingState.lock();
        this->_playingState = PlayingStates::Stopped;
        this->_changePlayingState.unlock();
    }
}

void Sequencer::Stop()
{
    auto state = this->currentState();

    // We do not change anymore when we are quitting.
    if (state == PlayingStates::Quit) return;

    // No need to change when we are already stopped
    if (state == PlayingStates::Stopped) return;

    // Stop and reset the current step
    if (state == PlayingStates::Paused || state == PlayingStates::Playing)
    {
        this->_changePlayingState.lock();
        this->_playingState = PlayingStates::Stopped;
        this->_changePlayingState.unlock();

        this->_changeCurrentStep.lock();
        this->_prevStep = 0;
        this->_currentStep = 0;
        this->_changeCurrentStep.unlock();
    }
}

void Sequencer::Quit()
{
    auto state = this->currentState();

    // We do not change anymore when we are quitting.
    if (state == PlayingStates::Quit) return;

    this->_changePlayingState.lock();
    this->_playingState = PlayingStates::Quit;
    this->_changePlayingState.unlock();
}

void Sequencer::setStep(int step, int note, double velocity)
{
    if (this->_channels.empty()) this->_channels.push_back(new SequencerStrip());
    std::cout << step << ", " << note << ", " << velocity << std::endl;
    SequencerStrip *strip = this->_channels.front();
    if (strip->_steps.find(step - 1) != strip->_steps.end()) strip->_steps.erase(step - 1);
    strip->_steps.insert(std::make_pair(step - 1, SequencerStep(char(note), char(255 * velocity))));
}

void Sequencer::clearStep(int step)
{
    std::cout << step << std::endl;
    SequencerStrip *strip = this->_channels.front();
    if (strip->_steps.find(step - 1) != strip->_steps.end()) strip->_steps.erase(step - 1);
}

bool Sequencer::isStepOn(int step)
{
    SequencerStrip *strip = this->_channels.front();
    return strip->_steps.find(step) != strip->_steps.end();
}
