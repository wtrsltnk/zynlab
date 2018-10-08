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
#include <zyn.common/IPresetsSerializer.h>
#include <zyn.common/globals.h>
#include <zyn.synth/Controller.h>
#include <zyn.synth/ifftwrapper.h>
#include <zyn.fx/EffectMgr.h>

typedef enum {
    MUTEX_TRYLOCK,
    MUTEX_LOCK,
    MUTEX_UNLOCK
} lockset;

/** It sends Midi Messages to Instruments, receives samples from instruments,
 *  process them with system/insertion effects and mix them */
class Mixer : public IMixer
{
public:
    /** Constructor TODO make private*/
    Mixer(SystemSettings *synth_, IBankManager *bank_);
    /** Destructor*/
    virtual ~Mixer();

    virtual IBankManager* GetBankManager();

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
    /**Audio Output (for callback mode). This allows the program to be controled by an external program*/
    virtual void GetAudioOutSamples(size_t nsamples, unsigned samplerate, float *outl, float *outr);

    void partonoff(int npart, int what);

    virtual int GetInstrumentCount();
    virtual Instrument *GetInstrument(int index);

    //parameters
    unsigned char Pvolume{};
    unsigned char Pkeyshift{};
    unsigned char Psysefxvol[NUM_SYS_EFX][NUM_MIDI_PARTS]{};
    unsigned char Psysefxsend[NUM_SYS_EFX][NUM_SYS_EFX]{};

    //parameters control
    void setPvolume(unsigned char Pvolume_);
    void setPkeyshift(unsigned char Pkeyshift_);
    void setPsysefxvol(int Ppart, int Pefx, unsigned char Pvol);
    void setPsysefxsend(int Pefxfrom, int Pefxto, unsigned char Pvol);

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

    IFFTwrapper *fft;

    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);
    void Defaults();

private:
    Instrument part[NUM_MIDI_PARTS];
    pthread_mutex_t mutex{};
    IBankManager *bank;
    float volume{};
    float sysefxvol[NUM_SYS_EFX][NUM_MIDI_PARTS]{};
    float sysefxsend[NUM_SYS_EFX][NUM_SYS_EFX]{};
    int keyshift{};

    //information relevent to generating plugin audio samples
    float *bufl;
    float *bufr;
    off_t off;
    size_t smps;
};

#endif // MIXER_H
