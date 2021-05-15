/*
  ZynAddSubFX - a software synthesizer

  Part.cpp - Part implementation
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

#include "Track.h"

#include "Microtonal.h"
#include <zyn.common/IFFTwrapper.h>
#include <zyn.common/PresetsSerializer.h>
#include <zyn.common/Util.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.synth/ADnote.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/PADnote.h>
#include <zyn.synth/PADnoteParams.h>
#include <zyn.synth/SUBnote.h>
#include <zyn.synth/SUBnoteParams.h>
#include <zyn.synth/SampleNote.h>
#include <zyn.synth/SampleNoteParams.h>

Track::Track()
    : _tmpoutr(nullptr),
      _tmpoutl(nullptr),
      _bufferr(SystemSettings::Instance().buffersize * 4),
      _bufferl(SystemSettings::Instance().buffersize * 4)
{}

Track::~Track()
{
    Cleanup(true);
    for (auto &n : Instruments)
    {
        if (n.adpars != nullptr)
            delete (n.adpars);
        if (n.subpars != nullptr)
            delete (n.subpars);
        if (n.padpars != nullptr)
            delete (n.padpars);
        if (n.smplpars != nullptr)
            delete (n.smplpars);
        n.adpars = nullptr;
        n.subpars = nullptr;
        n.padpars = nullptr;
        n.smplpars = nullptr;
        delete[] n.Pname;
    }

    delete[] partoutl;
    delete[] partoutr;
    for (auto &nefx : partefx)
        delete nefx;
    for (int n = 0; n < NUM_TRACK_EFX + 1; ++n)
    {
        delete[] partfxinputl[n];
        delete[] partfxinputr[n];
    }
    delete[] _tmpoutl;
    delete[] _tmpoutr;
}

void Track::Init(IMixer *mixer, Microtonal *microtonal)
{
    _mixer = mixer;
    ctl.Init();
    _microtonal = microtonal;
    _fft = mixer->GetFFT();

    partoutl = new float[SystemSettings::Instance().buffersize];
    partoutr = new float[SystemSettings::Instance().buffersize];

    for (auto &n : Instruments)
    {
        n.Pname = new unsigned char[TRACK_MAX_NAME_LEN];
        n.adpars = new ADnoteParameters(_mixer->GetFFT());
        n.subpars = new SUBnoteParameters();
        n.padpars = new PADnoteParameters(mixer->GetFFT());
        n.smplpars = new SampleNoteParameters();
    }

    //Part's Insertion Effects init
    for (auto &nefx : partefx)
    {
        nefx = new EffectManager();
        nefx->Init(mixer, true);
    }

    for (auto &nefx : Pefxbypass)
    {
        nefx = false;
    }

    for (int n = 0; n < NUM_TRACK_EFX + 1; ++n)
    {
        partfxinputl[n] = new float[SystemSettings::Instance().buffersize];
        partfxinputr[n] = new float[SystemSettings::Instance().buffersize];
    }

    _killallnotes = 0;
    _oldfreq = -1.0f;

    for (auto &i : _trackNotes)
    {
        i.status = KEY_OFF;
        i.note = -1;
        i.itemsplaying = 0;
        for (auto &j : i.instumentNotes)
        {
            j.adnote = nullptr;
            j.subnote = nullptr;
            j.padnote = nullptr;
            j.smplnote = nullptr;
        }
        i.time = 0;
    }
    Cleanup();

    oldvolumel = oldvolumer = 0.5f;
    lastnote = -1;
    _lastpos = 0;                 // lastpos will store previously used NoteOn(...)'s pos.
    _lastlegatomodevalid = false; // To store previous legatomodevalid value.

    Defaults();

    _tmpoutr = new float[SystemSettings::Instance().buffersize];
    _tmpoutl = new float[SystemSettings::Instance().buffersize];
}

void Track::Lock()
{
    _instrumentMutex.lock();
}

bool Track::TryLock()
{
    return _instrumentMutex.try_lock();
}

void Track::Unlock()
{
    _instrumentMutex.unlock();
}

void Track::Defaults()
{
    Penabled = 0;
    Pminkey = 0;
    Pmaxkey = 127;
    Pnoteon = 1;
    Ppolymode = 1;
    Plegatomode = 0;
    setPvolume(96);
    Pkeyshift = 64;
    Prcvchn = 0;
    setPpanning(64);
    Pvelsns = 64;
    Pveloffs = 64;
    Pkeylimit = 15;
    InstrumentDefaults();
    ctl.Defaults();
}

void Track::InstrumentDefaults()
{
    ZEROUNSIGNED(Pname, TRACK_MAX_NAME_LEN);

    info.Ptype = 0;
    ZEROUNSIGNED(info.Pauthor, MAX_INFO_TEXT_SIZE + 1);
    ZEROUNSIGNED(info.Pcomments, MAX_INFO_TEXT_SIZE + 1);

    Pkitmode = 0;
    Pdrummode = 0;

    for (int n = 0; n < NUM_TRACK_INSTRUMENTS; ++n)
    {
        Instruments[n].Penabled = 0;
        Instruments[n].Pmuted = 0;
        Instruments[n].Pminkey = 0;
        Instruments[n].Pmaxkey = 127;
        Instruments[n].Padenabled = 0;
        Instruments[n].Psubenabled = 0;
        Instruments[n].Ppadenabled = 0;
        Instruments[n].Psmplenabled = 0;
        ZEROUNSIGNED(Instruments[n].Pname, TRACK_MAX_NAME_LEN);
        Instruments[n].Psendtoparteffect = 0;
        if (n != 0)
            setkititemstatus(n, 0);
    }
    Instruments[0].Penabled = 1;
    Instruments[0].Padenabled = 1;
    Instruments[0].adpars->Defaults();
    Instruments[0].subpars->Defaults();
    Instruments[0].padpars->Defaults();
    Instruments[0].smplpars->Defaults();

    for (int nefx = 0; nefx < NUM_TRACK_EFX; ++nefx)
    {
        partefx[nefx]->Defaults();
        Pefxroute[nefx] = 0; //route to next effect
    }
}

float Track::ComputePeak(float volume)
{
    auto peak = 1.0e-12f;
    if (Penabled != 0)
    {
        float *outl = partoutl,
              *outr = partoutr;
        for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
        {
            float tmp = std::fabs(outl[i] + outr[i]);
            if (tmp > peak)
            {
                peak = tmp;
            }
        }
        peak *= volume;
    }

    return peak;
}

void Track::ComputePeakLeftAndRight(float volume, float &peakl, float &peakr)
{
    peakl = 1.0e-12f;
    peakr = 1.0e-12f;
    if (Penabled != 0)
    {
        float *outl = partoutl,
              *outr = partoutr;
        for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
        {
            if (outl[i] > peakl)
            {
                peakl = outl[i];
            }
            if (outr[i] > peakr)
            {
                peakr = outr[i];
            }
        }
        peakl *= volume;
        peakr *= volume;
    }
}
/*
 * Cleanup the part
 */
