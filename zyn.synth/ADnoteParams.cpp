/*
  ZynAddSubFX - a software synthesizer

  ADnoteParameters.cpp - Parameters for ADnote (ADsynth)
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

#include "ADnoteParams.h"
#include "EnvelopeParams.h"
#include "LFOParams.h"
#include "OscilGen.h"
#include "Resonance.h"
#include <cmath>
#include <zyn.common/IPresetsSerializer.h>
#include <zyn.dsp/FilterParams.h>

int ADnote_unison_sizes[] = {1, 2, 3, 4, 5, 6, 8, 10, 12, 15, 20, 25, 30, 40, 50, 0};

ADnoteParameters::ADnoteParameters()
{
    setpresettype("Padsynth");

    FreqEnvelope = EnvelopeParams::ASRinit(0, 0, 64, 50, 64, 60);
    FreqLfo = new LFOParams(70, 0, 64, 0, 0, 0, 0, 0);

    AmpEnvelope = EnvelopeParams::ADSRinit_dB(64, 1, 0, 40, 127, 25);
    AmpLfo = new LFOParams(80, 0, 64, 0, 0, 0, 0, 1);

    GlobalFilter = new FilterParams(2, 94, 40);
    FilterEnvelope = EnvelopeParams::ADSRinit_filter(0, 1, 64, 40, 64, 70, 60, 64);
    FilterLfo = new LFOParams(80, 0, 64, 0, 0, 0, 0, 2);
    Reson = new Resonance();

    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        EnableVoice(nvoice);
    }

    Defaults();
}

ADnoteParameters::~ADnoteParameters()
{
    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        DisableVoice(nvoice);
    }

    delete FreqEnvelope;
    delete FreqLfo;
    delete AmpEnvelope;
    delete AmpLfo;
    delete GlobalFilter;
    delete FilterEnvelope;
    delete FilterLfo;
    delete Reson;
}

void ADnoteParameters::Defaults()
{
    /* Frequency Global Parameters */
    PStereo = 1;    //stereo
    PDetune = 8192; //zero
    PCoarseDetune = 0;
    PDetuneType = 1;
    FreqEnvelope->Defaults();
    FreqLfo->Defaults();
    PBandwidth = 64;

    /* Amplitude Global Parameters */
    PVolume = 90;
    PPanning = 64; //center
    PAmpVelocityScaleFunction = 64;
    AmpEnvelope->Defaults();
    AmpLfo->Defaults();
    PPunchStrength = 0;
    PPunchTime = 60;
    PPunchStretch = 64;
    PPunchVelocitySensing = 72;
    Hrandgrouping = 0;

    /* Filter Global Parameters*/
    PFilterVelocityScale = 64;
    PFilterVelocityScaleFunction = 64;
    GlobalFilter->Defaults();
    FilterEnvelope->Defaults();
    FilterLfo->Defaults();
    Reson->Defaults();

    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        VoicePar[nvoice].Defaults();
    }

    VoicePar[0].Enabled = 1;
}

