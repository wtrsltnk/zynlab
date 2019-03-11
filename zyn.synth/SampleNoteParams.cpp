/*
  ZynAddSubFX - a software synthesizer

  SampleNoteParameters.cpp - Parameters for SUBnote (SUBsynth)
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

#include "SampleNoteParams.h"
#include <cmath>

SampleNoteParameters::SampleNoteParameters()
{
    setpresettype("Psubsynth");
    AmpEnvelope = EnvelopeParams::ADSRinit_dB(64, 1, 0, 40, 127, 25);
    FreqEnvelope = EnvelopeParams::ASRinit(64, 0, 30, 50, 64, 60);
    BandWidthEnvelope = EnvelopeParams::ASRinit_bw(64, 0, 100, 70, 64, 60);

    GlobalFilter = new FilterParams(2, 80, 40);
    GlobalFilterEnvelope = EnvelopeParams::ADSRinit_filter(0, 1, 64, 40, 64, 70, 60, 64);

    Defaults();
}

SampleNoteParameters::~SampleNoteParameters()
{
    delete (AmpEnvelope);
    delete (FreqEnvelope);
    delete (BandWidthEnvelope);
    delete (GlobalFilter);
    delete (GlobalFilterEnvelope);
}

void SampleNoteParameters::Defaults()
{
    PVolume = 96;
    PPanning = 64;
    PAmpVelocityScaleFunction = 90;

    Pfixedfreq = 0;
    PfixedfreqET = 0;
    Pnumstages = 2;
    Pbandwidth = 40;
    Phmagtype = 0;
    Pbwscale = 64;
    Pstereo = 1;
    Pstart = 1;

    PDetune = 8192;
    PCoarseDetune = 0;
    PDetuneType = 1;
    PFreqEnvelopeEnabled = 0;
    PBandWidthEnvelopeEnabled = 0;

    POvertoneSpread.type = 0;
    POvertoneSpread.par1 = 0;
    POvertoneSpread.par2 = 0;
    POvertoneSpread.par3 = 0;
    updateFrequencyMultipliers();

    for (int n = 0; n < MAX_SUB_HARMONICS; ++n)
    {
        Phmag[n] = 0;
        Phrelbw[n] = 64;
    }
    Phmag[0] = 127;

    PGlobalFilterEnabled = 0;
    PGlobalFilterVelocityScale = 64;
    PGlobalFilterVelocityScaleFunction = 64;

    AmpEnvelope->Defaults();
    FreqEnvelope->Defaults();
    BandWidthEnvelope->Defaults();
    GlobalFilter->Defaults();
    GlobalFilterEnvelope->Defaults();
}

void SampleNoteParameters::InitPresets()
{
    _presets.clear();

    AddPreset("num_stages", &Pnumstages);
    AddPreset("harmonic_mag_type", &Phmagtype);
    AddPreset("start", &Pstart);

    Preset harmonics("HARMONICS");
    for (int i = 0; i < MAX_SUB_HARMONICS; ++i)
    {
        Preset harmonic("HARMONIC", i);
        harmonic.AddPreset("mag", &Phmag[i]);
        harmonic.AddPreset("relbw", &Phrelbw[i]);
        harmonics.AddContainer(harmonic);
    }
    AddContainer(harmonics);

    Preset amplitudeParameters("AMPLITUDE_PARAMETERS");
    {
        amplitudeParameters.AddPresetAsBool("stereo", &Pstereo);
        amplitudeParameters.AddPreset("volume", &PVolume);
        amplitudeParameters.AddPreset("panning", &PPanning);
        amplitudeParameters.AddPreset("velocity_sensing", &PAmpVelocityScaleFunction);
        AmpEnvelope->InitPresets();
        amplitudeParameters.AddContainer(Preset("AMPLITUDE_ENVELOPE", *AmpEnvelope));
    }
    AddContainer(amplitudeParameters);

    Preset frequencyParameters("FREQUENCY_PARAMETERS");
    {
        frequencyParameters.AddPresetAsBool("fixed_freq", &Pfixedfreq);
        frequencyParameters.AddPreset("fixed_freq_et", &PfixedfreqET);

        frequencyParameters.AddPreset("detune", &PDetune);
        frequencyParameters.AddPreset("coarse_detune", &PCoarseDetune);
        frequencyParameters.AddPreset("overtone_spread_type", &POvertoneSpread.type);
        frequencyParameters.AddPreset("overtone_spread_par1", &POvertoneSpread.par1);
        frequencyParameters.AddPreset("overtone_spread_par2", &POvertoneSpread.par2);
        frequencyParameters.AddPreset("overtone_spread_par3", &POvertoneSpread.par3);
        frequencyParameters.AddPreset("detune_type", &PDetuneType);

        frequencyParameters.AddPreset("bandwidth", &Pbandwidth);
        frequencyParameters.AddPreset("bandwidth_scale", &Pbwscale);

        frequencyParameters.AddPresetAsBool("freq_envelope_enabled", &PFreqEnvelopeEnabled);
        FreqEnvelope->InitPresets();
        frequencyParameters.AddContainer(Preset("FREQUENCY_ENVELOPE", *FreqEnvelope));

        frequencyParameters.AddPresetAsBool("band_width_envelope_enabled", &PBandWidthEnvelopeEnabled);
        BandWidthEnvelope->InitPresets();
        frequencyParameters.AddContainer(Preset("BANDWIDTH_ENVELOPE", *BandWidthEnvelope));
    }
    AddContainer(frequencyParameters);

    Preset filterParameters("FILTER_PARAMETERS");
    {
        filterParameters.AddPresetAsBool("enabled", &PGlobalFilterEnabled);

        GlobalFilter->InitPresets();
        filterParameters.AddContainer(Preset("FILTER", *GlobalFilter));

        filterParameters.AddPreset("filter_velocity_sensing", &PGlobalFilterVelocityScaleFunction);
        filterParameters.AddPreset("filter_velocity_sensing_amplitude", &PGlobalFilterVelocityScale);

        GlobalFilterEnvelope->InitPresets();
        filterParameters.AddContainer(Preset("FILTER_ENVELOPE", *GlobalFilterEnvelope));
    }
    AddContainer(filterParameters);
}

void SampleNoteParameters::Serialize(IPresetsSerializer *xml)
{
    xml->addpar("num_stages", Pnumstages);
    xml->addpar("harmonic_mag_type", Phmagtype);
    xml->addpar("start", Pstart);

    xml->beginbranch("HARMONICS");
    for (int i = 0; i < MAX_SUB_HARMONICS; ++i)
    {
        if ((Phmag[i] == 0) && (xml->minimal))
        {
            continue;
        }
        xml->beginbranch("HARMONIC", i);
        xml->addpar("mag", Phmag[i]);
        xml->addpar("relbw", Phrelbw[i]);
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    {
        xml->addparbool("stereo", Pstereo);
        xml->addpar("volume", PVolume);
        xml->addpar("panning", PPanning);
        xml->addpar("velocity_sensing", PAmpVelocityScaleFunction);
        xml->beginbranch("AMPLITUDE_ENVELOPE");
        {
            AmpEnvelope->Serialize(xml);
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    {
        xml->addparbool("fixed_freq", Pfixedfreq);
        xml->addpar("fixed_freq_et", PfixedfreqET);

        xml->addpar("detune", PDetune);
        xml->addpar("coarse_detune", PCoarseDetune);
        xml->addpar("overtone_spread_type", POvertoneSpread.type);
        xml->addpar("overtone_spread_par1", POvertoneSpread.par1);
        xml->addpar("overtone_spread_par2", POvertoneSpread.par2);
        xml->addpar("overtone_spread_par3", POvertoneSpread.par3);
        xml->addpar("detune_type", PDetuneType);

        xml->addpar("bandwidth", Pbandwidth);
        xml->addpar("bandwidth_scale", Pbwscale);

        xml->addparbool("freq_envelope_enabled", PFreqEnvelopeEnabled);
        if ((PFreqEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FREQUENCY_ENVELOPE");
            {
                FreqEnvelope->Serialize(xml);
            }
            xml->endbranch();
        }

        xml->addparbool("band_width_envelope_enabled", PBandWidthEnvelopeEnabled);
        if ((PBandWidthEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("BANDWIDTH_ENVELOPE");
            {
                BandWidthEnvelope->Serialize(xml);
            }
            xml->endbranch();
        }
    }
    xml->endbranch();

    xml->beginbranch("FILTER_PARAMETERS");
    {
        xml->addparbool("enabled", PGlobalFilterEnabled);
        if ((PGlobalFilterEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FILTER");
            {
                GlobalFilter->Serialize(xml);
            }
            xml->endbranch();

            xml->addpar("filter_velocity_sensing", PGlobalFilterVelocityScaleFunction);
            xml->addpar("filter_velocity_sensing_amplitude", PGlobalFilterVelocityScale);

            xml->beginbranch("FILTER_ENVELOPE");
            {
                GlobalFilterEnvelope->Serialize(xml);
            }
            xml->endbranch();
        }
    }
    xml->endbranch();
}

void SampleNoteParameters::Deserialize(IPresetsSerializer *xml)
{
    Pnumstages = xml->getpar127("num_stages", Pnumstages);
    Phmagtype = xml->getpar127("harmonic_mag_type", Phmagtype);
    Pstart = xml->getpar127("start", Pstart);

    if (xml->enterbranch("HARMONICS"))
    {
        Phmag[0] = 0;
        for (int i = 0; i < MAX_SUB_HARMONICS; ++i)
        {
            if (xml->enterbranch("HARMONIC", i) == 0)
            {
                continue;
            }
            Phmag[i] = xml->getpar127("mag", Phmag[i]);
            Phrelbw[i] = xml->getpar127("relbw", Phrelbw[i]);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        Pstereo = xml->getparbool("stereo", Pstereo);
        PVolume = xml->getpar127("volume", PVolume);
        PPanning = xml->getpar127("panning", PPanning);
        PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", PAmpVelocityScaleFunction);
        if (xml->enterbranch("AMPLITUDE_ENVELOPE"))
        {
            AmpEnvelope->Deserialize(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("FREQUENCY_PARAMETERS"))
    {
        Pfixedfreq = xml->getparbool("fixed_freq", Pfixedfreq);
        PfixedfreqET = xml->getpar127("fixed_freq_et", PfixedfreqET);

        PDetune = xml->getpar("detune", PDetune, 0, 16383);
        PCoarseDetune = xml->getpar("coarse_detune", PCoarseDetune, 0, 16383);
        POvertoneSpread.type = xml->getpar127("overtone_spread_type", POvertoneSpread.type);
        POvertoneSpread.par1 = xml->getpar("overtone_spread_par1", POvertoneSpread.par1, 0, 255);
        POvertoneSpread.par2 = xml->getpar("overtone_spread_par2", POvertoneSpread.par2, 0, 255);
        POvertoneSpread.par3 = xml->getpar("overtone_spread_par3", POvertoneSpread.par3, 0, 255);
        updateFrequencyMultipliers();
        PDetuneType = xml->getpar127("detune_type", PDetuneType);

        Pbandwidth = xml->getpar127("bandwidth", Pbandwidth);
        Pbwscale = xml->getpar127("bandwidth_scale", Pbwscale);

        PFreqEnvelopeEnabled = xml->getparbool("freq_envelope_enabled", PFreqEnvelopeEnabled);
        if (xml->enterbranch("FREQUENCY_ENVELOPE"))
        {
            FreqEnvelope->Deserialize(xml);
            xml->exitbranch();
        }

        PBandWidthEnvelopeEnabled = xml->getparbool("band_width_envelope_enabled", PBandWidthEnvelopeEnabled);
        if (xml->enterbranch("BANDWIDTH_ENVELOPE"))
        {
            BandWidthEnvelope->Deserialize(xml);
            xml->exitbranch();
        }

        xml->exitbranch();
    }

    if (xml->enterbranch("FILTER_PARAMETERS"))
    {
        PGlobalFilterEnabled = xml->getparbool("enabled", PGlobalFilterEnabled);
        if (xml->enterbranch("FILTER"))
        {
            GlobalFilter->Deserialize(xml);
            xml->exitbranch();
        }

        PGlobalFilterVelocityScaleFunction = xml->getpar127("filter_velocity_sensing", PGlobalFilterVelocityScaleFunction);
        PGlobalFilterVelocityScale = xml->getpar127("filter_velocity_sensing_amplitude", PGlobalFilterVelocityScale);

        if (xml->enterbranch("FILTER_ENVELOPE"))
        {
            GlobalFilterEnvelope->Deserialize(xml);
            xml->exitbranch();
        }

        xml->exitbranch();
    }
}

void SampleNoteParameters::updateFrequencyMultipliers()
{
    float par1 = POvertoneSpread.par1 / 255.0f;
    float par1pow = powf(10.0f, -(1.0f - POvertoneSpread.par1 / 255.0f) * 3.0f);
    float par2 = POvertoneSpread.par2 / 255.0f;
    float par3 = 1.0f - POvertoneSpread.par3 / 255.0f;
    float result;
    float tmp = 0.0f;
    int thresh = 0;

    for (int n = 0; n < MAX_SUB_HARMONICS; ++n)
    {
        float n1 = n + 1.0f;
        switch (POvertoneSpread.type)
        {
            case 1:
            {
                thresh = static_cast<int>(100.0f * par2 * par2) + 1;
                if (n1 < thresh)
                    result = n1;
                else
                    result = n1 + 8.0f * (n1 - thresh) * par1pow;
                break;
            }
            case 2:
            {
                thresh = static_cast<int>(100.0f * par2 * par2) + 1;
                if (n1 < thresh)
                    result = n1;
                else
                    result = n1 + 0.9f * (thresh - n1) * par1pow;
                break;
            }
            case 3:
            {
                tmp = par1pow * 100.0f + 1.0f;
                result = powf(n / tmp, 1.0f - 0.8f * par2) * tmp + 1.0f;
                break;
            }
            case 4:
            {
                result = n * (1.0f - par1pow) +
                         powf(0.1f * n, 3.0f * par2 + 1.0f) *
                             10.0f * par1pow +
                         1.0f;
                break;
            }
            case 5:
            {
                result = n1 + 2.0f * sinf(n * par2 * par2 * PI * 0.999f) *
                                  std::sqrt(par1pow);
                break;
            }
            case 6:
            {
                tmp = powf(2.0f * par2, 2.0f) + 0.1f;
                result = n * powf(par1 * powf(0.8f * n, tmp) + 1.0f, tmp) +
                         1.0f;
                break;
            }
            case 7:
            {
                result = (n1 + par1) / (par1 + 1);
                break;
            }
            default:
            {
                result = n1;
            }
        }
        float iresult = std::floor(result + 0.5f);
        POvertoneFreqMult[n] = iresult + par3 * (result - iresult);
    }
}
