/*
  ZynAddSubFX - a software synthesizer

  SUBnoteParameters.h - Parameters for SUBnote (SUBsynth)
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

#ifndef SAMPLE_NOTE_PARAMETERS_H
#define SAMPLE_NOTE_PARAMETERS_H

#include "EnvelopeParams.h"
#include <map>
#include <zyn.common/IPresetsSerializer.h>
#include <zyn.common/Presets.h>
#include <zyn.common/WavData.h>
#include <zyn.common/globals.h>
#include <zyn.dsp/FilterParams.h>

class SampleNoteParameters : public WrappedPresets
{
public:
    SampleNoteParameters();
    virtual ~SampleNoteParameters();

    void InitPresets();

    void Defaults();

    std::map<int, WavData *> PwavData;

    //Parameters
    //AMPLITUDE PARAMETRERS
    unsigned char PVolume;
    unsigned char PPanning;
    unsigned char PAmpVelocityScaleFunction;
};

#endif // SAMPLE_NOTE_PARAMETERS_H
