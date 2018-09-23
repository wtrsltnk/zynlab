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

#include "Instrument.h"
#include "Microtonal.h"
#include <zyn.common/Util.h>
#include <zyn.common/XMLwrapper.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.synth/ADnote.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/PADnote.h>
#include <zyn.synth/PADnoteParams.h>
#include <zyn.synth/SUBnote.h>
#include <zyn.synth/SUBnoteParams.h>
#include <zyn.synth/ifftwrapper.h>

Instrument::Instrument(SystemSettings *synth_, Microtonal *microtonal_, IFFTwrapper *fft_, pthread_mutex_t *mutex_)
    : _synth(synth_), ctl(synth_)
{
    microtonal = microtonal_;
    fft = fft_;
    mutex = mutex_;
    pthread_mutex_init(&load_mutex, nullptr);
    partoutl = new float[this->_synth->buffersize];
    partoutr = new float[this->_synth->buffersize];

    for (auto &n : kit)
    {
        n.Pname = new unsigned char[PART_MAX_NAME_LEN];
        n.adpars = nullptr;
        n.subpars = nullptr;
        n.padpars = nullptr;
    }

    kit[0].adpars = new ADnoteParameters(this->_synth, fft);
    kit[0].subpars = new SUBnoteParameters(this->_synth);
    kit[0].padpars = new PADnoteParameters(this->_synth, fft, mutex);

    //Part's Insertion Effects init
    for (int nefx = 0; nefx < NUM_PART_EFX; ++nefx)
    {
        partefx[nefx] = new EffectManager(true, mutex, this->_synth);
        Pefxbypass[nefx] = false;
    }

    for (int n = 0; n < NUM_PART_EFX + 1; ++n)
    {
        partfxinputl[n] = new float[this->_synth->buffersize];
        partfxinputr[n] = new float[this->_synth->buffersize];
    }

    killallnotes = 0;
    oldfreq = -1.0f;

    for (auto &i : partnote)
    {
        i.status = KEY_OFF;
        i.note = -1;
        i.itemsplaying = 0;
        for (auto & j : i.kititem)
        {
            j.adnote = nullptr;
            j.subnote = nullptr;
            j.padnote = nullptr;
        }
        i.time = 0;
    }
    cleanup();

    Pname = new unsigned char[PART_MAX_NAME_LEN];

    oldvolumel = oldvolumer = 0.5f;
    lastnote = -1;
    lastpos = 0;                 // lastpos will store previously used NoteOn(...)'s pos.
    lastlegatomodevalid = false; // To store previous legatomodevalid value.

    defaults();
}

void Instrument::defaults()
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
    defaultsinstrument();
    ctl.defaults();
}

void Instrument::defaultsinstrument()
{
    ZEROUNSIGNED(Pname, PART_MAX_NAME_LEN);

    info.Ptype = 0;
    ZEROUNSIGNED(info.Pauthor, MAX_INFO_TEXT_SIZE + 1);
    ZEROUNSIGNED(info.Pcomments, MAX_INFO_TEXT_SIZE + 1);

    Pkitmode = 0;
    Pdrummode = 0;

    for (int n = 0; n < NUM_KIT_ITEMS; ++n)
    {
        kit[n].Penabled = 0;
        kit[n].Pmuted = 0;
        kit[n].Pminkey = 0;
        kit[n].Pmaxkey = 127;
        kit[n].Padenabled = 0;
        kit[n].Psubenabled = 0;
        kit[n].Ppadenabled = 0;
        ZEROUNSIGNED(kit[n].Pname, PART_MAX_NAME_LEN);
        kit[n].Psendtoparteffect = 0;
        if (n != 0)
            setkititemstatus(n, 0);
    }
    kit[0].Penabled = 1;
    kit[0].Padenabled = 1;
    kit[0].adpars->defaults();
    kit[0].subpars->defaults();
    kit[0].padpars->defaults();

    for (int nefx = 0; nefx < NUM_PART_EFX; ++nefx)
    {
        partefx[nefx]->defaults();
        Pefxroute[nefx] = 0; //route to next effect
    }
}

/*
 * Cleanup the part
 */
void Instrument::cleanup(bool final_)
{
    for (unsigned int k = 0; k < POLIPHONY; ++k)
        KillNotePos(k);

    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        partoutl[i] = final_ ? 0.0f : this->_synth->denormalkillbuf[i];
        partoutr[i] = final_ ? 0.0f : this->_synth->denormalkillbuf[i];
    }

    ctl.resetall();
    for (auto &nefx : partefx)
        nefx->cleanup();

    for (int n = 0; n < NUM_PART_EFX + 1; ++n)
    {
        for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
        {
            partfxinputl[n][i] = final_ ? 0.0f : this->_synth->denormalkillbuf[i];
            partfxinputr[n][i] = final_ ? 0.0f : this->_synth->denormalkillbuf[i];
        }
    }
}

Instrument::~Instrument()
{
    cleanup(true);
    for (auto &n : kit)
    {
        if (n.adpars != nullptr)
            delete (n.adpars);
        if (n.subpars != nullptr)
            delete (n.subpars);
        if (n.padpars != nullptr)
            delete (n.padpars);
        n.adpars = nullptr;
        n.subpars = nullptr;
        n.padpars = nullptr;
        delete[] n.Pname;
    }

    delete[] Pname;
    delete[] partoutl;
    delete[] partoutr;
    for (auto &nefx : partefx)
        delete nefx;
    for (int n = 0; n < NUM_PART_EFX + 1; ++n)
    {
        delete[] partfxinputl[n];
        delete[] partfxinputr[n];
    }
}

