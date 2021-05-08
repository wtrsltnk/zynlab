/*
  ZynAddSubFX - a software synthesizer

  Master.h - It sends Midi Messages to Parts, receives samples from parts,
             process them with system/insertion effects and mix them
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef MIXER_H
#define MIXER_H

#include "Meter.h"
#include "Microtonal.h"
#include "Track.h"
#include <chrono>
#include <memory>
#include <mutex>
#include <zyn.common/AudioFile.h>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/IFFTwrapper.h>
#include <zyn.common/IMidiEventHandler.h>
#include <zyn.common/Presets.h>
#include <zyn.common/Stereo.h>
#include <zyn.common/WavData.h>
#include <zyn.common/globals.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.synth/Controller.h>

class SamplePreview
{
public:
    SamplePreview();
    virtual ~SamplePreview();

    WavData *wavData;
    size_t wavProgress;
    bool done;
    void noteout(float *outl, float *outr);
};

typedef std::chrono::milliseconds::rep timestep;

class InstrumentPreview
{
public:
    timestep playUntil;
    unsigned int channel;
    unsigned int note;
    bool done;
};

/** It sends Midi Messages to Instruments, receives samples from instruments,
 *  process them with system/insertion effects and mix them */
class Mixer :
    public IMixer,
    public IAudioGenerator,
    public IMidiEventHandler,
    public WrappedPresets
{
    friend class MixerSerializer;

public:
    Mixer();
    virtual ~Mixer();

    void Init();

    virtual IMeter *GetMeter();

    void ApplyParameters(bool lockmutex = true);

    // Synth settings
    virtual unsigned int SampleRate() const;
    virtual unsigned int BufferSize() const;
    virtual unsigned int BufferSizeInBytes() const;
    virtual float BufferSizeFloat() const;

    // Mutex
    virtual void Lock();
    virtual void Unlock();

    //Midi IN
    virtual void NoteOn(unsigned char chan, unsigned char note, unsigned char velocity);
    virtual void NoteOff(unsigned char chan, unsigned char note);
    virtual void PolyphonicAftertouch(unsigned char chan, unsigned char note, unsigned char velocity);
    virtual void SetController(unsigned char chan, int type, int par);
    virtual void SetProgram(unsigned char chan, unsigned int pgm);

    virtual void PreviewNote(unsigned int channel, unsigned int note, unsigned int length, unsigned int velocity = 100);

    virtual INoteSource *GetNoteSource() const;
    virtual void SetNoteSource(INoteSource *source);

    void ShutUp();
    bool shutup;

    /**Audio Output*/
    virtual void AudioOut(float *outl, float *outr);

    virtual IFFTwrapper *GetFFT();

    void EnableTrack(int npart, int what);

    virtual unsigned int GetTrackCount() const;
    virtual Track *GetTrack(int index);
    virtual void EnableTrack(int index, bool enabled);

    //parameters
    unsigned char Pvolume;
    unsigned char Pkeyshift;
    unsigned char Psysefxvol[NUM_SYS_EFX][NUM_MIXER_TRACKS];
    unsigned char Psysefxsend[NUM_SYS_EFX][NUM_SYS_EFX];
    short int Psolotrack;

    //parameters control
    void setPvolume(unsigned char Pvolume_);
    void setPkeyshift(unsigned char Pkeyshift_);
    void setPsysefxvol(int Ppart, int Pefx, unsigned char Pvol);
    virtual unsigned char GetSystemEffectSend(int Pefxfrom, int Pefxto);
    virtual void SetSystemEffectSend(int Pefxfrom, int Pefxto, unsigned char Pvol);

    EffectManager sysefx[NUM_SYS_EFX]; //system
    EffectManager insefx[NUM_INS_EFX]; //insertion

    //part that's apply the insertion effect; -1 to disable
    short int Pinsparts[NUM_INS_EFX];

    Meter meter;
    Controller ctl;
    bool swaplr; //if L and R are swapped

    //other objects
    Microtonal microtonal;

    void InitPresets();

    void Defaults();

    void PreviewSample(std::string const &filename);

private:
    SamplePreview _samplePreview;
    std::vector<InstrumentPreview> _instrumentsPreview;
    Track _tracks[NUM_MIXER_TRACKS];
    std::mutex _mutex;
    std::unique_ptr<IFFTwrapper> _fft;
    INoteSource *_noteSource;

    float _volume;
    float _sysefxvol[NUM_SYS_EFX][NUM_MIXER_TRACKS];
    float _sysefxsend[NUM_SYS_EFX][NUM_SYS_EFX];
    int _keyshift;

    std::unique_ptr<float> _tmpmixl;
    std::unique_ptr<float> _tmpmixr;
};

#endif // MIXER_H