void Track::Cleanup(bool final_)
{
    for (unsigned int k = 0; k < POLIPHONY; ++k)
        KillNotePos(k);

    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        if (partoutl != nullptr) partoutl[i] = final_ ? 0.0f : SystemSettings::Instance().denormalkillbuf[i];
        if (partoutr != nullptr) partoutr[i] = final_ ? 0.0f : SystemSettings::Instance().denormalkillbuf[i];
    }

    ctl.resetall();
    for (auto &nefx : partefx)
    {
        if (nefx != nullptr) nefx->cleanup();
    }

    for (int n = 0; n < NUM_TRACK_EFX + 1; ++n)
    {
        for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
        {
            if (partfxinputl[n] != nullptr) partfxinputl[n][i] = final_ ? 0.0f : SystemSettings::Instance().denormalkillbuf[i];
            if (partfxinputr[n] != nullptr) partfxinputr[n][i] = final_ ? 0.0f : SystemSettings::Instance().denormalkillbuf[i];
        }
    }
}

int Track::GetActiveNotes()
{
    int count = 0;
    for (int i = 0; i < POLIPHONY; i++)
    {
        if (_trackNotes[i].status != NoteStatus::KEY_OFF)
        {
            count++;
        }
    }

    return count;
}

/*
 * Note On Messages
 */