/*
 * Note On Messages
 */
void Instrument::NoteOn(unsigned char note,
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
        return;
    if ((note < Pminkey) || (note > Pmaxkey))
        return;

    // MonoMem stuff:
    if (Ppolymode == 0)
    {                                             // If Poly is off
        monomemnotes.push_back(note);             // Add note to the list.
        monomem[note].velocity = velocity;        // Store this note's velocity.
        monomem[note].mkeyshift = masterkeyshift; /* Store masterkeyshift too,
                         I'm not sure why though... */
        if ((partnote[lastpos].status != KEY_PLAYING) && (partnote[lastpos].status != KEY_RELASED_AND_SUSTAINED))
            ismonofirstnote = true; // No other keys are held or sustained.
    }
    else
    {
        // Poly mode is On so just make sure the list is empty.
        if (not monomemnotes.empty())
            monomemnotes.clear();
    }

    lastnote = note;

    pos = -1;
    for (i = 0; i < POLIPHONY; ++i)
        if (partnote[i].status == KEY_OFF)
        {
            pos = static_cast<int>(i);
            break;
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
            if ((not ismonofirstnote) && (lastlegatomodevalid))
            {
                // At least one other key is held or sustained, and the
                // previous note was played while in valid legato mode.
                doinglegato = true;                // So we'll do a legato note.
                pos = static_cast<int>(lastpos);   // A legato note uses same pos as previous..
                posb = static_cast<int>(lastposb); // .. same goes for posb.
            }
            else
            {
                // Legato mode is valid, but this is only a first note.
                for (i = 0; i < POLIPHONY; ++i)
                    if ((partnote[i].status == KEY_PLAYING) || (partnote[i].status == KEY_RELASED_AND_SUSTAINED))
                        RelaseNotePos(i);

                // Set posb
                posb = (pos + 1) % POLIPHONY; //We really want it (if the following fails)
                for (i = 0; i < POLIPHONY; ++i)
                    if ((partnote[i].status == KEY_OFF) && (pos != static_cast<int>(i)))
                    {
                        posb = static_cast<int>(i);
                        break;
                    }
            }
            lastposb = static_cast<unsigned int>(posb); // Keep a trace of used posb
        }
    }
    else // Legato mode is either off or non-applicable.
        if (Ppolymode == 0)
    { //if the mode is 'mono' turn off all other notes
        for (i = 0; i < POLIPHONY; ++i)
            if (partnote[i].status == KEY_PLAYING)
                RelaseNotePos(i);
        RelaseSustainedKeys();
    }
    lastlegatomodevalid = legatomodevalid;

    if (pos == -1)
        //test
        fprintf(stderr,
                "%s",
                "NOTES TOO MANY (> POLIPHONY) - (Part.cpp::NoteOn(..))\n");
    else
    {
        //start the note
        partnote[pos].status = KEY_PLAYING;
        partnote[pos].note = note;
        if (legatomodevalid)
        {
            partnote[posb].status = KEY_PLAYING;
            partnote[posb].note = note;
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
            notebasefreq = microtonal->getnotefreq(note, keyshift);
            if (notebasefreq < 0.0f)
                return; //the key is no mapped
        }
        else
            notebasefreq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
        ;

        //Portamento
        if (oldfreq < 1.0f)
            oldfreq = notebasefreq; //this is only the first note is played

        // For Mono/Legato: Force Portamento Off on first
        // notes. That means it is required that the previous note is
        // still held down or sustained for the Portamento to activate
        // (that's like Legato).
        int portamento = 0;
        if ((Ppolymode != 0) || (not ismonofirstnote))
            // I added a third argument to the
            // ctl.initportamento(...) function to be able
            // to tell it if we're doing a legato note.
            portamento = ctl.initportamento(oldfreq, notebasefreq, doinglegato);

        if (portamento != 0)
            ctl.portamento.noteusing = pos;
        oldfreq = notebasefreq;

        lastpos = static_cast<unsigned int>(pos); // Keep a trace of used pos.

        if (doinglegato)
        {
            // Do Legato note
            if (Pkitmode == 0)
            { // "normal mode" legato note
                if ((kit[0].Padenabled != 0) && (partnote[pos].kititem[0].adnote != nullptr) && (partnote[posb].kititem[0].adnote != nullptr))
                {
                    partnote[pos].kititem[0].adnote->legatonote(notebasefreq,
                                                                vel,
                                                                portamento,
                                                                note,
                                                                true); //'true' is to tell it it's being called from here.
                    partnote[posb].kititem[0].adnote->legatonote(notebasefreq,
                                                                 vel,
                                                                 portamento,
                                                                 note,
                                                                 true);
                }

                if ((kit[0].Psubenabled != 0) && (partnote[pos].kititem[0].subnote != nullptr) && (partnote[posb].kititem[0].subnote != nullptr))
                {
                    partnote[pos].kititem[0].subnote->legatonote(
                        notebasefreq, vel, portamento, note, true);
                    partnote[posb].kititem[0].subnote->legatonote(
                        notebasefreq, vel, portamento, note, true);
                }

                if ((kit[0].Ppadenabled != 0) && (partnote[pos].kititem[0].padnote != nullptr) && (partnote[posb].kititem[0].padnote != nullptr))
                {
                    partnote[pos].kititem[0].padnote->legatonote(
                        notebasefreq, vel, portamento, note, true);
                    partnote[posb].kititem[0].padnote->legatonote(
                        notebasefreq, vel, portamento, note, true);
                }
            }
            else
            { // "kit mode" legato note
                int ci = 0;
                for (auto &item : kit)
                {
                    if (item.Pmuted != 0)
                        continue;
                    if ((note < item.Pminkey) || (note > item.Pmaxkey))
                        continue;

                    if ((lastnotecopy < item.Pminkey) || (lastnotecopy > item.Pmaxkey))
                        continue; // We will not perform legato across 2 key regions.

                    partnote[pos].kititem[ci].sendtoparteffect =
                        (item.Psendtoparteffect <
                                 NUM_PART_EFX
                             ? item.Psendtoparteffect
                             : NUM_PART_EFX); //if this parameter is 127 for "unprocessed"
                    partnote[posb].kititem[ci].sendtoparteffect =
                        (item.Psendtoparteffect <
                                 NUM_PART_EFX
                             ? item.Psendtoparteffect
                             : NUM_PART_EFX);

                    if ((item.Padenabled != 0) && (item.adpars != nullptr) && (partnote[pos].kititem[ci].adnote != nullptr) && (partnote[posb].kititem[ci].adnote != nullptr))
                    {
                        partnote[pos].kititem[ci].adnote->legatonote(
                            notebasefreq, vel, portamento, note, true);
                        partnote[posb].kititem[ci].adnote->legatonote(
                            notebasefreq, vel, portamento, note, true);
                    }
                    if ((item.Psubenabled != 0) && (item.subpars != nullptr) && (partnote[pos].kititem[ci].subnote != nullptr) && (partnote[posb].kititem[ci].subnote != nullptr))
                    {
                        partnote[pos].kititem[ci].subnote->legatonote(
                            notebasefreq, vel, portamento, note, true);
                        partnote[posb].kititem[ci].subnote->legatonote(
                            notebasefreq, vel, portamento, note, true);
                    }
                    if ((item.Ppadenabled != 0) && (item.padpars != nullptr) && (partnote[pos].kititem[ci].padnote != nullptr) && (partnote[posb].kititem[ci].padnote != nullptr))
                    {
                        partnote[pos].kititem[ci].padnote->legatonote(
                            notebasefreq, vel, portamento, note, true);
                        partnote[posb].kititem[ci].padnote->legatonote(
                            notebasefreq, vel, portamento, note, true);
                    }

                    if ((item.adpars != nullptr) || (item.subpars != nullptr) || (item.padpars != nullptr))
                    {
                        ci++;
                        if (((item.Padenabled != 0) || (item.Psubenabled != 0) || (item.Ppadenabled != 0)) && (Pkitmode == 2))
                            break;
                    }
                }
                if (ci == 0)
                {
                    // No legato were performed at all, so pretend nothing happened:
                    monomemnotes.pop_back(); // Remove last note from the list.
                    lastnote = lastnotecopy; // Set lastnote back to previous value.
                }
            }
            return; // Ok, Legato note done, return.
        }

        partnote[pos].itemsplaying = 0;
        if (legatomodevalid)
            partnote[posb].itemsplaying = 0;

        if (Pkitmode == 0)
        { //init the notes for the "normal mode"
            partnote[pos].kititem[0].sendtoparteffect = 0;
            if (kit[0].Padenabled != 0)
                partnote[pos].kititem[0].adnote = new ADnote(kit[0].adpars,
                                                             &ctl,
                                                             this->_synth,
                                                             notebasefreq,
                                                             vel,
                                                             portamento,
                                                             note,
                                                             false);
            if (kit[0].Psubenabled != 0)
                partnote[pos].kititem[0].subnote = new SUBnote(kit[0].subpars,
                                                               &ctl,
                                                               this->_synth,
                                                               notebasefreq,
                                                               vel,
                                                               portamento,
                                                               note,
                                                               false);
            if (kit[0].Ppadenabled != 0)
                partnote[pos].kititem[0].padnote = new PADnote(kit[0].padpars,
                                                               &ctl,
                                                               this->_synth,
                                                               notebasefreq,
                                                               vel,
                                                               portamento,
                                                               note,
                                                               false);
            if ((kit[0].Padenabled != 0) || (kit[0].Psubenabled != 0) || (kit[0].Ppadenabled != 0))
                partnote[pos].itemsplaying++;

            // Spawn another note (but silent) if legatomodevalid==true
            if (legatomodevalid)
            {
                partnote[posb].kititem[0].sendtoparteffect = 0;
                if (kit[0].Padenabled != 0)
                    partnote[posb].kititem[0].adnote = new ADnote(kit[0].adpars,
                                                                  &ctl,
                                                                  this->_synth,
                                                                  notebasefreq,
                                                                  vel,
                                                                  portamento,
                                                                  note,
                                                                  true); //true for silent.
                if (kit[0].Psubenabled != 0)
                    partnote[posb].kititem[0].subnote = new SUBnote(
                        kit[0].subpars,
                        &ctl,
                        this->_synth,
                        notebasefreq,
                        vel,
                        portamento,
                        note,
                        true);
                if (kit[0].Ppadenabled != 0)
                    partnote[posb].kititem[0].padnote = new PADnote(
                        kit[0].padpars,
                        &ctl,
                        this->_synth,
                        notebasefreq,
                        vel,
                        portamento,
                        note,
                        true);
                if ((kit[0].Padenabled != 0) || (kit[0].Psubenabled != 0) || (kit[0].Ppadenabled != 0))
                    partnote[posb].itemsplaying++;
            }
        }
        else //init the notes for the "kit mode"
            for (auto &item : kit)
            {
                if (item.Pmuted != 0)
                    continue;
                if ((note < item.Pminkey) || (note > item.Pmaxkey))
                    continue;

                int ci = partnote[pos].itemsplaying; //ci=current item

                //if this parameter is 127 for "unprocessed"
                partnote[pos].kititem[ci].sendtoparteffect =
                    (item.Psendtoparteffect < NUM_PART_EFX ? item.Psendtoparteffect : NUM_PART_EFX);

                if ((item.adpars != nullptr) && ((item.Padenabled) != 0))
                    partnote[pos].kititem[ci].adnote = new ADnote(
                        item.adpars,
                        &ctl,
                        this->_synth,
                        notebasefreq,
                        vel,
                        portamento,
                        note,
                        false);

                if ((item.subpars != nullptr) && ((item.Psubenabled) != 0))
                    partnote[pos].kititem[ci].subnote = new SUBnote(
                        item.subpars,
                        &ctl,
                        this->_synth,
                        notebasefreq,
                        vel,
                        portamento,
                        note,
                        false);

                if ((item.padpars != nullptr) && ((item.Ppadenabled) != 0))
                    partnote[pos].kititem[ci].padnote = new PADnote(
                        item.padpars,
                        &ctl,
                        this->_synth,
                        notebasefreq,
                        vel,
                        portamento,
                        note,
                        false);

                // Spawn another note (but silent) if legatomodevalid==true
                if (legatomodevalid)
                {
                    partnote[posb].kititem[ci].sendtoparteffect =
                        (item.Psendtoparteffect <
                                 NUM_PART_EFX
                             ? item.Psendtoparteffect
                             : NUM_PART_EFX); //if this parameter is 127 for "unprocessed"

                    if ((item.adpars != nullptr) && ((item.Padenabled) != 0))
                        partnote[posb].kititem[ci].adnote = new ADnote(
                            item.adpars,
                            &ctl,
                            this->_synth,
                            notebasefreq,
                            vel,
                            portamento,
                            note,
                            true); //true for silent.
                    if ((item.subpars != nullptr) && ((item.Psubenabled) != 0))
                        partnote[posb].kititem[ci].subnote =
                            new SUBnote(item.subpars,
                                        &ctl,
                                        this->_synth,
                                        notebasefreq,
                                        vel,
                                        portamento,
                                        note,
                                        true);
                    if ((item.padpars != nullptr) && ((item.Ppadenabled) != 0))
                        partnote[posb].kititem[ci].padnote =
                            new PADnote(item.padpars,
                                        &ctl,
                                        this->_synth,
                                        notebasefreq,
                                        vel,
                                        portamento,
                                        note,
                                        true);

                    if ((item.adpars != nullptr) || (item.subpars != nullptr))
                        partnote[posb].itemsplaying++;
                }

                if ((item.adpars != nullptr) || (item.subpars != nullptr))
                {
                    partnote[pos].itemsplaying++;
                    if (((item.Padenabled != 0) || (item.Psubenabled != 0) || (item.Ppadenabled != 0)) && (Pkitmode == 2))
                        break;
                }
            }
    }

    //this only relase the keys if there is maximum number of keys allowed
    setkeylimit(Pkeylimit);
}

