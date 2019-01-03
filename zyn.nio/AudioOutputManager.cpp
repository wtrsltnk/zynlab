#include "AudioOutputManager.h"
#include "AudioOutput.h"
#include "Engine.h"
#include "EngineManager.h"
#include "MidiInputManager.h"
#include "WavEngine.h"
#include <zyn.common/Util.h>

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory.h>

AudioOutputManager *AudioOutputManager::_instance = nullptr;

AudioOutputManager &AudioOutputManager::createInstance(IAudioGenerator *audioGenerator)
{
    if (AudioOutputManager::_instance == nullptr)
    {
        AudioOutputManager::_instance = new AudioOutputManager(audioGenerator);
    }

    return *AudioOutputManager::_instance;
}

AudioOutputManager &AudioOutputManager::getInstance()
{
    return *AudioOutputManager::_instance;
}

void AudioOutputManager::destroyInstance()
{
    delete AudioOutputManager::_instance;

    AudioOutputManager::_instance = nullptr;
}

AudioOutputManager::AudioOutputManager(IAudioGenerator *audioGenerator)
    : priBuf(new float[4096], new float[4096]),
      priBuffCurrent(priBuf),
      _audioGenerator(audioGenerator)
{
    currentOut = nullptr;
    stales = 0;

    //init samples
    outr = new float[this->_audioGenerator->BufferSize()];
    outl = new float[this->_audioGenerator->BufferSize()];
    memset(outl, 0, this->_audioGenerator->BufferSizeInBytes());
    memset(outr, 0, this->_audioGenerator->BufferSizeInBytes());
}

AudioOutputManager::~AudioOutputManager()
{
    delete[] priBuf._left;
    delete[] priBuf._right;
    delete[] outr;
    delete[] outl;
}

//perform a cheap linear interpolation for resampling
//This will result in some distortion at frame boundries
//returns number of samples produced
static size_t resample(float *dest,
                       const float *src,
                       float s_in,
                       float s_out,
                       size_t elms)
{
    auto out_elms = static_cast<size_t>(elms * s_out / s_in);
    float r_pos = 0.0f;

    for (size_t i = 0; i < out_elms; ++i, r_pos += s_in / s_out)
    {
        dest[i] = interpolate(src, elms, r_pos);
    }

    return out_elms;
}

/* Sequence of a tick
 * 1) Lets remove old/stale samples
 * 2) Apply appliciable midi events
 * 3) Lets see if we need to generate samples
 * 4) Lets generate some
 * 5) Goto 2 if more are needed
 * 6) Lets return those samples to the primary and secondary outputs
 * 7) Lets wait for another tick
 */
const Stereo<float *> AudioOutputManager::NextSample(unsigned int frameSize)
{
    removeStaleSmps();
    unsigned int i = 0;
    while (frameSize > storedSmps())
    {
        this->_audioGenerator->Lock();
        MidiInputManager::Instance()
            .Flush(i * this->_audioGenerator->BufferSize(), (i + 1) * this->_audioGenerator->BufferSize());
        this->_audioGenerator->AudioOut(outl, outr);
        this->_audioGenerator->Unlock();

        const unsigned int s_out = currentOut->SampleRate();
        const unsigned int s_sys = this->_audioGenerator->SampleRate();

        if (s_out != s_sys)
        { //we need to resample
            const size_t stepsLeft = resample(
                priBuffCurrent._left,
                outl, s_sys, s_out,
                this->_audioGenerator->BufferSize());

            const size_t stepsRight = resample(
                priBuffCurrent._right,
                outr, s_sys, s_out,
                this->_audioGenerator->BufferSize());

            priBuffCurrent._left += stepsLeft;
            priBuffCurrent._right += stepsRight;
        }
        else
        { //just copy the samples
            memcpy(priBuffCurrent._left, outl, this->_audioGenerator->BufferSizeInBytes());
            memcpy(priBuffCurrent._right, outr, this->_audioGenerator->BufferSizeInBytes());
            priBuffCurrent._left += this->_audioGenerator->BufferSize();
            priBuffCurrent._right += this->_audioGenerator->BufferSize();
        }
        //allow wave file to syphon off stream
        //        wave->push(Stereo<float *>(outl, outr), synth->buffersize);

        i++;
    }
    stales = frameSize;
    return priBuf;
}

AudioOutput *AudioOutputManager::GetAudioOutput(std::string const &name)
{
    return dynamic_cast<AudioOutput *>(EngineManager::Instance().GetEngine(name));
}

bool AudioOutputManager::SetSink(std::string const &name)
{
    AudioOutput *sink = GetAudioOutput(name);

    if (sink == nullptr)
    {
        return false;
    }

    if (currentOut != nullptr)
    {
        currentOut->SetAudioEnabled(false);
    }

    currentOut = sink;
    currentOut->SetAudioEnabled(true);

    //Keep system in a valid state (aka with a running driver)
    if (!currentOut->IsAudioEnabled())
    {
        currentOut = GetAudioOutput("NULL");
        currentOut->SetAudioEnabled(true);

        return false;
    }

    return true;
}

std::string AudioOutputManager::GetSink() const
{
    if (currentOut)
    {
        return currentOut->_name;
    }

    std::cerr << "BUG: No current output in AudioOutputManager " << __LINE__ << std::endl;

    return "ERROR";
}

void AudioOutputManager::removeStaleSmps()
{
    if (!stales)
    {
        return;
    }

    assert(storedSmps() >= stales);

    const unsigned int leftover = storedSmps() - stales;

    //leftover samples [seen at very low latencies]
    if (leftover)
    {
        memmove(priBuf._left, priBuffCurrent._left - leftover, leftover * sizeof(float));
        memmove(priBuf._right, priBuffCurrent._right - leftover, leftover * sizeof(float));
        priBuffCurrent._left = priBuf._left + leftover;
        priBuffCurrent._right = priBuf._right + leftover;
    }
    else
    {
        priBuffCurrent = priBuf;
    }

    stales = 0;
}