void Track::NoteOn(unsigned char note,
                   unsigned char velocity,
                   int masterkeyshift)
{
    unsigned int i;
    int pos;

    // Legato and MonoMem used vars:
    int posb = POLIPHONY - 1;     // Just a dummy initial value.
    bool legatomodevalid = false; //true when legato mode is determined applicable.
    bool doinglegato = false;     // true when we determined we do a legato note.
    bool ismonofirstnote = false; /*(In Mono/Legato) true when we determined
                  no other notes are held down or sustained.*/
    int lastnotecopy = lastnote;  //Useful after lastnote has been changed.

    if (Pnoteon == 0)
    {
        return;
    }
    if ((note < Pminkey) || (note > Pmaxkey))
    {
        return;
    }

    // MonoMem stuff:
    if (Ppolymode == 0)
    {                                              // If Poly is off
        _monomemnotes.push_back(note);             // Add note to the list.
        _monomem[note].velocity = velocity;        // Store this note's velocity.
        _monomem[note].mkeyshift = masterkeyshift; /* Store masterkeyshift too,
                         I'm not sure why though... */
        if ((_trackNotes[_lastpos].status != KEY_PLAYING) && (_trackNotes[_lastpos].status != KEY_RELASED_AND_SUSTAINED))
            ismonofirstnote = true; // No other keys are held or sustained.
    }
    else
    {
        // Poly mode is On so just make sure the list is empty.
        if (!_monomemnotes.empty())
            _monomemnotes.clear();
    }

    lastnote = note;

    pos = -1;
    for (i = 0; i < POLIPHONY; ++i)
    {
        if (_trackNotes[i].status == KEY_OFF)
        {
            pos = static_cast<int>(i);
            break;
        }
    }

    if ((Plegatomode != 0) && (Pdrummode == 0))
    {
        if (Ppolymode != 0)
        {
            fprintf(
                stderr,
                "ZynAddSubFX WARNING: Poly and Legato modes are both On, that should not happen ! ... Disabling Legato mode ! - (Part.cpp::NoteOn(..))\n");
            Plegatomode = 0;
        }
        else
        {
            // Legato mode is on and applicable.
            legatomodevalid = true;
            if ((!ismonofirstnote) && (_lastlegatomodevalid))
            {
                // At least one other key is held or sustained, and the
                // previous note was played while in valid legato mode.
                doinglegato = true;                 // So we'll do a legato note.
                pos = static_cast<int>(_lastpos);   // A legato note uses same pos as previous..
                posb = static_cast<int>(_lastposb); // .. same goes for posb.
            }
            else
            {
                // Legato mode is valid, but this is only a first note.
                for (i = 0; i < POLIPHONY; ++i)
                {
                    if ((_trackNotes[i].status == KEY_PLAYING) || (_trackNotes[i].status == KEY_RELASED_AND_SUSTAINED))
                    {
                        RelaseNotePos(i);
                    }
                }

                // Set posb
                posb = (pos + 1) % POLIPHONY; //We really want it (if the following fails)
                for (i = 0; i < POLIPHONY; ++i)
                {
                    if ((_trackNotes[i].status == KEY_OFF) && (pos != static_cast<int>(i)))
                    {
                        posb = static_cast<int>(i);
                        break;
                    }
                }
            }
            _lastposb = static_cast<unsigned int>(posb); // Keep a trace of used posb
        }
    }
    else if (Ppolymode == 0)
    {
        // Legato mode is either off or non-applicable.
        //if the mode is 'mono' turn off all other notes
        for (i = 0; i < POLIPHONY; ++i)
        {
            if (_trackNotes[i].status == KEY_PLAYING)
            {
                RelaseNotePos(i);
            }
        }
        RelaseSustainedKeys();
    }
    _lastlegatomodevalid = legatomodevalid;

    if (pos == -1)
    {
        //test
        fprintf(stderr, "%s", "NOTES TOO MANY (> POLIPHONY) - (Part.cpp::NoteOn(..))\n");
    }
    else
    {
        //start the note
        _trackNotes[pos].status = KEY_PLAYING;
        _trackNotes[pos].note = note;
        if (legatomodevalid)
        {
            _trackNotes[posb].status = KEY_PLAYING;
            _trackNotes[posb].note = note;
        }

        //this computes the velocity sensing of the part
        float vel = VelF(velocity / 127.0f, Pvelsns);

        //compute the velocity offset
        vel += (Pveloffs - 64.0f) / 64.0f;
        if (vel < 0.0f)
            vel = 0.0f;
        else if (vel > 1.0f)
            vel = 1.0f;

        //compute the keyshift
        auto partkeyshift = static_cast<int>(Pkeyshift - 64);
        int keyshift = masterkeyshift + partkeyshift;

        //initialise note frequency
        float notebasefreq;
        if (Pdrummode == 0)
        {
            notebasefreq = _microtonal->getnotefreq(note, keyshift);
            if (notebasefreq < 0.0f)
                return; //the key is no mapped
        }
        else
        {
            notebasefreq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
        }

        //Portamento
        if (_oldfreq < 1.0f)
        {
            _oldfreq = notebasefreq; //this is only the first note is played
        }

        // For Mono/Legato: Force Portamento Off on first
        // notes. That means it is required that the previous note is
        // still held down or sustained for the Portamento to activate
        // (that's like Legato).
        int portamento = 0;
        if ((Ppolymode != 0) || (!ismonofirstnote))
        {
            // I added a third argument to the
            // ctl.initportamento(...) function to be able
            // to tell it if we're doing a legato note.
            portamento = ctl.initportamento(_oldfreq, notebasefreq, doinglegato);
        }

        if (portamento != 0)
        {
            ctl.portamento.noteusing = pos;
        }
        _oldfreq = notebasefreq;

        _lastpos = static_cast<unsigned int>(pos); // Keep a trace of used pos.

        if (doinglegato)
        {
            // Do Legato note
            if (Pkitmode == 0)
            { // "normal mode" legato note
                if ((Instruments[0].Padenabled != 0) && (_trackNotes[pos].instumentNotes[0].adnote != nullptr) && (_trackNotes[posb].instumentNotes[0].adnote != nullptr))
                {
                    _trackNotes[pos].instumentNotes[0].adnote->legatonote(notebasefreq, vel, portamento, note, true); //'true' is to tell it it's being called from here.
                    _trackNotes[posb].instumentNotes[0].adnote->legatonote(notebasefreq, vel, portamento, note, true);
                }

                if ((Instruments[0].Psubenabled != 0) && (_trackNotes[pos].instumentNotes[0].subnote != nullptr) && (_trackNotes[posb].instumentNotes[0].subnote != nullptr))
                {
                    _trackNotes[pos].instumentNotes[0].subnote->legatonote(notebasefreq, vel, portamento, note, true);
                    _trackNotes[posb].instumentNotes[0].subnote->legatonote(notebasefreq, vel, portamento, note, true);
                }

                if ((Instruments[0].Ppadenabled != 0) && (_trackNotes[pos].instumentNotes[0].padnote != nullptr) && (_trackNotes[posb].instumentNotes[0].padnote != nullptr))
                {
                    _trackNotes[pos].instumentNotes[0].padnote->legatonote(notebasefreq, vel, portamento, note, true);
                    _trackNotes[posb].instumentNotes[0].padnote->legatonote(notebasefreq, vel, portamento, note, true);
                }

                if ((Instruments[0].Psmplenabled != 0) && (_trackNotes[pos].instumentNotes[0].smplnote != nullptr) && (_trackNotes[posb].instumentNotes[0].smplnote != nullptr))
                {
                    _trackNotes[pos].instumentNotes[0].smplnote->legatonote(notebasefreq, vel, portamento, note, true);
                    _trackNotes[posb].instumentNotes[0].smplnote->legatonote(notebasefreq, vel, portamento, note, true);
                }
            }
            else
            { // "kit mode" legato note
                int ci = 0;
                for (auto &item : Instruments)
                {
                    if (item.Pmuted != 0)
                    {
                        continue;
                    }

                    if ((note < item.Pminkey) || (note > item.Pmaxkey))
                    {
                        continue;
                    }

                    if ((lastnotecopy < item.Pminkey) || (lastnotecopy > item.Pmaxkey))
                    {
                        continue; // We will not perform legato across 2 key regions.
                    }

                    //if this parameter is 127 for "unprocessed"
                    _trackNotes[pos].instumentNotes[ci].sendtoparteffect = (item.Psendtoparteffect < NUM_TRACK_EFX ? item.Psendtoparteffect : NUM_TRACK_EFX);
                    _trackNotes[posb].instumentNotes[ci].sendtoparteffect = (item.Psendtoparteffect < NUM_TRACK_EFX ? item.Psendtoparteffect : NUM_TRACK_EFX);

                    if ((item.Padenabled != 0) && (item.adpars != nullptr) && (_trackNotes[pos].instumentNotes[ci].adnote != nullptr) && (_trackNotes[posb].instumentNotes[ci].adnote != nullptr))
                    {
                        _trackNotes[pos].instumentNotes[ci].adnote->legatonote(notebasefreq, vel, portamento, note, true);
                        _trackNotes[posb].instumentNotes[ci].adnote->legatonote(notebasefreq, vel, portamento, note, true);
                    }
                    if ((item.Psubenabled != 0) && (item.subpars != nullptr) && (_trackNotes[pos].instumentNotes[ci].subnote != nullptr) && (_trackNotes[posb].instumentNotes[ci].subnote != nullptr))
                    {
                        _trackNotes[pos].instumentNotes[ci].subnote->legatonote(notebasefreq, vel, portamento, note, true);
                        _trackNotes[posb].instumentNotes[ci].subnote->legatonote(notebasefreq, vel, portamento, note, true);
                    }
                    if ((item.Psmplenabled != 0) && (item.smplpars != nullptr) && (_trackNotes[pos].instumentNotes[ci].smplnote != nullptr) && (_trackNotes[posb].instumentNotes[ci].smplnote != nullptr))
                    {
                        _trackNotes[pos].instumentNotes[ci].smplnote->legatonote(notebasefreq, vel, portamento, note, true);
                        _trackNotes[posb].instumentNotes[ci].smplnote->legatonote(notebasefreq, vel, portamento, note, true);
                    }
                    if ((item.Ppadenabled != 0) && (item.padpars != nullptr) && (_trackNotes[pos].instumentNotes[ci].padnote != nullptr) && (_trackNotes[posb].instumentNotes[ci].padnote != nullptr))
                    {
                        _trackNotes[pos].instumentNotes[ci].padnote->legatonote(notebasefreq, vel, portamento, note, true);
                        _trackNotes[posb].instumentNotes[ci].padnote->legatonote(notebasefreq, vel, portamento, note, true);
                    }

                    if ((item.adpars != nullptr) || (item.subpars != nullptr) || (item.smplpars != nullptr) || (item.padpars != nullptr))
                    {
                        ci++;
                        if (((item.Padenabled != 0) || (item.Psubenabled != 0) || (item.Psmplenabled != 0) || (item.Ppadenabled != 0)) && (Pkitmode == 2))
                        {
                            break;
                        }
                    }
                }
                if (ci == 0)
                {
                    // No legato were performed at all, so pretend nothing happened:
                    _monomemnotes.pop_back(); // Remove last note from the list.
                    lastnote = lastnotecopy;  // Set lastnote back to previous value.
                }
            }
            return; // Ok, Legato note done, return.
        }

        _trackNotes[pos].itemsplaying = 0;
        if (legatomodevalid)
        {
            _trackNotes[posb].itemsplaying = 0;
        }

        if (Pkitmode == 0)
        { //init the notes for the "normal mode"
            _trackNotes[pos].instumentNotes[0].sendtoparteffect = 0;

            if (Instruments[0].Padenabled != 0)
            {
                _trackNotes[pos].instumentNotes[0].adnote = new ADnote(Instruments[0].adpars, &ctl, notebasefreq, vel, portamento, note, false);
            }

            if (Instruments[0].Psubenabled != 0)
            {
                _trackNotes[pos].instumentNotes[0].subnote = new SUBnote(Instruments[0].subpars, &ctl, notebasefreq, vel, portamento, note, false);
            }

            if (Instruments[0].Ppadenabled != 0)
            {
                _trackNotes[pos].instumentNotes[0].padnote = new PADnote(Instruments[0].padpars, &ctl, notebasefreq, vel, portamento, note, false);
            }

            if (Instruments[0].Psmplenabled != 0)
            {
                _trackNotes[pos].instumentNotes[0].smplnote = new SampleNote(Instruments[0].smplpars, &ctl, notebasefreq, vel, note);
            }

            if ((Instruments[0].Padenabled != 0) || (Instruments[0].Psubenabled != 0) || (Instruments[0].Psmplenabled != 0) || (Instruments[0].Ppadenabled != 0))
            {
                _trackNotes[pos].itemsplaying++;
            }

            // Spawn another note (but silent) if legatomodevalid==true
            if (legatomodevalid)
            {
                _trackNotes[posb].instumentNotes[0].sendtoparteffect = 0;
                if (Instruments[0].Padenabled != 0)
                {
                    //true for silent.
                    _trackNotes[posb].instumentNotes[0].adnote = new ADnote(Instruments[0].adpars, &ctl, notebasefreq, vel, portamento, note, true);
                }

                if (Instruments[0].Psubenabled != 0)
                {
                    _trackNotes[posb].instumentNotes[0].subnote = new SUBnote(Instruments[0].subpars, &ctl, notebasefreq, vel, portamento, note, true);
                }

                if (Instruments[0].Ppadenabled != 0)
                {
                    _trackNotes[posb].instumentNotes[0].padnote = new PADnote(Instruments[0].padpars, &ctl, notebasefreq, vel, portamento, note, true);
                }

                if (Instruments[0].Psmplenabled != 0)
                {
                    _trackNotes[posb].instumentNotes[0].smplnote = new SampleNote(Instruments[0].smplpars, &ctl, notebasefreq, vel, note);
                }

                if ((Instruments[0].Padenabled != 0) || (Instruments[0].Psubenabled != 0) || (Instruments[0].Psmplenabled != 0) || (Instruments[0].Ppadenabled != 0))
                {
                    _trackNotes[posb].itemsplaying++;
                }
            }
        }
        else //init the notes for the "kit mode"
        {
            for (auto &item : Instruments)
            {
                if (item.Pmuted != 0)
                {
                    continue;
                }
                if ((note < item.Pminkey) || (note > item.Pmaxkey))
                {
                    continue;
                }

                int ci = _trackNotes[pos].itemsplaying; //ci=current item

                //if this parameter is 127 for "unprocessed"
                _trackNotes[pos].instumentNotes[ci].sendtoparteffect = (item.Psendtoparteffect < NUM_TRACK_EFX ? item.Psendtoparteffect : NUM_TRACK_EFX);

                if ((item.adpars != nullptr) && ((item.Padenabled) != 0))
                {
                    _trackNotes[pos].instumentNotes[ci].adnote = new ADnote(item.adpars, &ctl, notebasefreq, vel, portamento, note, false);
                }
                if ((item.subpars != nullptr) && ((item.Psubenabled) != 0))
                {
                    _trackNotes[pos].instumentNotes[ci].subnote = new SUBnote(item.subpars, &ctl, notebasefreq, vel, portamento, note, false);
                }
                if ((item.padpars != nullptr) && ((item.Ppadenabled) != 0))
                {
                    _trackNotes[pos].instumentNotes[ci].padnote = new PADnote(item.padpars, &ctl, notebasefreq, vel, portamento, note, false);
                }
                if ((item.smplpars != nullptr) && ((item.Psmplenabled) != 0))
                {
                    _trackNotes[pos].instumentNotes[ci].smplnote = new SampleNote(item.smplpars, &ctl, notebasefreq, vel, note);
                }

                // Spawn another note (but silent) if legatomodevalid==true
                if (legatomodevalid)
                {
                    //if this parameter is 127 for "unprocessed"
                    _trackNotes[posb].instumentNotes[ci].sendtoparteffect = (item.Psendtoparteffect < NUM_TRACK_EFX ? item.Psendtoparteffect : NUM_TRACK_EFX);

                    if ((item.adpars != nullptr) && ((item.Padenabled) != 0))
                    {
                        //true for silent.
                        _trackNotes[posb].instumentNotes[ci].adnote = new ADnote(item.adpars, &ctl, notebasefreq, vel, portamento, note, true);
                    }
                    if ((item.subpars != nullptr) && ((item.Psubenabled) != 0))
                    {
                        _trackNotes[posb].instumentNotes[ci].subnote = new SUBnote(item.subpars, &ctl, notebasefreq, vel, portamento, note, true);
                    }
                    if ((item.padpars != nullptr) && ((item.Ppadenabled) != 0))
                    {
                        _trackNotes[posb].instumentNotes[ci].padnote = new PADnote(item.padpars, &ctl, notebasefreq, vel, portamento, note, true);
                    }
                    if ((item.smplpars != nullptr) && ((item.Psmplenabled) != 0))
                    {
                        _trackNotes[posb].instumentNotes[ci].smplnote = new SampleNote(item.smplpars, &ctl, notebasefreq, vel, note);
                    }

                    if ((item.adpars != nullptr) || (item.subpars != nullptr) || (item.smplpars != nullptr))
                    {
                        _trackNotes[posb].itemsplaying++;
                    }
                }

                if ((item.adpars != nullptr) || (item.subpars != nullptr))
                {
                    _trackNotes[pos].itemsplaying++;
                    if (((item.Padenabled != 0) || (item.Psubenabled != 0) || (item.Psmplenabled != 0) || (item.Ppadenabled != 0)) && (Pkitmode == 2))
                    {
                        break;
                    }
                }
            }
        }
    }

    //this only relase the keys if there is maximum number of keys allowed
    setkeylimit(Pkeylimit);
}