/*
 * Note Off Messages
 */
void Instrument::NoteOff(unsigned char note) //relase the key
{
    int i;

    // This note is released, so we remove it from the list.
    if (not monomemnotes.empty())
        monomemnotes.remove(note);

    for (i = POLIPHONY - 1; i >= 0; i--) //first note in, is first out if there are same note multiple times
        if ((partnote[i].status == KEY_PLAYING) && (partnote[i].note == note))
        {
            if (ctl.sustain.sustain == 0)
            { //the sustain pedal is not pushed
                if ((Ppolymode == 0) && (not monomemnotes.empty()))
                    MonoMemRenote(); // To play most recent still held note.
                else
                    RelaseNotePos(static_cast<unsigned int>(i));
                /// break;
            }
            else //the sustain pedal is pushed
                partnote[i].status = KEY_RELASED_AND_SUSTAINED;
        }
}

void Instrument::PolyphonicAftertouch(unsigned char note,
                                      unsigned char velocity,
                                      int masterkeyshift)
{
    (void)masterkeyshift;
    if (!Pnoteon || (note < Pminkey) || (note > Pmaxkey))
        return;
    if (Pdrummode)
        return;

    // MonoMem stuff:
    if (!Ppolymode) // if Poly is off

        monomem[note].velocity = velocity; // Store this note's velocity.

    for (auto & i : partnote)
        if ((i.note == note) && (i.status == KEY_PLAYING))
        {
            /* update velocity */
            // compute the velocity offset
            float vel =
                VelF(velocity / 127.0f, Pvelsns) + (Pveloffs - 64.0f) / 64.0f;
            vel = (vel < 0.0f) ? 0.0f : vel;
            vel = (vel > 1.0f) ? 1.0f : vel;

            if (!Pkitmode)
            { // "normal mode"
                if (kit[0].Padenabled && i.kititem[0].adnote)
                    i.kititem[0].adnote->setVelocity(vel);
                if (kit[0].Psubenabled && i.kititem[0].subnote)
                    i.kititem[0].subnote->setVelocity(vel);
                if (kit[0].Ppadenabled && i.kititem[0].padnote)
                    i.kititem[0].padnote->setVelocity(vel);
            }
            else // "kit mode"
                for (int item = 0; item < NUM_KIT_ITEMS; ++item)
                {
                    if (kit[item].Pmuted)
                        continue;
                    if ((note < kit[item].Pminkey) || (note > kit[item].Pmaxkey))
                        continue;

                    if (kit[item].Padenabled && i.kititem[item].adnote)
                        i.kititem[item].adnote->setVelocity(vel);
                    if (kit[item].Psubenabled && i.kititem[item].subnote)
                        i.kititem[item].subnote->setVelocity(vel);
                    if (kit[item].Ppadenabled && i.kititem[item].padnote)
                        i.kititem[item].padnote->setVelocity(vel);
                }
        }
}

