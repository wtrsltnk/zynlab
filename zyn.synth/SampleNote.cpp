/*
  ZynAddSubFX - a software synthesizer

  SampleNote.cpp - The "subtractive" synthesizer
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

#include "SampleNote.h"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <zyn.common/Util.h>

SampleNote::SampleNote(SampleNoteParameters *parameters, Controller *ctl_, float freq, float velocity, int midinote)
    : SynthNote(freq, velocity, false, midinote, false), _parameters(parameters), wavProgress(0), ctl(ctl_)
{
    NoteEnabled = ON;
    setup(freq, velocity, midinote);
}

void SampleNote::setup(float /*freq*/, float velocity, int /*midinote*/)
{
    NoteEnabled = ON;
    volume = powf(0.1f, 3.0f * (1.0f - _parameters->PVolume / 96.0f)); //-60 dB .. 0 dB
    volume *= VelF(velocity, _parameters->PAmpVelocityScaleFunction);
    if (_parameters->PPanning != 0)
    {
        panning = _parameters->PPanning / 127.0f;
    }
    else
    {
        panning = RND;
    }
}

void SampleNote::legatonote(float freq, float velocity, int portamento_, int midinote, bool externcall)
{
    // Manage legato stuff
    if (legato.update(freq, velocity, portamento_, midinote, externcall))
    {
        return;
    }

    setup(freq, velocity, midinote);
}

SampleNote::~SampleNote()
{
    if (NoteEnabled != OFF)
    {
        KillNote();
    }
}

/*
 * Kill the note
 */
void SampleNote::KillNote()
{
    if (NoteEnabled == OFF)
    {
        return;
    }

    NoteEnabled = OFF;
}

/*
 * Note Output
 */
int SampleNote::noteout(float *outl, float *outr)
{
    memcpy(outl, SystemSettings::Instance().denormalkillbuf, SystemSettings::Instance().bufferbytes);
    memcpy(outr, SystemSettings::Instance().denormalkillbuf, SystemSettings::Instance().bufferbytes);

    if (NoteEnabled == OFF)
    {
        return 0;
    }

    if (_parameters->PwavData.find(legato.getMidinote()) == _parameters->PwavData.end())
    {
        return 0;
    }

    auto wavData = _parameters->PwavData.find(legato.getMidinote())->second;
    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        if (wavProgress < (wavData->samplesPerChannel * wavData->channels))
        {
            outl[i] += (wavData->PwavData[wavProgress++] * panning);
            outr[i] += (wavData->PwavData[wavProgress++] * (1.0f - panning));
        }
        else
        {
            KillNote();
            return 1;
        }
    }

    return 1;
}

/*
 * Relase Key (Note Off)
 */
void SampleNote::relasekey()
{}

/*
 * Check if the note is finished
 */
bool SampleNote::finished() const
{
    return NoteEnabled == OFF;
}
