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
#include <list> // For the monomemnotes list.
#include <mutex>
#include <zyn.common/ILibraryManager.h>
#include <zyn.common/circularbuffer.h>
#include <zyn.common/globals.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.synth/Controller.h>

class ADnoteParameters;
class SUBnoteParameters;
class PADnoteParameters;
class SampleNoteParameters;
class SynthNote;
class XMLWrapper;

enum NoteStatus
{
    KEY_OFF,
    KEY_PLAYING,
    KEY_RELASED_AND_SUSTAINED,
    KEY_RELASED
};

struct TrackNotes
{
    NoteStatus status = NoteStatus::KEY_OFF;
    int note = 0; // if there is no note playing, the "note"=-1
    int itemsplaying = 0;
    struct
    {
        SynthNote *adnote = nullptr;
        SynthNote *subnote = nullptr;
        SynthNote *padnote = nullptr;
        SynthNote *smplnote = nullptr;
        int sendtoparteffect = 0;
    } instumentNotes[NUM_TRACK_INSTRUMENTS];
    int time = 0;
};

// the Track's instrument
class Instrument
{
public:
    ADnoteParameters *adpars = nullptr;
    SUBnoteParameters *subpars = nullptr;
    PADnoteParameters *padpars = nullptr;
    SampleNoteParameters *smplpars = nullptr;
    unsigned char Penabled, Pmuted, Pminkey, Pmaxkey;
    unsigned char Pname[TRACK_MAX_NAME_LEN];
    unsigned char Padenabled, Psubenabled, Ppadenabled, Psmplenabled;
    unsigned char Psendtoparteffect;
    unsigned char padding[3];
};

/** Track implementation*/
class Track : public WrappedPresets
{
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
    void AllNotesOff(); // panic
    void SetController(unsigned int type, int par);
    void RelaseSustainedKeys(); // this is called when the sustain pedal is relased

    virtual void ComputeInstrumentSamples(); // compute Track output

    void ApplyParameters();

    void Cleanup(bool final = false);
    int GetActiveNotes();

    // the Track's instruments
    Instrument Instruments[NUM_TRACK_INSTRUMENTS];

    // Track parameters
    void setkeylimit(unsigned char Pkeylimit);
    void setkititemstatus(int kititem, int Penabled_);

    unsigned char Penabled = 0;
    unsigned char Pvolume = 0; /**<Track volume*/
    void SetVolume(unsigned char Pvolume);
    unsigned char Ppanning = 0; // Track panning
    void setPpanning(unsigned char Ppanning);
    unsigned char Pminkey = 0;   /**<the minimum key that the Track receives noteon messages*/
    unsigned char Pmaxkey = 0;   // the maximum key that the Track receives noteon messages
    unsigned char Pkeyshift = 0; // Track keyshift
    unsigned char Prcvchn = 0;   // from what midi channel it receive commnads
    unsigned char Pvelsns = 0;   // velocity sensing (amplitude velocity scale)
    unsigned char Pveloffs = 0;  // velocity offset
    unsigned char Pnoteon = 0;   // if the Track receives NoteOn messages
    unsigned char Pkitmode = 0;  // if the kitmode is enabled
    unsigned char Pdrummode = 0; // if all keys are mapped and the system is 12tET (used for drums)

    unsigned char Ppolymode = 0;   // Track mode - 0=monophonic , 1=polyphonic
    unsigned char Plegatomode = 0; // 0=normal, 1=legato
    unsigned char Pkeylimit = 0;   // how many keys are alowed to be played same time (0=off), the older will be relased

    unsigned char Pname[TRACK_MAX_NAME_LEN + 1]; // name of the instrument
    struct
    { // instrument additional information
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
    float *partoutl = nullptr; // Left channel output of the Track
    float *partoutr = nullptr; // Right channel output of the Track

    float *partfxinputl[NUM_TRACK_EFX + 1]; // Left and right signal that pass thru part effects;
    float *partfxinputr[NUM_TRACK_EFX + 1]; // partfxinput l/r [NUM_PART_EFX] is for "no effect" buffer

    float volume = 0;
    float oldvolumel = 0;
    float oldvolumer = 0; // this is applied by Master
    float panning = 0;    // this is applied by Master, too

    Controller ctl;

    EffectManager partefx[NUM_TRACK_EFX];    // insertion part effects (they are part of the instrument)
    unsigned char Pefxroute[NUM_TRACK_EFX];  // how the effect's output is routed(to next effect/to out)
    unsigned char Pefxbypass[NUM_TRACK_EFX]; // if the effects are bypassed

    int lastnote = 0;

    struct
    {
        std::string libraryPath;
        std::string instrumentName;
    } loadedInstrument;

    void RelaseAllKeys(); // this is called on AllNotesOff controller
private:
    void RunNote(unsigned int k);
    void KillNotePos(unsigned int pos);
    void RelaseNotePos(unsigned int pos);
    void MonoMemRenote(); // MonoMem stuff.

    int _killallnotes; // is set to 1 if I want to kill all notes

    unsigned int _lastpos = 0, _lastposb = 0; // To keep track of previously used pos and posb.
    bool _lastlegatomodevalid = false;        // To keep track of previous legatomodevalid.

    // MonoMem stuff
    std::list<unsigned char> _monomemnotes; // A list to remember held notes.
    struct
    {
        unsigned char velocity = 0;
        unsigned char stub[3] = {0, 0, 0};
        int mkeyshift = 0; // I'm not sure masterkeyshift should be remembered.
    } _monomem[256];
    /* 256 is to cover all possible note values.
           monomem[] is used in conjunction with the list to
           store the velocity and masterkeyshift values of a given note (the list only store note values).
           For example 'monomem[note].velocity' would be the velocity value of the note 'note'.*/

    TrackNotes _trackNotes[POLIPHONY];

    float _oldfreq = 0; // this is used for portamento
    IMixer *_mixer = nullptr;
    Microtonal *_microtonal = nullptr;
    std::mutex _instrumentMutex;
    float *_tmpoutr = nullptr;
    float *_tmpoutl = nullptr;
};

#endif // TRACK_H
