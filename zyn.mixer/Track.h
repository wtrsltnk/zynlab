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

#ifndef TRACK_H
#define TRACK_H

#define MAX_INFO_TEXT_SIZE 1000

#include "Microtonal.h"
#include <zyn.common/ILibraryManager.h>
#include <zyn.common/globals.h>
#include <zyn.common/circularbuffer.h>
#include <zyn.synth/Controller.h>

#include <list> // For the monomemnotes list.
#include <pthread.h>

class EffectManager;
class ADnoteParameters;
class SUBnoteParameters;
class PADnoteParameters;
class SampleNoteParameters;
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

struct TrackNotes
{
    NoteStatus status;
    int note; //if there is no note playing, the "note"=-1
    int itemsplaying;
    struct
    {
        SynthNote *adnote;
        SynthNote *subnote;
        SynthNote *padnote;
        SynthNote *smplnote;
        int sendtoparteffect;
    } instumentNotes[NUM_TRACK_INSTRUMENTS];
    int time;
};

//the Track's instrument
class Instrument
{
public:
    ADnoteParameters *adpars;
    SUBnoteParameters *subpars;
    PADnoteParameters *padpars;
    SampleNoteParameters *smplpars;
    unsigned char Penabled, Pmuted, Pminkey, Pmaxkey;
    unsigned char *Pname;
    unsigned char Padenabled, Psubenabled, Ppadenabled, Psmplenabled;
    unsigned char Psendtoparteffect;
    unsigned char padding[3];
};

/** Track implementation*/
class Track : public WrappedPresets
{
    float *_tmpoutr;
    float *_tmpoutl;

public:
    Track();
    virtual ~Track();

    circular_buffer<float> _bufferr;
    circular_buffer<float> _bufferl;

    void Init(IMixer *mixer, Microtonal *microtonal_);

    // Mutex
    virtual void Lock();
    virtual bool TryLock();
    virtual void Unlock();

    // Midi commands implemented
    void NoteOn(unsigned char note, unsigned char velocity, int masterkeyshift);
    void NoteOff(unsigned char note);
    void PolyphonicAftertouch(unsigned char note, unsigned char velocity, int masterkeyshift);
    void AllNotesOff(); //panic
    void SetController(unsigned int type, int par);
    void RelaseSustainedKeys(); //this is called when the sustain pedal is relased

    virtual void ComputeInstrumentSamples(); // compute Track output

    void ApplyParameters(bool lockmutex = true);

    void Cleanup(bool final = false);
    int GetActiveNotes();

    //the Track's instruments
    Instrument Instruments[NUM_TRACK_INSTRUMENTS];

    // Track parameters
    void setkeylimit(unsigned char Pkeylimit);
    void setkititemstatus(int kititem, int Penabled_);

    unsigned char Penabled;
    unsigned char Pvolume; /**<Track volume*/
    void setPvolume(unsigned char Pvolume);
    unsigned char Ppanning; //Track panning
    void setPpanning(unsigned char Ppanning);
    unsigned char Pminkey;   /**<the minimum key that the Track receives noteon messages*/
    unsigned char Pmaxkey;   //the maximum key that the Track receives noteon messages
    unsigned char Pkeyshift; //Track keyshift
    unsigned char Prcvchn;   //from what midi channel it receive commnads
    unsigned char Pvelsns;   //velocity sensing (amplitude velocity scale)
    unsigned char Pveloffs;  //velocity offset
    unsigned char Pnoteon;   //if the Track receives NoteOn messages
    unsigned char Pkitmode;  //if the kitmode is enabled
    unsigned char Pdrummode; //if all keys are mapped and the system is 12tET (used for drums)

    unsigned char Ppolymode;   //Track mode - 0=monophonic , 1=polyphonic
    unsigned char Plegatomode; // 0=normal, 1=legato
    unsigned char Pkeylimit;   //how many keys are alowed to be played same time (0=off), the older will be relased

    unsigned char Pname[TRACK_MAX_NAME_LEN + 1]; //name of the instrument
    struct
    { //instrument additional information
        unsigned char Ptype;
        unsigned char Pauthor[MAX_INFO_TEXT_SIZE + 1];
        unsigned char Pcomments[MAX_INFO_TEXT_SIZE + 1];
    } info;

public:
    void InitPresets();

    void Defaults();
    void InstrumentDefaults();

    float ComputePeak(float volume);
    void ComputePeakLeftAndRight(float volume, float &peakl, float &peakr);

public:
    float *partoutl; //Left channel output of the Track
    float *partoutr; //Right channel output of the Track

    float *partfxinputl[NUM_TRACK_EFX + 1]; //Left and right signal that pass thru part effects;
    float *partfxinputr[NUM_TRACK_EFX + 1]; //partfxinput l/r [NUM_PART_EFX] is for "no effect" buffer

    float volume;
    float oldvolumel;
    float oldvolumer; //this is applied by Master
    float panning;    //this is applied by Master, too

    Controller ctl;

    EffectManager *partefx[NUM_TRACK_EFX];   //insertion part effects (they are part of the instrument)
    unsigned char Pefxroute[NUM_TRACK_EFX];  //how the effect's output is routed(to next effect/to out)
    unsigned char Pefxbypass[NUM_TRACK_EFX]; //if the effects are bypassed

    int lastnote;

    struct
    {
        std::string libraryPath;
        std::string instrumentName;
    } loadedInstrument;

    void RelaseAllKeys(); //this is called on AllNotesOff controller
private:
    void RunNote(unsigned int k);
    void KillNotePos(unsigned int pos);
    void RelaseNotePos(unsigned int pos);
    void MonoMemRenote(); // MonoMem stuff.

    int _killallnotes; //is set to 1 if I want to kill all notes

    unsigned int _lastpos, _lastposb; // To keep track of previously used pos and posb.
    bool _lastlegatomodevalid;        // To keep track of previous legatomodevalid.

    // MonoMem stuff
    std::list<unsigned char> _monomemnotes; // A list to remember held notes.
    struct
    {
        unsigned char velocity;
        unsigned char stub[3];
        int mkeyshift; // I'm not sure masterkeyshift should be remembered.
    } _monomem[256];
    /* 256 is to cover all possible note values.
           monomem[] is used in conjunction with the list to
           store the velocity and masterkeyshift values of a given note (the list only store note values).
           For example 'monomem[note].velocity' would be the velocity value of the note 'note'.*/

    TrackNotes _trackNotes[POLIPHONY];

    float _oldfreq; //this is used for portamento
    IMixer *_mixer;
    Microtonal *_microtonal;
    IFFTwrapper *_fft;
    pthread_mutex_t _instrumentMutex;
};

#endif // TRACK_H