/*
 * Controllers
 */
void Instrument::SetController(unsigned int type, int par)
{
    switch (type)
    {
        case C_pitchwheel:
            ctl.setpitchwheel(par);
            break;
        case C_expression:
            ctl.setexpression(par);
            setPvolume(Pvolume); //update the volume
            break;
        case C_portamento:
            ctl.setportamento(par);
            break;
        case C_panning:
            ctl.setpanning(par);
            setPpanning(Ppanning); //update the panning
            break;
        case C_filtercutoff:
            ctl.setfiltercutoff(par);
            break;
        case C_filterq:
            ctl.setfilterq(par);
            break;
        case C_bandwidth:
            ctl.setbandwidth(par);
            break;
        case C_modwheel:
            ctl.setmodwheel(par);
            break;
        case C_fmamp:
            ctl.setfmamp(par);
            break;
        case C_volume:
            ctl.setvolume(par);
            if (ctl.volume.receive != 0)
                volume = ctl.volume.volume;
            else
                setPvolume(Pvolume);
            break;
        case C_sustain:
            ctl.setsustain(par);
            if (ctl.sustain.sustain == 0)
                RelaseSustainedKeys();
            break;
        case C_allsoundsoff:
            AllNotesOff(); //Panic
            break;
        case C_resetallcontrollers:
            ctl.resetall();
            RelaseSustainedKeys();
            if (ctl.volume.receive != 0)
                volume = ctl.volume.volume;
            else
                setPvolume(Pvolume);
            setPvolume(Pvolume);   //update the volume
            setPpanning(Ppanning); //update the panning

            for (auto & item : kit)
            {
                if (item.adpars == nullptr)
                    continue;
                item.adpars->GlobalPar.Reson->sendcontroller(C_resonance_center, 1.0f);

                item.adpars->GlobalPar.Reson->sendcontroller(C_resonance_bandwidth, 1.0f);
            }
            //more update to add here if I add controllers
            break;
        case C_allnotesoff:
            RelaseAllKeys();
            break;
        case C_resonance_center:
            ctl.setresonancecenter(par);
            for (auto & item : kit)
            {
                if (item.adpars == nullptr)
                    continue;
                item.adpars->GlobalPar.Reson->sendcontroller(C_resonance_center,
                                                                  ctl.resonancecenter.relcenter);
            }
            break;
        case C_resonance_bandwidth:
            ctl.setresonancebw(par);
            kit[0].adpars->GlobalPar.Reson->sendcontroller(C_resonance_bandwidth, ctl.resonancebandwidth.relbw);
            break;
    }
}
/*
 * Relase the sustained keys
 */