void ADnoteParameters::InitPresets()
{
    _presets.clear();

    AddPresetAsBool("stereo", &PStereo);

    Preset amplitudeParameters("AMPLITUDE_PARAMETERS");
    {
        amplitudeParameters.AddPreset("volume", &PVolume);
        amplitudeParameters.AddPreset("panning", &PPanning);
        amplitudeParameters.AddPreset("velocity_sensing", &PAmpVelocityScaleFunction);
        amplitudeParameters.AddPreset("punch_strength", &PPunchStrength);
        amplitudeParameters.AddPreset("punch_time", &PPunchTime);
        amplitudeParameters.AddPreset("punch_stretch", &PPunchStretch);
        amplitudeParameters.AddPreset("punch_velocity_sensing", &PPunchVelocitySensing);
        amplitudeParameters.AddPreset("harmonic_randomness_grouping", &Hrandgrouping);

        AmpEnvelope->InitPresets();
        amplitudeParameters.AddContainer(Preset("AMPLITUDE_ENVELOPE", *AmpEnvelope));

        AmpLfo->InitPresets();
        amplitudeParameters.AddContainer(Preset("AMPLITUDE_LFO", *AmpLfo));
    }
    AddContainer(amplitudeParameters);

    Preset frequencyParameters("FREQUENCY_PARAMETERS");
    {
        frequencyParameters.AddPreset("detune", &PDetune);

        frequencyParameters.AddPreset("coarse_detune", &PCoarseDetune);
        frequencyParameters.AddPreset("detune_type", &PDetuneType);

        frequencyParameters.AddPreset("bandwidth", &PBandwidth);

        FreqEnvelope->InitPresets();
        frequencyParameters.AddContainer(Preset("FREQUENCY_ENVELOPE", *FreqEnvelope));

        FreqLfo->InitPresets();
        frequencyParameters.AddContainer(Preset("FREQUENCY_LFO", *FreqLfo));
    }
    AddContainer(frequencyParameters);

    Preset filterParameters("FILTER_PARAMETERS");
    {
        filterParameters.AddPreset("velocity_sensing_amplitude", &PFilterVelocityScale);
        filterParameters.AddPreset("velocity_sensing", &PFilterVelocityScaleFunction);

        GlobalFilter->InitPresets();
        filterParameters.AddContainer(Preset("FILTER", *GlobalFilter));

        FilterEnvelope->InitPresets();
        filterParameters.AddContainer(Preset("FILTER_ENVELOPE", *FilterEnvelope));

        FilterLfo->InitPresets();
        filterParameters.AddContainer(Preset("FILTER_LFO", *FilterLfo));
    }
    AddContainer(filterParameters);

    Reson->InitPresets();
    AddContainer(Preset("RESONANCE", *Reson));

    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        VoicePar[nvoice].InitPresets();
        AddContainer(Preset("VOICE", nvoice, VoicePar[nvoice]));
    }
}

/*
 * Init the voice parameters
 */
void ADnoteParameters::EnableVoice(int nvoice)
{
    VoicePar[nvoice].Enable(Reson);
}

/*
 * Get the Multiplier of the fine detunes of the voices
 */
float ADnoteParameters::getBandwidthDetuneMultiplier()
{
    float bw = (PBandwidth - 64.0f) / 64.0f;

    bw = powf(2.0f, bw * powf(std::fabs(bw), 0.2f) * 5.0f);

    return bw;
}

/*
 * Get the unison spread in cents for a voice
 */

float ADnoteParameters::getUnisonFrequencySpreadCents(int nvoice)
{
    float unison_spread = VoicePar[nvoice].Unison_frequency_spread / 127.0f;
    unison_spread = powf(unison_spread * 2.0f, 2.0f) * 50.0f; //cents
    return unison_spread;
}

/*
 * Kill the voice
 */
void ADnoteParameters::DisableVoice(int nvoice)
{
    VoicePar[nvoice].Disable();
}

//void ADnoteParameters::InitPresets()
//{
//    AddPreset("stereo", &GlobalPar.PStereo);

//    AddContainer(PresetContainer("AMPLITUDE_PARAMETERS")
//                 .AddPreset("volume", &GlobalPar.PVolume)
//                 .AddPreset("panning", &GlobalPar.PPanning)
//                 .AddPreset("velocity_sensing", &GlobalPar.PAmpVelocityScaleFunction)
//                 .AddPreset("punch_strength", &GlobalPar.PPunchStrength)
//                 .AddPreset("punch_time", &GlobalPar.PPunchTime)
//                 .AddPreset("punch_stretch", &GlobalPar.PPunchStretch)
//                 .AddPreset("punch_velocity_sensing", &GlobalPar.PPunchVelocitySensing)
//                 .AddPreset("harmonic_randomness_grouping", &GlobalPar.Hrandgrouping)
//                 .AddContainer("AMPLITUDE_ENVELOPE", *GlobalPar.AmpEnvelope)
//                 );
//}

int ADnoteParameters::get_unison_size_index(int nvoice)
{
    int index = 0;

    if (nvoice >= NUM_VOICES)
    {
        return 0;
    }

    int unison = VoicePar[nvoice].Unison_size;

    while (true)
    {
        if (ADnote_unison_sizes[index] >= unison)
        {
            return index;
        }

        if (ADnote_unison_sizes[index] == 0)
        {
            return index - 1;
        }

        index++;
    }
}

void ADnoteParameters::set_unison_size_index(int nvoice, int index)
{
    int unison = 1;
    for (int i = 0; i <= index; ++i)
    {
        unison = ADnote_unison_sizes[i];
        if (unison == 0)
        {
            unison = ADnote_unison_sizes[i - 1];
            break;
        }
    }

    VoicePar[nvoice].Unison_size = static_cast<unsigned char>(unison);
}