/*
 * Note Off Messages
 */
void Track::NoteOff(unsigned char note) //relase the key
{
    // This note is released, so we remove it from the list.
    if (!_monomemnotes.empty())
    {
        _monomemnotes.remove(note);
    }

    for (int i = POLIPHONY - 1; i >= 0; i--) //first note in, is first out if there are same note multiple times
    {
        if ((_trackNotes[i].status == KEY_PLAYING) && (_trackNotes[i].note == note))
        {
            if (ctl.sustain.sustain == 0)
            { //the sustain pedal is not pushed
                if ((Ppolymode == 0) && (!_monomemnotes.empty()))
                {
                    MonoMemRenote(); // To play most recent still held note.
                }
                else
                {
                    RelaseNotePos(static_cast<unsigned int>(i));
                }
            }
            else //the sustain pedal is pushed
            {
                _trackNotes[i].status = KEY_RELASED_AND_SUSTAINED;
            }
        }
    }
}

void Track::PolyphonicAftertouch(unsigned char note,
                                 unsigned char velocity,
                                 int masterkeyshift)
{
    (void)masterkeyshift;
    if (!Pnoteon || (note < Pminkey) || (note > Pmaxkey))
    {
        return;
    }
    if (Pdrummode)
    {
        return;
    }

    // MonoMem stuff:
    if (!Ppolymode) // if Poly is off
    {
        _monomem[note].velocity = velocity; // Store this note's velocity.
    }

    for (auto &i : _trackNotes)
    {
        if ((i.note == note) && (i.status == KEY_PLAYING))
        {
            /* update velocity */
            // compute the velocity offset
            float vel = VelF(velocity / 127.0f, Pvelsns) + (Pveloffs - 64.0f) / 64.0f;
            vel = (vel < 0.0f) ? 0.0f : vel;
            vel = (vel > 1.0f) ? 1.0f : vel;

            if (!Pkitmode)
            { // "normal mode"
                if (Instruments[0].Padenabled && i.instumentNotes[0].adnote)
                    i.instumentNotes[0].adnote->setVelocity(vel);
                if (Instruments[0].Psubenabled && i.instumentNotes[0].subnote)
                    i.instumentNotes[0].subnote->setVelocity(vel);
                if (Instruments[0].Ppadenabled && i.instumentNotes[0].padnote)
                    i.instumentNotes[0].padnote->setVelocity(vel);
                if (Instruments[0].Psmplenabled && i.instumentNotes[0].smplnote)
                    i.instumentNotes[0].smplnote->setVelocity(vel);
            }
            else // "kit mode"
            {
                for (int item = 0; item < NUM_TRACK_INSTRUMENTS; ++item)
                {
                    if (Instruments[item].Pmuted)
                        continue;
                    if ((note < Instruments[item].Pminkey) || (note > Instruments[item].Pmaxkey))
                        continue;

                    if (Instruments[item].Padenabled && i.instumentNotes[item].adnote)
                        i.instumentNotes[item].adnote->setVelocity(vel);
                    if (Instruments[item].Psubenabled && i.instumentNotes[item].subnote)
                        i.instumentNotes[item].subnote->setVelocity(vel);
                    if (Instruments[item].Ppadenabled && i.instumentNotes[item].padnote)
                        i.instumentNotes[item].padnote->setVelocity(vel);
                    if (Instruments[item].Psmplenabled && i.instumentNotes[item].smplnote)
                        i.instumentNotes[item].smplnote->setVelocity(vel);
                }
            }
        }
    }
}