void Instrument::RelaseSustainedKeys()
{
    // Let's call MonoMemRenote() on some conditions:
    if ((Ppolymode == 0) && (not monomemnotes.empty()))
        if (monomemnotes.back() != lastnote) // Sustain controller manipulation would cause repeated same note respawn without this check.
            MonoMemRenote();                 // To play most recent still held note.

    for (unsigned int i = 0; i < POLIPHONY; ++i)
        if (partnote[i].status == KEY_RELASED_AND_SUSTAINED)
            RelaseNotePos(i);
}

/*
 * Relase all keys
 */

void Instrument::RelaseAllKeys()
{
    for (unsigned int i = 0; i < POLIPHONY; ++i)
        if ((partnote[i].status != KEY_RELASED) && (partnote[i].status != KEY_OFF)) //thanks to Frank Neumann
            RelaseNotePos(i);
}

// Call NoteOn(...) with the most recent still held key as new note
// (Made for Mono/Legato).
void Instrument::MonoMemRenote()
{
    unsigned char mmrtempnote = monomemnotes.back(); // Last list element.
    monomemnotes.pop_back();                         // We remove it, will be added again in NoteOn(...).
    if (Pnoteon == 0)
        RelaseNotePos(lastpos);
    else
        NoteOn(mmrtempnote, monomem[mmrtempnote].velocity,
               monomem[mmrtempnote].mkeyshift);
}

/*
 * Release note at position
 */
void Instrument::RelaseNotePos(unsigned int pos)
{
    for (auto & j : partnote[pos].kititem)
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
    }
    partnote[pos].status = KEY_RELASED;
}

/*
 * Kill note at position
 */
