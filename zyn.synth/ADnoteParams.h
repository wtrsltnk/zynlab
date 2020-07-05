/*
  ZynAddSubFX - a software synthesizer

  ADnoteParameters.h - Parameters for ADnote (ADsynth)
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

#ifndef AD_NOTE_PARAMETERS_H
#define AD_NOTE_PARAMETERS_H

#include "ADnoteVoiceParam.h"
#include <zyn.common/Presets.h>
#include <zyn.common/Util.h>
#include <zyn.common/globals.h>

class EnvelopeParams;
class LFOParams;
class FilterParams;
class Resonance;
class OscilGen;
class IFFTwrapper;

enum FMTYPE
{
    NONE,
    MORPH,
    RING_MOD,
    PHASE_MOD,
    FREQ_MOD,
    PITCH_MOD
};
extern int ADnote_unison_sizes[];

class ADnoteParameters :
    public AbstractNoteParameters
{
    IFFTwrapper *_fft;

public:
    ADnoteParameters(IFFTwrapper *fft);
    virtual ~ADnoteParameters();

    void InitPresets();

    void Defaults();

    /******************************************
    *     FREQUENCY GLOBAL PARAMETERS        *
    ******************************************/
    unsigned short int PDetune;       //fine detune
    unsigned short int PCoarseDetune; //coarse detune+octave
    unsigned char PDetuneType;        //detune type

    unsigned char PBandwidth; //how much the relative fine detunes of the voices are changed

    EnvelopeParams *FreqEnvelope; //Frequency Envelope

    LFOParams *FreqLfo; //Frequency LFO

    /********************************************
    *     AMPLITUDE GLOBAL PARAMETERS          *
    ********************************************/

    EnvelopeParams *AmpEnvelope;

    LFOParams *AmpLfo;

    unsigned char PPunchStrength;
    unsigned char PPunchTime;
    unsigned char PPunchStretch;
    unsigned char PPunchVelocitySensing;

    /******************************************
    *        FILTER GLOBAL PARAMETERS        *
    ******************************************/
    FilterParams *GlobalFilter;

    // filter velocity sensing
    unsigned char PFilterVelocityScale;

    // filter velocity sensing
    unsigned char PFilterVelocityScaleFunction;

    EnvelopeParams *FilterEnvelope;

    LFOParams *FilterLfo;

    // RESONANCE
    Resonance *Reson;

    //how the randomness is applied to the harmonics on more voices using the same oscillator
    unsigned char Hrandgrouping;

    ADnoteVoiceParam VoicePar[NUM_VOICES];

    float getBandwidthDetuneMultiplier();
    float getUnisonFrequencySpreadCents(int nvoice);
    int get_unison_size_index(int nvoice);
    void set_unison_size_index(int nvoice, int index);

private:
    void EnableVoice(int nvoice);
    void DisableVoice(int nvoice);
};

#endif
