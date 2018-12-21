/*
  ZynAddSubFX - a software synthesizer

  Part.h - Part implementation
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

#ifndef CHANNEL_H
#define CHANNEL_H

#define MAX_INFO_TEXT_SIZE 1000

#include "Microtonal.h"
#include <zyn.common/globals.h>
#include <zyn.synth/Controller.h>

#include <list> // For the monomemnotes list.
#include <pthread.h>

class EffectManager;
class ADnoteParameters;
class SUBnoteParameters;
class PADnoteParameters;
class SynthNote;
class XMLWrapper;
class IFFTwrapper;

enum NoteStatus
{
    KEY_OFF,
    KEY_PLAYING,
    KEY_RELASED_AND_SUSTAINED,
    KEY_RELASED
};

struct ChannelNotes
{
    NoteStatus status;
    int note; //if there is no note playing, the "note"=-1
    int itemsplaying;
    struct
    {
        SynthNote *adnote;
        SynthNote *subnote;
        SynthNote *padnote;
        int sendtoparteffect;
    } instumentNotes[NUM_CHANNEL_INSTRUMENTS];
    int time;
};

//the Channel's instrument
class Instrument
{
public:
    unsigned char Penabled, Pmuted, Pminkey, Pmaxkey;
    unsigned char *Pname;
    unsigned char Padenabled, Psubenabled, Ppadenabled;
    unsigned char Psendtoparteffect;
    ADnoteParameters *adpars;
    SUBnoteParameters *subpars;
    PADnoteParameters *padpars;
};

/** Channel implementation*/
class Channel
{
    float *_tmpoutr;
    float *_tmpoutl;
public:
    /**Constructor*/
    Channel();
    /**Destructor*/
    virtual ~Channel();

    /**Ìnit()
         * @param fft_ Pointer to the mixer
         * @param microtonal_ Pointer to the microtonal object*/
    void Init(IMixer *mixer, Microtonal *microtonal_);

    // Mutex
    void Lock();
    bool TryLock();
    void Unlock();

    // Midi commands implemented
    void NoteOn(unsigned char note, unsigned char velocity, int masterkeyshift);
    void NoteOff(unsigned char note);
    void PolyphonicAftertouch(unsigned char note, unsigned char velocity, int masterkeyshift);
    void AllNotesOff(); //panic
    void SetController(unsigned int type, int par);
    void RelaseSustainedKeys(); //this is called when the sustain pedal is relased
    void RelaseAllKeys();       //this is called on AllNotesOff controller

    void ComputeInstrumentSamples(); // compute channel output

    //saves the instrument settings to a XML file
    //returns 0 for ok or <0 if there is an error
    int saveXML(const char *filename);
    int loadXMLinstrument(const char *filename);

    void ApplyParameters(bool lockmutex = true);

    void Cleanup(bool final = false);

    //the Channel's instruments
    Instrument _instruments[NUM_CHANNEL_INSTRUMENTS];

    //Channel parameters
    void setkeylimit(unsigned char Pkeylimit);
    void setkititemstatus(int kititem, int Penabled_);

    unsigned char Penabled; /**<if the Channel is enabled*/
    unsigned char Pvolume;  /**<Channel volume*/
    unsigned char Pminkey;  /**<the minimum key that the Channel receives noteon messages*/
    unsigned char Pmaxkey;  //the maximum key that the Channel receives noteon messages
    void setPvolume(unsigned char Pvolume);
    unsigned char Pkeyshift; //Channel keyshift
    unsigned char Prcvchn;   //from what midi channel it receive commnads
    unsigned char Ppanning;  //Channel panning
    void setPpanning(unsigned char Ppanning);
    unsigned char Pvelsns;   //velocity sensing (amplitude velocity scale)
    unsigned char Pveloffs;  //velocity offset
    unsigned char Pnoteon;   //if the Channel receives NoteOn messages
    unsigned char Pkitmode;  //if the kitmode is enabled
    unsigned char Pdrummode; //if all keys are mapped and the system is 12tET (used for drums)

    unsigned char Ppolymode;   //Channel mode - 0=monophonic , 1=polyphonic
    unsigned char Plegatomode; // 0=normal, 1=legato
    unsigned char Pkeylimit;   //how many keys are alowed to be played same time (0=off), the older will be relased

    unsigned char *Pname; //name of the instrument
    struct
    { //instrument additional information
        unsigned char Ptype;
        unsigned char Pauthor[MAX_INFO_TEXT_SIZE + 1];
        unsigned char Pcomments[MAX_INFO_TEXT_SIZE + 1];
    } info;

public:
    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);
    void Defaults();

    void SerializeInstrument(IPresetsSerializer *xml);
    void DeserializeInstrument(IPresetsSerializer *xml);
    void InstrumentDefaults();

    float ComputePeak(float volume);
    void ComputePeakLeftAndRight(float volume, float &peakl, float &peakr);

public:
    float *partoutl; //Left channel output of the Channel
    float *partoutr; //Right channel output of the Channel

    float *partfxinputl[NUM_CHANNEL_EFX + 1]; //Left and right signal that pass thru part effects;
    float *partfxinputr[NUM_CHANNEL_EFX + 1]; //partfxinput l/r [NUM_PART_EFX] is for "no effect" buffer

    float volume;
    float oldvolumel;
    float oldvolumer; //this is applied by Master
    float panning;    //this is applied by Master, too

    Controller ctl;

    EffectManager *partefx[NUM_CHANNEL_EFX];  //insertion part effects (they are part of the instrument)
    unsigned char Pefxroute[NUM_CHANNEL_EFX]; //how the effect's output is routed(to next effect/to out)
    bool Pefxbypass[NUM_CHANNEL_EFX];         //if the effects are bypassed

    int lastnote;

private:
    void RunNote(unsigned int k);
    void KillNotePos(unsigned int pos);
    void RelaseNotePos(unsigned int pos);
    void MonoMemRenote(); // MonoMem stuff.

    int killallnotes; //is set to 1 if I want to kill all notes

    unsigned int lastpos, lastposb; // To keep track of previously used pos and posb.
    bool lastlegatomodevalid;         // To keep track of previous legatomodevalid.

    // MonoMem stuff
    std::list<unsigned char> monomemnotes; // A list to remember held notes.
    struct
    {
        unsigned char velocity;
        unsigned char stub[3];
        int mkeyshift; // I'm not sure masterkeyshift should be remembered.
    } monomem[256];
    /* 256 is to cover all possible note values.
           monomem[] is used in conjunction with the list to
           store the velocity and masterkeyshift values of a given note (the list only store note values).
           For example 'monomem[note].velocity' would be the velocity value of the note 'note'.*/

    ChannelNotes _channelNotes[POLIPHONY];

    float oldfreq; //this is used for portamento
    IMixer *_mixer;
    SystemSettings *_synth;
    Microtonal *_microtonal;
    IFFTwrapper *_fft;
    pthread_mutex_t _instrumentMutex;
};

#endif // CHANNEL_H
