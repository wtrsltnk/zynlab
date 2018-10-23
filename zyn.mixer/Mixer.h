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

#include "Instrument.h"
#include "Meter.h"
#include "Microtonal.h"
#include <pthread.h>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/IPresetsSerializer.h>
#include <zyn.common/globals.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.synth/Controller.h>
#include <zyn.synth/ifftwrapper.h>

/** It sends Midi Messages to Instruments, receives samples from instruments,
 *  process them with system/insertion effects and mix them */
class Mixer : public IMixer, public IAudioGenerator
{
public:
    /** Constructor TODO make private*/
    Mixer(SystemSettings *synth_, IBankManager *bank_);
    /** Destructor*/
    virtual ~Mixer();

    virtual IBankManager* GetBankManager();
    virtual IMeter *GetMeter();

    /**Saves all settings to a XML file
         * @return 0 for ok or <0 if there is an error*/
    int saveXML(const char *filename);


    /**loads all settings from a XML file
         * @return 0 for ok or -1 if there is an error*/
    int loadXML(const char *filename);
    void applyparameters(bool lockmutex = true);

    /**get all data to a newly allocated array (used for VST)
         * @return the datasize*/
    int getalldata(char **data);
    /**put all data from the *data array to zynaddsubfx parameters (used for VST)*/
    void putalldata(char *data, int size);

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

    void ShutUp();
    int shutup;

    /**Audio Output*/
    virtual void AudioOut(float *outl, float *outr);

    virtual SystemSettings *GetSettings();
    virtual IFFTwrapper *GetFFT();

    void partonoff(int npart, int what);

    virtual int GetChannelCount() const;
    virtual Instrument *GetChannel(int index);
    virtual void EnableChannel(int index, bool enabled);

    //parameters
    unsigned char Pvolume{};
    unsigned char Pkeyshift{};
    unsigned char Psysefxvol[NUM_SYS_EFX][NUM_MIXER_CHANNELS]{};
    unsigned char Psysefxsend[NUM_SYS_EFX][NUM_SYS_EFX]{};

    //parameters control
    void setPvolume(unsigned char Pvolume_);
    void setPkeyshift(unsigned char Pkeyshift_);
    void setPsysefxvol(int Ppart, int Pefx, unsigned char Pvol);
    virtual unsigned char GetSystemEffectSend(int Pefxfrom, int Pefxto);
    virtual void SetSystemEffectSend(int Pefxfrom, int Pefxto, unsigned char Pvol);

    EffectManager sysefx[NUM_SYS_EFX]; //system
    EffectManager insefx[NUM_INS_EFX]; //insertion
                                              //      void swapcopyeffects(int what,int type,int neff1,int neff2);

    //part that's apply the insertion effect; -1 to disable
    short int Pinsparts[NUM_INS_EFX]{};

    Meter _meter;
    Controller ctl;
    bool swaplr; //if L and R are swapped

    //other objects
    Microtonal microtonal;

    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);
    void Defaults();

private:
    Instrument _instruments[NUM_MIXER_CHANNELS];
    pthread_mutex_t mutex{};
    IBankManager *bank;
    IFFTwrapper *fft;
    SystemSettings *_settings;

    float volume{};
    float sysefxvol[NUM_SYS_EFX][NUM_MIXER_CHANNELS]{};
    float sysefxsend[NUM_SYS_EFX][NUM_SYS_EFX]{};
    int keyshift{};

    //information relevent to generating plugin audio samples
    float *bufl;
    float *bufr;
    off_t off;
    size_t smps;
};

#endif // MIXER_H
