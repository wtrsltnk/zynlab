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
#include <zyn.common/IFFTwrapper.h>
#include <zyn.common/IPresetsSerializer.h>
#include <zyn.dsp/FilterParams.h>

int ADnote_unison_sizes[] = {1, 2, 3, 4, 5, 6, 8, 10, 12, 15, 20, 25, 30, 40, 50, 0};

ADnoteParameters::ADnoteParameters(IFFTwrapper *fft)
    : _fft(fft), GlobalPar()
{
    setpresettype("Padsynth");

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
}

void ADnoteParameters::Defaults()
{
    //Default Parameters
    GlobalPar.Defaults();

    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        VoicePar[nvoice].Defaults();
    }

    VoicePar[0].Enabled = 1;
}

void ADnoteParameters::Serialize(IPresetsSerializer *xml)
{
    GlobalPar.Serialize(xml);

    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        xml->beginbranch("VOICE", nvoice);
        bool oscilused = false, fmoscilused = false; //if the oscil or fmoscil are used by another voice

        for (auto &i : VoicePar)
        {
            if (i.Pextoscil == nvoice)
            {
                oscilused = true;
            }
            if (i.PextFMoscil == nvoice)
            {
                fmoscilused = true;
            }
        }

        xml->addparbool("enabled", VoicePar[nvoice].Enabled);

        if (VoicePar[nvoice].Enabled || oscilused || fmoscilused || !xml->minimal)
        {
            VoicePar[nvoice].Serialize(xml, fmoscilused);
        }

        xml->endbranch();
    }
}

void ADnoteParameters::Deserialize(IPresetsSerializer *xml)
{
    GlobalPar.Deserialize(xml);

    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        VoicePar[nvoice].Enabled = 0;
        if (xml->enterbranch("VOICE", nvoice) == 0)
        {
            continue;
        }
        VoicePar[nvoice].Deserialize(xml, static_cast<unsigned char>(nvoice));
        xml->exitbranch();
    }
}

/*
 * Init the voice parameters
 */
void ADnoteParameters::EnableVoice(int nvoice)
{
    VoicePar[nvoice].Enable(_fft, GlobalPar.Reson);
}

/*
 * Get the Multiplier of the fine detunes of the voices
 */
float ADnoteParameters::getBandwidthDetuneMultiplier()
{
    float bw = (GlobalPar.PBandwidth - 64.0f) / 64.0f;

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