void Instrument::KillNotePos(unsigned int pos)
{
    partnote[pos].status = KEY_OFF;
    partnote[pos].note = -1;
    partnote[pos].time = 0;
    partnote[pos].itemsplaying = 0;

    for (auto & j : partnote[pos].kititem)
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
void Instrument::setkeylimit(unsigned char Pkeylimit)
{
    this->Pkeylimit = Pkeylimit;
    int keylimit = Pkeylimit;
    if (keylimit == 0)
        keylimit = POLIPHONY - 5;

    //release old keys if the number of notes>keylimit
    if (Ppolymode != 0)
    {
        int notecount = 0;
        for (auto & i : partnote)
            if ((i.status == KEY_PLAYING) || (i.status == KEY_RELASED_AND_SUSTAINED))
                notecount++;

        int oldestnotepos = -1;
        if (notecount > keylimit) //find out the oldest note
            for (int i = 0; i < POLIPHONY; ++i)
            {
                int maxtime = 0;
                if (((partnote[i].status == KEY_PLAYING) || (partnote[i].status == KEY_RELASED_AND_SUSTAINED)) && (partnote[i].time > maxtime))
                {
                    maxtime = partnote[i].time;
                    oldestnotepos = i;
                }
            }
        if (oldestnotepos != -1)
            RelaseNotePos(static_cast<unsigned int>(oldestnotepos));
    }
}

/*
 * Prepare all notes to be turned off
 */
void Instrument::AllNotesOff()
{
    killallnotes = 1;
}

void Instrument::RunNote(unsigned int k)
{
    unsigned noteplay = 0;
    for (int item = 0; item < partnote[k].itemsplaying; ++item)
    {
        int sendcurrenttofx = partnote[k].kititem[item].sendtoparteffect;

        for (unsigned type = 0; type < 3; ++type)
        {
            //Select a note
            SynthNote **note = nullptr;
            if (type == 0)
                note = &partnote[k].kititem[item].adnote;
            else if (type == 1)
                note = &partnote[k].kititem[item].subnote;
            else if (type == 2)
                note = &partnote[k].kititem[item].padnote;

            //Process if it exists
            if (!(*note))
                continue;
            noteplay++;

            float tmpoutr[this->_synth->buffersize];
            float tmpoutl[this->_synth->buffersize];
            (*note)->noteout(&tmpoutl[0], &tmpoutr[0]);

            if ((*note)->finished())
            {
                delete (*note);
                (*note) = nullptr;
            }
            for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
            { //add the note to part(mix)
                partfxinputl[sendcurrenttofx][i] += tmpoutl[i];
                partfxinputr[sendcurrenttofx][i] += tmpoutr[i];
            }
        }
    }

    //Kill note if there is no synth on that note
    if (noteplay == 0)
        KillNotePos(k);
}

/*
 * Compute Part samples and store them in the partoutl[] and partoutr[]
 */
void Instrument::ComputePartSmps()
{
    for (unsigned nefx = 0; nefx < NUM_PART_EFX + 1; ++nefx)
        for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
        {
            partfxinputl[nefx][i] = 0.0f;
            partfxinputr[nefx][i] = 0.0f;
        }

    for (unsigned k = 0; k < POLIPHONY; ++k)
    {
        if (partnote[k].status == KEY_OFF)
            continue;
        partnote[k].time++;
        //get the sampledata of the note and kill it if it's finished
        RunNote(k);
    }

    //Apply part's effects and mix them
    for (int nefx = 0; nefx < NUM_PART_EFX; ++nefx)
    {
        if (!Pefxbypass[nefx])
        {
            partefx[nefx]->out(partfxinputl[nefx], partfxinputr[nefx]);
            if (Pefxroute[nefx] == 2)
                for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
                {
                    partfxinputl[nefx + 1][i] += partefx[nefx]->efxoutl[i];
                    partfxinputr[nefx + 1][i] += partefx[nefx]->efxoutr[i];
                }
        }
        int routeto = ((Pefxroute[nefx] == 0) ? nefx + 1 : NUM_PART_EFX);
        for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
        {
            partfxinputl[routeto][i] += partfxinputl[nefx][i];
            partfxinputr[routeto][i] += partfxinputr[nefx][i];
        }
    }
    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        partoutl[i] = partfxinputl[NUM_PART_EFX][i];
        partoutr[i] = partfxinputr[NUM_PART_EFX][i];
    }

    //Kill All Notes if killallnotes!=0
    if (killallnotes != 0)
    {
        for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
        {
            float tmp = (this->_synth->buffersize_f - i) / this->_synth->buffersize_f;
            partoutl[i] *= tmp;
            partoutr[i] *= tmp;
        }
        for (unsigned int k = 0; k < POLIPHONY; ++k)
            KillNotePos(k);
        killallnotes = 0;
        for (auto & nefx : partefx)
            nefx->cleanup();
    }
    ctl.updateportamento();
}

/*
 * Parameter control
 */
void Instrument::setPvolume(unsigned char Pvolume_)
{
    Pvolume = Pvolume_;
    volume =
        dB2rap((Pvolume - 96.0f) / 96.0f * 40.0f) * ctl.expression.relvolume;
}

void Instrument::setPpanning(unsigned char Ppanning_)
{
    Ppanning = Ppanning_;
    panning = Ppanning / 127.0f + ctl.panning.pan;
    if (panning < 0.0f)
        panning = 0.0f;
    else if (panning > 1.0f)
        panning = 1.0f;
}

/*
 * Enable or disable a kit item
 */