/*
 * Controllers
 */
void Track::SetController(unsigned int type, int par)
{
    switch (type)
    {
        case C_pitchwheel:
        {
            ctl.setpitchwheel(par);
            break;
        }
        case C_expression:
        {
            ctl.setexpression(par);
            setPvolume(Pvolume); //update the volume
            break;
        }
        case C_portamento:
        {
            ctl.setportamento(par);
            break;
        }
        case C_panning:
        {
            ctl.setpanning(par);
            setPpanning(Ppanning); //update the panning
            break;
        }
        case C_filtercutoff:
        {
            ctl.setfiltercutoff(par);
            break;
        }
        case C_filterq:
        {
            ctl.setfilterq(par);
            break;
        }
        case C_bandwidth:
        {
            ctl.setbandwidth(par);
            break;
        }
        case C_modwheel:
        {
            ctl.setmodwheel(par);
            break;
        }
        case C_fmamp:
        {
            ctl.setfmamp(par);
            break;
        }
        case C_volume:
        {
            ctl.setvolume(par);
            if (ctl.volume.receive != 0)
            {
                volume = ctl.volume.volume;
            }
            else
            {
                setPvolume(Pvolume);
            }
            break;
        }
        case C_sustain:
        {
            ctl.setsustain(par);
            if (ctl.sustain.sustain == 0)
            {
                RelaseSustainedKeys();
            }
            break;
        }
        case C_allsoundsoff:
        {
            AllNotesOff(); //Panic
            break;
        }
        case C_resetallcontrollers:
        {
            ctl.resetall();
            RelaseSustainedKeys();
            if (ctl.volume.receive != 0)
            {
                volume = ctl.volume.volume;
            }
            else
            {
                setPvolume(Pvolume);
            }
            setPvolume(Pvolume);   //update the volume
            setPpanning(Ppanning); //update the panning

            for (auto &item : Instruments)
            {
                if (item.adpars == nullptr)
                    continue;
                item.adpars->Reson->sendcontroller(C_resonance_center, 1.0f);

                item.adpars->Reson->sendcontroller(C_resonance_bandwidth, 1.0f);
            }
            //more update to add here if I add controllers
            break;
        }
        case C_allnotesoff:
        {
            RelaseAllKeys();
            break;
        }
        case C_resonance_center:
        {
            ctl.setresonancecenter(par);
            for (auto &item : Instruments)
            {
                if (item.adpars == nullptr)
                    continue;
                item.adpars->Reson->sendcontroller(C_resonance_center,
                                                   ctl.resonancecenter.relcenter);
            }
            break;
        }
        case C_resonance_bandwidth:
        {
            ctl.setresonancebw(par);
            Instruments[0].adpars->Reson->sendcontroller(C_resonance_bandwidth, ctl.resonancebandwidth.relbw);
            break;
        }
    }
}
/*
 * Relase the sustained keys
 */

