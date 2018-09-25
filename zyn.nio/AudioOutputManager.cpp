#include "AudioOutputManager.h"
#include "AudioOutput.h"
#include "Engine.h"
#include "EngineManager.h"
#include "MidiInputManager.h"
#include "WavEngine.h"
#include <zyn.common/Util.h> //for set_realtime()

#include <algorithm>
#include <cassert>
#include <iostream>
#include <memory.h>

AudioOutputManager *AudioOutputManager::_instance = nullptr;

AudioOutputManager &AudioOutputManager::createInstance(IMixer *mixer)
{
    if (AudioOutputManager::_instance == nullptr)
    {
        AudioOutputManager::_instance = new AudioOutputManager(mixer);
    }

    return *AudioOutputManager::_instance;
}

AudioOutputManager &AudioOutputManager::getInstance()
{
    return *AudioOutputManager::_instance;
}

void AudioOutputManager::destroyInstance()
{
    if (AudioOutputManager::_instance != nullptr)
    {
        delete AudioOutputManager::_instance;
    }

    AudioOutputManager::_instance = nullptr;
}

AudioOutputManager::AudioOutputManager(IMixer *mixer)
    : priBuf(new float[4096], new float[4096]),
      priBuffCurrent(priBuf),
      mixer(mixer)
{
    currentOut = nullptr;
    stales = 0;

    //init samples
    outr = new float[this->mixer->_synth->buffersize];
    outl = new float[this->mixer->_synth->buffersize];
    memset(outl, 0, this->mixer->_synth->bufferbytes);
    memset(outr, 0, this->mixer->_synth->bufferbytes);
}

AudioOutputManager::~AudioOutputManager()
{
    delete[] priBuf._left;
    delete[] priBuf._right;
    delete[] outr;
    delete[] outl;
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
    MidiInputManager &midi = MidiInputManager::Instance();

    removeStaleSmps();
    unsigned int i = 0;
    while (frameSize > storedSmps())
    {
        if (!midi.Empty())
        {
            this->mixer->Lock();
            midi.Flush(i * this->mixer->_synth->buffersize, (i + 1) * this->mixer->_synth->buffersize);
            this->mixer->Unlock();
        }
        this->mixer->Lock();
        this->mixer->AudioOut(outl, outr);
        this->mixer->Unlock();
        addSmps(outl, outr);

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

//perform a cheap linear interpolation for resampling
//This will result in some distortion at frame boundries
//returns number of samples produced
static size_t resample(float *dest,
                       const float *src,
                       float s_in,
                       float s_out,
                       size_t elms)
{
    size_t out_elms = static_cast<size_t>(elms * s_out / s_in);
    float r_pos = 0.0f;

    for (size_t i = 0; i < out_elms; ++i, r_pos += s_in / s_out)
    {
        dest[i] = interpolate(src, elms, r_pos);
    }

    return out_elms;
}

void AudioOutputManager::addSmps(float *l, float *r)
{
    const unsigned int s_out = currentOut->SampleRate();
    const unsigned int s_sys = this->mixer->_synth->samplerate;

    if (s_out != s_sys)
    { //we need to resample
        const size_t steps = resample(priBuffCurrent._left,
                                      l,
                                      s_sys,
                                      s_out,
                                      this->mixer->_synth->buffersize);
        resample(priBuffCurrent._right, r, s_sys, s_out, this->mixer->_synth->buffersize);

        priBuffCurrent._left += steps;
        priBuffCurrent._right += steps;
    }
    else
    { //just copy the samples
        memcpy(priBuffCurrent._left, l, this->mixer->_synth->bufferbytes);
        memcpy(priBuffCurrent._right, r, this->mixer->_synth->bufferbytes);
        priBuffCurrent._left += this->mixer->_synth->buffersize;
        priBuffCurrent._right += this->mixer->_synth->buffersize;
    }
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