void Instrument::setkititemstatus(int kititem, int Penabled_)
{
    if ((kititem == 0) || (kititem >= NUM_KIT_ITEMS))
        return; //nonexistent kit item and the first kit item is always enabled
    kit[kititem].Penabled = static_cast<unsigned char>(Penabled_);

    bool resetallnotes = false;
    if (Penabled_ == 0)
    {
        if (kit[kititem].adpars != nullptr)
            delete (kit[kititem].adpars);
        if (kit[kititem].subpars != nullptr)
            delete (kit[kititem].subpars);
        if (kit[kititem].padpars != nullptr)
        {
            delete (kit[kititem].padpars);
            resetallnotes = true;
        }
        kit[kititem].adpars = nullptr;
        kit[kititem].subpars = nullptr;
        kit[kititem].padpars = nullptr;
        kit[kititem].Pname[0] = '\0';
    }
    else
    {
        if (kit[kititem].adpars == nullptr)
            kit[kititem].adpars = new ADnoteParameters(this->_synth, fft);
        if (kit[kititem].subpars == nullptr)
            kit[kititem].subpars = new SUBnoteParameters(this->_synth);
        if (kit[kititem].padpars == nullptr)
            kit[kititem].padpars = new PADnoteParameters(this->_synth, fft, mutex);
    }

    if (resetallnotes)
        for (unsigned int k = 0; k < POLIPHONY; ++k)
            KillNotePos(k);
}