void Track::RelaseSustainedKeys()
{
    // Let's call MonoMemRenote() on some conditions:
    if ((Ppolymode == 0) && (!_monomemnotes.empty()))
    {
        if (_monomemnotes.back() != lastnote) // Sustain controller manipulation would cause repeated same note respawn without this check.
        {
            MonoMemRenote(); // To play most recent still held note.
        }
    }

    for (unsigned int i = 0; i < POLIPHONY; ++i)
    {
        if (_trackNotes[i].status == KEY_RELASED_AND_SUSTAINED)
        {
            RelaseNotePos(i);
        }
    }
}

/*
 * Relase all keys
 */

void Track::RelaseAllKeys()
{
    for (unsigned int i = 0; i < POLIPHONY; ++i)
    {
        if ((_trackNotes[i].status != KEY_RELASED) && (_trackNotes[i].status != KEY_OFF)) //thanks to Frank Neumann
        {
            RelaseNotePos(i);
        }
    }
}

// Call NoteOn(...) with the most recent still held key as new note
// (Made for Mono/Legato).
void Track::MonoMemRenote()
{
    unsigned char mmrtempnote = _monomemnotes.back(); // Last list element.
    _monomemnotes.pop_back();                         // We remove it, will be added again in NoteOn(...).
    if (Pnoteon == 0)
    {
        RelaseNotePos(_lastpos);
    }
    else
    {
        NoteOn(mmrtempnote, _monomem[mmrtempnote].velocity, _monomem[mmrtempnote].mkeyshift);
    }
}

/*
 * Release note at position
 */
