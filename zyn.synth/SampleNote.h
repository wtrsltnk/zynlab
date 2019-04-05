/*
  ZynAddSubFX - a software synthesizer

  SUBnote.h - The subtractive synthesizer
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

#ifndef SAMPLE_NOTE_H
#define SAMPLE_NOTE_H

#include "Controller.h"
#include "SampleNoteParams.h"
#include "SynthNote.h"
#include <zyn.common/globals.h>

class SampleNote : public SynthNote
{
public:
    SampleNote(SampleNoteParameters *parameters, Controller *ctl_, float freq,
               float velocity, int midinote);
    virtual ~SampleNote();

    void legatonote(float freq, float velocity, int portamento_,
                    int midinote, bool externcall);

    int noteout(float *outl, float *outr); //note output,return 0 if the note is finished
    void relasekey();
    bool finished() const;

private:
    void setup(float freq,
               float velocity,
               int midinote);
    void KillNote();

    SampleNoteParameters *_parameters;
    unsigned int wavProgress;

    //parameters
    float panning;

    //internal values
    ONOFFTYPE NoteEnabled;
    float volume;

    Controller *ctl;
};

#endif // SAMPLE_NOTE_H
