/*
  ZynAddSubFX - a software synthesizer

  PADnoteParameters.h - Parameters for PADnote (PADsynth)
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

#ifndef PAD_NOTE_PARAMETERS_H
#define PAD_NOTE_PARAMETERS_H

#include "AbstractNoteParameters.h"
#include "LFOParams.h"
#include "OscilGen.h"
#include "Resonance.h"

#include <zyn.common/IPresetsSerializer.h>
#include <zyn.common/Presets.h>
#include <zyn.common/Util.h>
#include <zyn.common/globals.h>
#include <zyn.dsp/FilterParams.h>

#include <mutex>
#include <string>

class PADnoteParameters :
    public AbstractNoteParameters
{
public:
    PADnoteParameters();
    virtual ~PADnoteParameters();

    void InitPresets();

    void Defaults();

    //returns a value between 0.0f-1.0f that represents the estimation perceived bandwidth
    float getprofile(
        float *smp,
        int size);

    //returns the BandWidth in cents
    float setPBandwidth(
        int PBandwidth);

    //gets the n-th overtone position relatively to N harmonic
    float getNhr(
        int n);

    void ApplyParameters(
        std::mutex &mutex);

    void export2wav(
        std::string basefilename,
        std::mutex &mutex);

    //the mode: 0 - bandwidth, 1 - discrete (bandwidth=0), 2 - continous
    //the harmonic profile is used only on mode 0
    unsigned char Pmode;

    //Harmonic profile (the frequency distribution of a single harmonic)
    struct
    {
        struct
        { //base function
            unsigned char type;
            unsigned char par1;
        } base;
        unsigned char freqmult; //frequency multiplier of the distribution
        struct
        { //the modulator of the distribution
            unsigned char par1;
            unsigned char freq;
        } modulator;

        unsigned char width; //the width of the resulting function after the modulation
        struct
        { //the amplitude multiplier of the harmonic profile
            unsigned char mode;
            unsigned char type;
            unsigned char par1;
            unsigned char par2;
        } amp;
        unsigned char autoscale; //if the scale of the harmonic profile is computed automaticaly
        unsigned char onehalf;   //what part of the base function is used to make the distribution
    } Php;

    unsigned int PBandwidth; //the values are from 0 to 1000
    unsigned char Pbwscale;  //how the bandwidth is increased according to the harmonic's frequency

    struct
    { //where are positioned the harmonics (on integer multimplier or different places)
        unsigned char type;
        unsigned char par1, par2, par3; //0..255
    } Phrpos;

    struct
    { //quality of the samples (how many samples, the length of them,etc.)
        unsigned char samplesize;
        unsigned char basenote, oct, smpoct;
    } Pquality;

    /***************************
    *   AMPLITUDE PARAMETERS   *
    ***************************/

    /** Amplitude LFO */
    LFOParams *AmpLfo = nullptr;

    unsigned char PPunchStrength;
    unsigned char PPunchTime;
    unsigned char PPunchStretch;
    unsigned char PPunchVelocitySensing;

    /*************************
    *   FILTER PARAMETERS    *
    *************************/

    /** LFO Envelope */
    LFOParams *FilterLfo = nullptr;

    /** Oscillator */
    OscilGen *oscilgen = nullptr;

    Resonance *resonance = nullptr;

    struct
    {
        int size;
        float basefreq;
        float *smp;
    } sample[PAD_MAX_SAMPLES], newsample;

private:
    void generatespectrum_bandwidthMode(
        float *spectrum,
        int size,
        float basefreq,
        const float *profile,
        int profilesize,
        float bwadjust);

    void generatespectrum_otherModes(
        float *spectrum,
        int size,
        float basefreq);

    void deletesamples();

    void deletesample(
        int n);
};

#endif
