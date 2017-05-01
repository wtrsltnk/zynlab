#include "AudioOutputManager.h"
#include "AudioOutput.h"
#include "Engine.h"
#include "EngineManager.h"
#include "MidiInputManager.h"
#include "WavEngine.h"
#include "zyn.common/Util.h" //for set_realtime()

#include <algorithm>
#include <iostream>
#include <cassert>
#include <memory.h>

using namespace std;

AudioOutputManager *AudioOutputManager::_instance = 0;

AudioOutputManager &AudioOutputManager::createInstance(IMixer* mixer)
{
    if (AudioOutputManager::_instance == 0) AudioOutputManager::_instance = new AudioOutputManager(mixer);
    return *AudioOutputManager::_instance;
}

AudioOutputManager &AudioOutputManager::getInstance()
{
    return *AudioOutputManager::_instance;
}

void AudioOutputManager::destroyInstance()
{
    if (AudioOutputManager::_instance != 0) delete AudioOutputManager::_instance;
    AudioOutputManager::_instance = 0;
}

AudioOutputManager::AudioOutputManager(IMixer* mixer)
    :wave(new WavEngine()),
      priBuf(new float[4096],
             new float[4096]), priBuffCurrent(priBuf),
      mixer(mixer)
{
    currentOut = NULL;
    stales     = 0;

    //init samples
    outr = new float[synth->buffersize];
    outl = new float[synth->buffersize];
    memset(outl, 0, synth->bufferbytes);
    memset(outr, 0, synth->bufferbytes);
}

AudioOutputManager::~AudioOutputManager()
{
    delete wave;
    delete [] priBuf.l;
    delete [] priBuf.r;
    delete [] outr;
    delete [] outl;
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
const Stereo<float *> AudioOutputManager::tick(unsigned int frameSize)
{
    MidiInputManager &midi = MidiInputManager::getInstance();
    //SysEv->execute();
    removeStaleSmps();
    int i=0;
    while(frameSize > storedSmps()) {
        if(!midi.empty()) {
            this->mixer->Lock();
            midi.flush(i*synth->buffersize, (i+1)*synth->buffersize);
            this->mixer->Unlock();
        }
        this->mixer->Lock();
        this->mixer->AudioOut(outl, outr);
        this->mixer->Unlock();
        addSmps(outl, outr);
        i++;
    }
    stales = frameSize;
    return priBuf;
}

AudioOutput *AudioOutputManager::getOut(string name)
{
    return dynamic_cast<AudioOutput *>(EngineManager::getInstance().getEng(name));
}

string AudioOutputManager::getDriver() const
{
    return currentOut->name;
}

bool AudioOutputManager::setSink(string name)
{
    AudioOutput *sink = getOut(name);

    if(!sink)
        return false;

    if(currentOut)
        currentOut->setAudioEn(false);

    currentOut = sink;
    currentOut->setAudioEn(true);

    bool success = currentOut->getAudioEn();

    //Keep system in a valid state (aka with a running driver)
    if(!success)
        (currentOut = getOut("NULL"))->setAudioEn(true);

    return success;
}

string AudioOutputManager::getSink() const
{
    if(currentOut)
        return currentOut->name;
    else {
        cerr << "BUG: No current output in OutMgr " << __LINE__ << endl;
        return "ERROR";
    }
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
    size_t out_elms = elms * s_out / s_in;
    float  r_pos    = 0.0f;
    for(int i = 0; i < (int)out_elms; ++i, r_pos += s_in / s_out)
        dest[i] = interpolate(src, elms, r_pos);

    return out_elms;
}

void AudioOutputManager::addSmps(float *l, float *r)
{
    //allow wave file to syphon off stream
    wave->push(Stereo<float *>(l, r), synth->buffersize);

    const int s_out = currentOut->getSampleRate(),
              s_sys = synth->samplerate;

    if(s_out != s_sys) { //we need to resample
        const size_t steps = resample(priBuffCurrent.l,
                                      l,
                                      s_sys,
                                      s_out,
                                      synth->buffersize);
        resample(priBuffCurrent.r, r, s_sys, s_out, synth->buffersize);

        priBuffCurrent.l += steps;
        priBuffCurrent.r += steps;
    }
    else { //just copy the samples
        memcpy(priBuffCurrent.l, l, synth->bufferbytes);
        memcpy(priBuffCurrent.r, r, synth->bufferbytes);
        priBuffCurrent.l += synth->buffersize;
        priBuffCurrent.r += synth->buffersize;
    }
}

void AudioOutputManager::removeStaleSmps()
{
    if(!stales)
        return;

    const int leftover = storedSmps() - stales;

    assert(leftover > -1);

    //leftover samples [seen at very low latencies]
    if(leftover) {
        memmove(priBuf.l, priBuffCurrent.l - leftover, leftover * sizeof(float));
        memmove(priBuf.r, priBuffCurrent.r - leftover, leftover * sizeof(float));
        priBuffCurrent.l = priBuf.l + leftover;
        priBuffCurrent.r = priBuf.r + leftover;
    }
    else
        priBuffCurrent = priBuf;

    stales = 0;
}