void Track::RelaseNotePos(unsigned int pos)
{
    for (auto &j : _trackNotes[pos].instumentNotes)
    {
        if (j.adnote != nullptr)
            if (j.adnote)
                j.adnote->relasekey();

        if (j.subnote != nullptr)
            if (j.subnote != nullptr)
                j.subnote->relasekey();

        if (j.padnote != nullptr)
            if (j.padnote)
                j.padnote->relasekey();

        if (j.smplnote != nullptr)
            if (j.smplnote)
                j.smplnote->relasekey();
    }
    _trackNotes[pos].status = KEY_RELASED;
}

/*
 * Kill note at position
 */
void Track::KillNotePos(unsigned int pos)
{
    _trackNotes[pos].status = KEY_OFF;
    _trackNotes[pos].note = -1;
    _trackNotes[pos].time = 0;
    _trackNotes[pos].itemsplaying = 0;

    for (auto &j : _trackNotes[pos].instumentNotes)
    {
        if (j.adnote != nullptr)
        {
            delete (j.adnote);
            j.adnote = nullptr;
        }
        if (j.subnote != nullptr)
        {
            delete (j.subnote);
            j.subnote = nullptr;
        }
        if (j.padnote != nullptr)
        {
            delete (j.padnote);
            j.padnote = nullptr;
        }
        if (j.smplnote != nullptr)
        {
            delete (j.smplnote);
            j.smplnote = nullptr;
        }
    }
    if (pos == static_cast<unsigned int>(ctl.portamento.noteusing))
    {
        ctl.portamento.noteusing = -1;
        ctl.portamento.used = 0;
    }
}

/*
 * Set Part's key limit
 */
void Track::setkeylimit(unsigned char Pkeylimit)
{
    this->Pkeylimit = Pkeylimit;
    int keylimit = Pkeylimit;
    if (keylimit == 0)
    {
        keylimit = POLIPHONY - 5;
    }

    //release old keys if the number of notes>keylimit
    if (Ppolymode != 0)
    {
        int notecount = 0;
        for (auto &i : _trackNotes)
        {
            if ((i.status == KEY_PLAYING) || (i.status == KEY_RELASED_AND_SUSTAINED))
            {
                notecount++;
            }
        }
        int oldestnotepos = -1;
        if (notecount > keylimit) //find out the oldest note
        {
            for (int i = 0; i < POLIPHONY; ++i)
            {
                int maxtime = 0;
                if (((_trackNotes[i].status == KEY_PLAYING) || (_trackNotes[i].status == KEY_RELASED_AND_SUSTAINED)) && (_trackNotes[i].time > maxtime))
                {
                    maxtime = _trackNotes[i].time;
                    oldestnotepos = i;
                }
            }
        }
        if (oldestnotepos != -1)
        {
            RelaseNotePos(static_cast<unsigned int>(oldestnotepos));
        }
    }
}

/*
 * Prepare all notes to be turned off
 */
void Track::AllNotesOff()
{
    _killallnotes = 1;
}

void Track::RunNote(unsigned int k)
{
    unsigned noteplay = 0;

    for (int item = 0; item < _trackNotes[k].itemsplaying; ++item)
    {
        int sendcurrenttofx = _trackNotes[k].instumentNotes[item].sendtoparteffect;

        for (unsigned type = 0; type < 4; ++type)
        {
            //Select a note
            SynthNote **note = nullptr;
            if (type == 0)
            {
                note = &_trackNotes[k].instumentNotes[item].adnote;
            }
            else if (type == 1)
            {
                note = &_trackNotes[k].instumentNotes[item].subnote;
            }
            else if (type == 2)
            {
                note = &_trackNotes[k].instumentNotes[item].padnote;
            }
            else if (type == 3)
            {
                note = &_trackNotes[k].instumentNotes[item].smplnote;
            }

            //Process if it exists
            if (!(*note))
            {
                continue;
            }
            noteplay++;

            (*note)->noteout(&_tmpoutl[0], &_tmpoutr[0]);

            if ((*note)->finished())
            {
                delete (*note);
                (*note) = nullptr;
            }
            for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
            { //add the note to part(mix)
                partfxinputl[sendcurrenttofx][i] += _tmpoutl[i];
                partfxinputr[sendcurrenttofx][i] += _tmpoutr[i];
            }
        }
    }

    //Kill note if there is no synth on that note
    if (noteplay == 0)
    {
        KillNotePos(k);
    }
}

/*
 * Compute Part samples and store them in the partoutl[] and partoutr[]
 */
void Track::ComputeInstrumentSamples()
{
    std::lock_guard<std::mutex> guard(_instrumentMutex);

    for (unsigned nefx = 0; nefx < NUM_TRACK_EFX + 1; ++nefx)
    {
        for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
        {
            partfxinputl[nefx][i] = 0.0f;
            partfxinputr[nefx][i] = 0.0f;
        }
    }

    for (unsigned k = 0; k < POLIPHONY; ++k)
    {
        if (_trackNotes[k].status == KEY_OFF)
        {
            continue;
        }
        _trackNotes[k].time++;
        //get the sampledata of the note and kill it if it's finished
        RunNote(k);
    }

    //Apply part's effects and mix them
    for (int nefx = 0; nefx < NUM_TRACK_EFX; ++nefx)
    {
        if (!Pefxbypass[nefx])
        {
            partefx[nefx]->out(partfxinputl[nefx], partfxinputr[nefx]);
            if (Pefxroute[nefx] == 2)
                for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
                {
                    partfxinputl[nefx + 1][i] += partefx[nefx]->_effectOutL[i];
                    partfxinputr[nefx + 1][i] += partefx[nefx]->_effectOutR[i];
                }
        }
        int routeto = ((Pefxroute[nefx] == 0) ? nefx + 1 : NUM_TRACK_EFX);
        for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
        {
            partfxinputl[routeto][i] += partfxinputl[nefx][i];
            partfxinputr[routeto][i] += partfxinputr[nefx][i];
        }
    }
    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        partoutl[i] = partfxinputl[NUM_TRACK_EFX][i];
        partoutr[i] = partfxinputr[NUM_TRACK_EFX][i];
    }

    //Kill All Notes if killallnotes!=0
    if (_killallnotes != 0)
    {
        for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
        {
            float tmp = (SystemSettings::Instance().buffersize_f - i) / SystemSettings::Instance().buffersize_f;
            partoutl[i] *= tmp;
            partoutr[i] *= tmp;
        }
        for (unsigned int k = 0; k < POLIPHONY; ++k)
        {
            KillNotePos(k);
        }
        _killallnotes = 0;
        for (auto &nefx : partefx)
        {
            nefx->cleanup();
        }
    }
    ctl.updateportamento();

    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; i++)
    {
        _bufferr.put(partoutr[i]);
        _bufferl.put(partoutl[i]);
    }
}

