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

    void noteout(
        float *outl,
        float *outr);

    WavData *wavData = nullptr;
    size_t wavProgress = 0;
    bool done = true;
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

    void ApplyParameters();

    // Synth settings
    virtual unsigned int SampleRate() const;
    virtual unsigned int BufferSize() const;
    virtual unsigned int BufferSizeInBytes() const;
    virtual float BufferSizeFloat() const;

    // Mutex
    virtual void Lock();
    virtual void Unlock();
    virtual std::mutex &Mutex();

    //Midi IN
    virtual void NoteOn(
        unsigned char chan,
        unsigned char note,
        unsigned char velocity);

    virtual void NoteOff(
        unsigned char chan,
        unsigned char note);

    virtual void PolyphonicAftertouch(
        unsigned char chan,
        unsigned char note,
        unsigned char velocity);

    virtual void SetController(
        unsigned char chan,
        int type,
        int par);

    virtual void SetProgram(
        unsigned char chan,
        unsigned int pgm);

    virtual void PreviewNote(
        unsigned int channel,
        unsigned int note,
        unsigned int lengthInMs,
        unsigned int velocity = 100);

    virtual INoteSource *GetNoteSource() const;

    virtual void SetNoteSource(
        INoteSource *source);

    void ShutUp();

    /**Audio Output*/
    virtual void AudioOut(
        float *outl,
        float *outr);

    void EnableTrack(
        int npart,
        int what);

    virtual unsigned int GetTrackCount() const;

    virtual Track *GetTrack(
        int index);

    virtual void EnableTrack(
        int index,
        bool enabled);

    //parameters control
    virtual unsigned char GetVolume() const;

    virtual void SetVolume(
        unsigned char Pvolume_);

    void SetKeyShift(
        unsigned char Pkeyshift_);

    virtual unsigned char GetSystemEffectVolume(
        int Ppart,
        int Pefx);

    virtual void SetSystemEffectVolume(
        int Ppart,
        int Pefx,
        unsigned char Pvol);

    virtual unsigned char GetSystemSendEffectVolume(
        int Pefxfrom,
        int Pefxto);

    virtual void SetSystemSendEffectVolume(
        int Pefxfrom,
        int Pefxto,
        unsigned char Pvol);

    virtual short int GetTrackIndexForInsertEffect(
        int fx);

    virtual void SetTrackIndexForInsertEffect(
        int fx,
        short int trackIndex);

    virtual int GetSystemEffectType(
        int fx);

    virtual void SetSystemEffectType(
        int fx,
        int type);

    virtual const char *GetSystemEffectName(
        int fx);

    virtual int GetInsertEffectType(
        int fx);

    virtual void SetInsertEffectType(
        int fx,
        int type);

    virtual const char *GetInsertEffectName(
        int fx);

    void InitPresets();

    void Defaults();

    void PreviewSample(
        std::string const &filename);

    //parameters
    unsigned char Pvolume = 0;
    unsigned char Pkeyshift = 0;
    unsigned char Psysefxvol[NUM_SYS_EFX][NUM_MIXER_TRACKS];
    unsigned char Psysefxsend[NUM_SYS_EFX][NUM_SYS_EFX];
    short int Psolotrack = 0;

    EffectManager sysefx[NUM_SYS_EFX]; //system
    EffectManager insefx[NUM_INS_EFX]; //insertion

    //part that's apply the insertion effect; -1 to disable
    short int Pinsparts[NUM_INS_EFX];

    Meter meter;
    Controller ctl;

    bool shutup = false;

    //other objects
    Microtonal microtonal;

private:
    SamplePreview _samplePreview;
    std::vector<InstrumentPreview> _instrumentsPreview;
    Track _tracks[NUM_MIXER_TRACKS];
    std::mutex _mutex;
    INoteSource *_noteSource = nullptr;

    float _volume = 0;
    float _sysefxvol[NUM_SYS_EFX][NUM_MIXER_TRACKS];
    float _sysefxsend[NUM_SYS_EFX][NUM_SYS_EFX];
    int _keyshift = 0;

    std::unique_ptr<float> _tmpmixl;
    std::unique_ptr<float> _tmpmixr;
};

#endif // MIXER_H
