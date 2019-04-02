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

    void updateFrequencyMultipliers(void);

    std::map<unsigned char, WavData *> PwavData;

    //Parameters
    //AMPLITUDE PARAMETRERS
    unsigned char Pstereo; //0 for mono,1 for stereo
    unsigned char PVolume;
    unsigned char PPanning;
    unsigned char PAmpVelocityScaleFunction;
    EnvelopeParams *AmpEnvelope;

    //Frequency Parameters
    unsigned short int PDetune;
    unsigned short int PCoarseDetune;
    unsigned char PDetuneType;
    unsigned char PFreqEnvelopeEnabled;
    EnvelopeParams *FreqEnvelope;
    unsigned char PBandWidthEnvelopeEnabled;
    EnvelopeParams *BandWidthEnvelope;

    //Filter Parameters (Global)
    unsigned char PGlobalFilterEnabled;
    FilterParams *GlobalFilter;
    unsigned char PGlobalFilterVelocityScale;
    unsigned char PGlobalFilterVelocityScaleFunction;
    EnvelopeParams *GlobalFilterEnvelope;

    //Other Parameters

    //If the base frequency is fixed to 440 Hz
    unsigned char Pfixedfreq;

    /* Equal temperate (this is used only if the Pfixedfreq is enabled)
           If this parameter is 0, the frequency is fixed (to 440 Hz);
           if this parameter is 64, 1 MIDI halftone -> 1 frequency halftone */
    unsigned char PfixedfreqET;

    // Overtone spread parameters
    struct
    {
        unsigned char type;
        unsigned char par1;
        unsigned char par2;
        unsigned char par3;
    } POvertoneSpread;
    float POvertoneFreqMult[MAX_SUB_HARMONICS];

    //how many times the filters are applied
    unsigned char Pnumstages;

    //bandwidth
    unsigned char Pbandwidth;

    //How the magnitudes are computed (0=linear,1=-60dB,2=-60dB)
    unsigned char Phmagtype;

    //Magnitudes
    unsigned char Phmag[MAX_SUB_HARMONICS];

    //Relative BandWidth ("64"=1.0f)
    unsigned char Phrelbw[MAX_SUB_HARMONICS];

    //how much the bandwidth is increased according to lower/higher frequency; 64-default
    unsigned char Pbwscale;

    //how the harmonics start("0"=0,"1"=random,"2"=1)
    unsigned char Pstart;

private:
};

#endif // SAMPLE_NOTE_PARAMETERS_H