void Instrument::add2XMLinstrument(XMLwrapper *xml)
{
    xml->beginbranch("INFO");
    xml->addparstr("name", (char *)Pname);
    xml->addparstr("author", (char *)info.Pauthor);
    xml->addparstr("comments", (char *)info.Pcomments);
    xml->addpar("type", info.Ptype);
    xml->endbranch();

    xml->beginbranch("INSTRUMENT_KIT");
    xml->addpar("kit_mode", Pkitmode);
    xml->addparbool("drum_mode", Pdrummode);

    for (int i = 0; i < NUM_KIT_ITEMS; ++i)
    {
        xml->beginbranch("INSTRUMENT_KIT_ITEM", i);
        xml->addparbool("enabled", kit[i].Penabled);
        if (kit[i].Penabled != 0)
        {
            xml->addparstr("name", (char *)kit[i].Pname);

            xml->addparbool("muted", kit[i].Pmuted);
            xml->addpar("min_key", kit[i].Pminkey);
            xml->addpar("max_key", kit[i].Pmaxkey);

            xml->addpar("send_to_instrument_effect", kit[i].Psendtoparteffect);

            xml->addparbool("add_enabled", kit[i].Padenabled);
            if ((kit[i].Padenabled != 0) && (kit[i].adpars != nullptr))
            {
                xml->beginbranch("ADD_SYNTH_PARAMETERS");
                kit[i].adpars->add2XML(xml);
                xml->endbranch();
            }

            xml->addparbool("sub_enabled", kit[i].Psubenabled);
            if ((kit[i].Psubenabled != 0) && (kit[i].subpars != nullptr))
            {
                xml->beginbranch("SUB_SYNTH_PARAMETERS");
                kit[i].subpars->add2XML(xml);
                xml->endbranch();
            }

            xml->addparbool("pad_enabled", kit[i].Ppadenabled);
            if ((kit[i].Ppadenabled != 0) && (kit[i].padpars != nullptr))
            {
                xml->beginbranch("PAD_SYNTH_PARAMETERS");
                kit[i].padpars->add2XML(xml);
                xml->endbranch();
            }
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("INSTRUMENT_EFFECTS");
    for (int nefx = 0; nefx < NUM_PART_EFX; ++nefx)
    {
        xml->beginbranch("INSTRUMENT_EFFECT", nefx);
        xml->beginbranch("EFFECT");
        partefx[nefx]->add2XML(xml);
        xml->endbranch();

        xml->addpar("route", Pefxroute[nefx]);
        partefx[nefx]->setdryonly(Pefxroute[nefx] == 2);
        xml->addparbool("bypass", Pefxbypass[nefx]);
        xml->endbranch();
    }
    xml->endbranch();
}

void Instrument::add2XML(XMLwrapper *xml)
{
    //parameters
    xml->addparbool("enabled", Penabled);
    if ((Penabled == 0) && (xml->minimal))
        return;

    xml->addpar("volume", Pvolume);
    xml->addpar("panning", Ppanning);

    xml->addpar("min_key", Pminkey);
    xml->addpar("max_key", Pmaxkey);
    xml->addpar("key_shift", Pkeyshift);
    xml->addpar("rcv_chn", Prcvchn);

    xml->addpar("velocity_sensing", Pvelsns);
    xml->addpar("velocity_offset", Pveloffs);

    xml->addparbool("note_on", Pnoteon);
    xml->addparbool("poly_mode", Ppolymode);
    xml->addpar("legato_mode", Plegatomode);
    xml->addpar("key_limit", Pkeylimit);

    xml->beginbranch("INSTRUMENT");
    add2XMLinstrument(xml);
    xml->endbranch();

    xml->beginbranch("CONTROLLER");
    ctl.add2XML(xml);
    xml->endbranch();
}

int Instrument::saveXML(const char *filename)
{
    XMLwrapper *xml;
    xml = new XMLwrapper();

    xml->beginbranch("INSTRUMENT");
    add2XMLinstrument(xml);
    xml->endbranch();

    int result = xml->saveXMLfile(filename);
    delete (xml);
    return result;
}

int Instrument::loadXMLinstrument(const char *filename) /*{*/
{
    auto *xml = new XMLwrapper();
    if (xml->loadXMLfile(filename) < 0)
    {
        delete (xml);
        return -1;
    }

    if (xml->enterbranch("INSTRUMENT") == 0)
        return -10;
    getfromXMLinstrument(xml);
    xml->exitbranch();

    delete (xml);
    return 0;
} /*}*/

void Instrument::applyparameters(bool lockmutex) /*{*/
{
    for (auto & n : kit)
        if ((n.padpars != nullptr) && (n.Ppadenabled != 0))
            n.padpars->applyparameters(lockmutex);
} /*}*/

void Instrument::getfromXMLinstrument(XMLwrapper *xml)
{
    if (xml->enterbranch("INFO"))
    {
        xml->getparstr("name", (char *)Pname, PART_MAX_NAME_LEN);
        xml->getparstr("author", (char *)info.Pauthor, MAX_INFO_TEXT_SIZE);
        xml->getparstr("comments", (char *)info.Pcomments, MAX_INFO_TEXT_SIZE);
        info.Ptype = xml->getpar("type", info.Ptype, 0, 16);

        xml->exitbranch();
    }

    if (xml->enterbranch("INSTRUMENT_KIT"))
    {
        Pkitmode = xml->getpar127("kit_mode", Pkitmode);
        Pdrummode = xml->getparbool("drum_mode", Pdrummode);

        setkititemstatus(0, 0);
        for (int i = 0; i < NUM_KIT_ITEMS; ++i)
        {
            if (xml->enterbranch("INSTRUMENT_KIT_ITEM", i) == 0)
                continue;
            setkititemstatus(i, xml->getparbool("enabled", kit[i].Penabled));
            if (kit[i].Penabled == 0)
            {
                xml->exitbranch();
                continue;
            }

            xml->getparstr("name", (char *)kit[i].Pname, PART_MAX_NAME_LEN);

            kit[i].Pmuted = xml->getparbool("muted", kit[i].Pmuted);
            kit[i].Pminkey = xml->getpar127("min_key", kit[i].Pminkey);
            kit[i].Pmaxkey = xml->getpar127("max_key", kit[i].Pmaxkey);

            kit[i].Psendtoparteffect = xml->getpar127(
                "send_to_instrument_effect",
                kit[i].Psendtoparteffect);

            kit[i].Padenabled = xml->getparbool("add_enabled",
                                                kit[i].Padenabled);
            if (xml->enterbranch("ADD_SYNTH_PARAMETERS"))
            {
                kit[i].adpars->getfromXML(xml);
                xml->exitbranch();
            }

            kit[i].Psubenabled = xml->getparbool("sub_enabled",
                                                 kit[i].Psubenabled);
            if (xml->enterbranch("SUB_SYNTH_PARAMETERS"))
            {
                kit[i].subpars->getfromXML(xml);
                xml->exitbranch();
            }

            kit[i].Ppadenabled = xml->getparbool("pad_enabled",
                                                 kit[i].Ppadenabled);
            if (xml->enterbranch("PAD_SYNTH_PARAMETERS"))
            {
                kit[i].padpars->getfromXML(xml);
                xml->exitbranch();
            }

            xml->exitbranch();
        }

        xml->exitbranch();
    }

    if (xml->enterbranch("INSTRUMENT_EFFECTS"))
    {
        for (int nefx = 0; nefx < NUM_PART_EFX; ++nefx)
        {
            if (xml->enterbranch("INSTRUMENT_EFFECT", nefx) == 0)
                continue;
            if (xml->enterbranch("EFFECT"))
            {
                partefx[nefx]->getfromXML(xml);
                xml->exitbranch();
            }

            Pefxroute[nefx] = xml->getpar("route",
                                          Pefxroute[nefx],
                                          0,
                                          NUM_PART_EFX);
            partefx[nefx]->setdryonly(Pefxroute[nefx] == 2);
            Pefxbypass[nefx] = xml->getparbool("bypass", Pefxbypass[nefx]);
            xml->exitbranch();
        }
        xml->exitbranch();
    }
}

void Instrument::getfromXML(XMLwrapper *xml)
{
    Penabled = xml->getparbool("enabled", Penabled);

    setPvolume(xml->getpar127("volume", Pvolume));
    setPpanning(xml->getpar127("panning", Ppanning));

    Pminkey = xml->getpar127("min_key", Pminkey);
    Pmaxkey = xml->getpar127("max_key", Pmaxkey);
    Pkeyshift = xml->getpar127("key_shift", Pkeyshift);
    Prcvchn = xml->getpar127("rcv_chn", Prcvchn);

    Pvelsns = xml->getpar127("velocity_sensing", Pvelsns);
    Pveloffs = xml->getpar127("velocity_offset", Pveloffs);

    Pnoteon = xml->getparbool("note_on", Pnoteon);
    Ppolymode = xml->getparbool("poly_mode", Ppolymode);
    Plegatomode = xml->getparbool("legato_mode", Plegatomode); //older versions
    if (!Plegatomode)
        Plegatomode = xml->getpar127("legato_mode", Plegatomode);
    Pkeylimit = xml->getpar127("key_limit", Pkeylimit);

    if (xml->enterbranch("INSTRUMENT"))
    {
        getfromXMLinstrument(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("CONTROLLER"))
    {
        ctl.getfromXML(xml);
        xml->exitbranch();
    }
}