/*
 * Parameter control
 */
void Track::setPvolume(unsigned char Pvolume_)
{
    Pvolume = Pvolume_;
    volume = dB2rap((Pvolume - 96.0f) / 96.0f * 40.0f) * ctl.expression.relvolume;
}

void Track::setPpanning(unsigned char Ppanning_)
{
    Ppanning = Ppanning_;
    panning = Ppanning / 127.0f + ctl.panning.pan;
    if (panning < 0.0f)
    {
        panning = 0.0f;
    }
    else if (panning > 1.0f)
    {
        panning = 1.0f;
    }
}

/*
 * Enable or disable a kit item
 */
void Track::setkititemstatus(int kititem, int Penabled_)
{
    if ((kititem == 0) || (kititem >= NUM_TRACK_INSTRUMENTS))
    {
        return; //nonexistent kit item and the first kit item is always enabled
    }
    Instruments[kititem].Penabled = static_cast<unsigned char>(Penabled_);

    bool resetallnotes = false;
    if (Penabled_ == 0)
    {
        resetallnotes = true;
        Instruments[kititem].Pname[0] = '\0';
    }

    Instruments[kititem].adpars->Defaults();
    Instruments[kititem].subpars->Defaults();
    Instruments[kititem].padpars->Defaults();
    Instruments[kititem].smplpars->Defaults();

    if (resetallnotes)
    {
        for (unsigned int k = 0; k < POLIPHONY; ++k)
        {
            KillNotePos(k);
        }
    }
}

void Track::InitPresets()
{
    Preset infoPreset("INFO");
    infoPreset.AddPresetAsString("name", Pname, MAX_INFO_TEXT_SIZE);
    infoPreset.AddPresetAsString("author", info.Pauthor, MAX_INFO_TEXT_SIZE);
    infoPreset.AddPresetAsString("comments", info.Pcomments, MAX_INFO_TEXT_SIZE);
    infoPreset.AddPreset("type", &info.Ptype);
    AddContainer(infoPreset);

    Preset instrumentKit("INSTRUMENT_KIT");
    instrumentKit.AddPreset("kit_mode", &Pkitmode);
    instrumentKit.AddPresetAsBool("drum_mode", &Pdrummode);

    for (int i = 0; i < NUM_TRACK_INSTRUMENTS; ++i)
    {
        Preset instrumentKitItem("INSTRUMENT_KIT_ITEM", i);
        instrumentKitItem.AddPresetAsBool("enabled", &Instruments[i].Penabled);

        instrumentKitItem.AddPresetAsString("name", Instruments[i].Pname, MAX_INFO_TEXT_SIZE);

        instrumentKitItem.AddPresetAsBool("muted", &Instruments[i].Pmuted);
        instrumentKitItem.AddPreset("min_key", &Instruments[i].Pminkey);
        instrumentKitItem.AddPreset("max_key", &Instruments[i].Pmaxkey);

        instrumentKitItem.AddPreset("send_to_instrument_effect", &Instruments[i].Psendtoparteffect);

        instrumentKitItem.AddPresetAsBool("add_enabled", &(Instruments[i].Padenabled));
        Instruments[i].adpars->InitPresets();
        instrumentKitItem.AddContainer(Preset("ADD_SYNTH_PARAMETERS", *Instruments[i].adpars));

        instrumentKitItem.AddPresetAsBool("sub_enabled", &(Instruments[i].Psubenabled));
        Instruments[i].subpars->InitPresets();
        instrumentKitItem.AddContainer(Preset("SUB_SYNTH_PARAMETERS", *Instruments[i].subpars));

        instrumentKitItem.AddPresetAsBool("pad_enabled", &(Instruments[i].Ppadenabled));
        Instruments[i].padpars->InitPresets();
        instrumentKitItem.AddContainer(Preset("PAD_SYNTH_PARAMETERS", *Instruments[i].padpars));

        instrumentKit.AddContainer(instrumentKitItem);
    }
    AddContainer(instrumentKit);

    Preset instrumentEffects("INSTRUMENT_EFFECTS");
    for (int nefx = 0; nefx < NUM_TRACK_EFX; ++nefx)
    {
        Preset instrumentEffect("INSTRUMENT_EFFECT", nefx);
        {
            partefx[nefx]->InitPresets();
            instrumentEffect.AddContainer(Preset("EFFECT", *partefx[nefx]));

            instrumentEffect.AddPreset("route", &Pefxroute[nefx]);
            instrumentEffect.AddPresetAsBool("bypass", &Pefxbypass[nefx]);
        }
        instrumentEffects.AddContainer(instrumentEffect);
    }
    AddContainer(instrumentEffects);
}

void Track::ApplyParameters(bool lockmutex)
{
    for (auto &n : Instruments)
    {
        if ((n.padpars != nullptr) && (n.Ppadenabled != 0))
        {
            n.padpars->ApplyParameters(lockmutex ? _mixer : nullptr);
        }
    }
}
