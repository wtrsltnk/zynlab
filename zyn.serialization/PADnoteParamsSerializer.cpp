/*
  ZynAddSubFX - a software synthesizer

  PADnoteParameters.cpp - Parameters for PADnote (PADsynth)
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

#include "PADnoteParamsSerializer.h"

PADnoteParametersSerializer::PADnoteParametersSerializer(PADnoteParameters *parameters)
    : _parameters(parameters)
{}

PADnoteParametersSerializer::~PADnoteParametersSerializer() = default;

void PADnoteParameters::Serialize(IPresetsSerializer *xml)
{
    xml->setPadSynth(true);

    xml->addparbool("stereo", PStereo);
    xml->addpar("mode", Pmode);
    xml->addpar("bandwidth", Pbandwidth);
    xml->addpar("bandwidth_scale", Pbwscale);

    xml->beginbranch("HARMONIC_PROFILE");
    {
        xml->addpar("base_type", Php.base.type);
        xml->addpar("base_par1", Php.base.par1);
        xml->addpar("frequency_multiplier", Php.freqmult);
        xml->addpar("modulator_par1", Php.modulator.par1);
        xml->addpar("modulator_frequency", Php.modulator.freq);
        xml->addpar("width", Php.width);
        xml->addpar("amplitude_multiplier_type", Php.amp.type);
        xml->addpar("amplitude_multiplier_mode", Php.amp.mode);
        xml->addpar("amplitude_multiplier_par1", Php.amp.par1);
        xml->addpar("amplitude_multiplier_par2", Php.amp.par2);
        xml->addparbool("autoscale", Php.autoscale);
        xml->addpar("one_half", Php.onehalf);
    }
    xml->endbranch();

    xml->beginbranch("OSCIL");
    {
        oscilgen->Serialize(xml);
    }
    xml->endbranch();

    xml->beginbranch("RESONANCE");
    {
        resonance->Serialize(xml);
    }
    xml->endbranch();

    xml->beginbranch("HARMONIC_POSITION");
    {
        xml->addpar("type", Phrpos.type);
        xml->addpar("parameter1", Phrpos.par1);
        xml->addpar("parameter2", Phrpos.par2);
        xml->addpar("parameter3", Phrpos.par3);
    }
    xml->endbranch();

    xml->beginbranch("SAMPLE_QUALITY");
    {
        xml->addpar("samplesize", Pquality.samplesize);
        xml->addpar("basenote", Pquality.basenote);
        xml->addpar("octaves", Pquality.oct);
        xml->addpar("samples_per_octave", Pquality.smpoct);
    }
    xml->endbranch();

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    {
        xml->addpar("volume", PVolume);
        xml->addpar("panning", PPanning);
        xml->addpar("velocity_sensing", PAmpVelocityScaleFunction);
        xml->addpar("punch_strength", PPunchStrength);
        xml->addpar("punch_time", PPunchTime);
        xml->addpar("punch_stretch", PPunchStretch);
        xml->addpar("punch_velocity_sensing", PPunchVelocitySensing);

        xml->beginbranch("AMPLITUDE_ENVELOPE");
        {
            AmpEnvelope->Serialize(xml);
        }
        xml->endbranch();

        xml->beginbranch("AMPLITUDE_LFO");
        {
            AmpLfo->Serialize(xml);
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    {
        xml->addpar("fixed_freq", Pfixedfreq);
        xml->addpar("fixed_freq_et", PfixedfreqET);
        xml->addpar("detune", PDetune);
        xml->addpar("coarse_detune", PCoarseDetune);
        xml->addpar("detune_type", PDetuneType);

        xml->beginbranch("FREQUENCY_ENVELOPE");
        {
            FreqEnvelope->Serialize(xml);
        }
        xml->endbranch();

        xml->beginbranch("FREQUENCY_LFO");
        {
            FreqLfo->Serialize(xml);
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FILTER_PARAMETERS");
    {
        xml->addpar("velocity_sensing_amplitude", PFilterVelocityScale);
        xml->addpar("velocity_sensing", PFilterVelocityScaleFunction);

        xml->beginbranch("FILTER");
        {
            GlobalFilter->Serialize(xml);
        }
        xml->endbranch();

        xml->beginbranch("FILTER_ENVELOPE");
        {
            FilterEnvelope->Serialize(xml);
        }
        xml->endbranch();

        xml->beginbranch("FILTER_LFO");
        {
            FilterLfo->Serialize(xml);
        }
        xml->endbranch();
    }
    xml->endbranch();
}

void PADnoteParameters::Deserialize(IPresetsSerializer *xml)
{
    PStereo = xml->getparbool("stereo", PStereo);
    Pmode = xml->getpar127("mode", 0);
    Pbandwidth = xml->getpar("bandwidth", Pbandwidth, 0, 1000);
    Pbwscale = xml->getpar127("bandwidth_scale", Pbwscale);

    if (xml->enterbranch("HARMONIC_PROFILE"))
    {
        Php.base.type = xml->getpar127("base_type", Php.base.type);
        Php.base.par1 = xml->getpar127("base_par1", Php.base.par1);
        Php.freqmult = xml->getpar127("frequency_multiplier", Php.freqmult);
        Php.modulator.par1 = xml->getpar127("modulator_par1", Php.modulator.par1);
        Php.modulator.freq = xml->getpar127("modulator_frequency", Php.modulator.freq);
        Php.width = xml->getpar127("width", Php.width);
        Php.amp.type = xml->getpar127("amplitude_multiplier_type", Php.amp.type);
        Php.amp.mode = xml->getpar127("amplitude_multiplier_mode", Php.amp.mode);
        Php.amp.par1 = xml->getpar127("amplitude_multiplier_par1", Php.amp.par1);
        Php.amp.par2 = xml->getpar127("amplitude_multiplier_par2", Php.amp.par2);
        Php.autoscale = xml->getparbool("autoscale", Php.autoscale);
        Php.onehalf = xml->getpar127("one_half", Php.onehalf);
        xml->exitbranch();
    }

    if (xml->enterbranch("OSCIL"))
    {
        oscilgen->Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("RESONANCE"))
    {
        resonance->Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("HARMONIC_POSITION"))
    {
        Phrpos.type = xml->getpar127("type", Phrpos.type);
        Phrpos.par1 = xml->getpar("parameter1", Phrpos.par1, 0, 255);
        Phrpos.par2 = xml->getpar("parameter2", Phrpos.par2, 0, 255);
        Phrpos.par3 = xml->getpar("parameter3", Phrpos.par3, 0, 255);
        xml->exitbranch();
    }

    if (xml->enterbranch("SAMPLE_QUALITY"))
    {
        Pquality.samplesize = xml->getpar127("samplesize", Pquality.samplesize);
        Pquality.basenote = xml->getpar127("basenote", Pquality.basenote);
        Pquality.oct = xml->getpar127("octaves", Pquality.oct);
        Pquality.smpoct = xml->getpar127("samples_per_octave", Pquality.smpoct);
        xml->exitbranch();
    }

    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        PVolume = xml->getpar127("volume", PVolume);
        PPanning = xml->getpar127("panning", PPanning);
        PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", PAmpVelocityScaleFunction);
        PPunchStrength = xml->getpar127("punch_strength", PPunchStrength);
        PPunchTime = xml->getpar127("punch_time", PPunchTime);
        PPunchStretch = xml->getpar127("punch_stretch", PPunchStretch);
        PPunchVelocitySensing = xml->getpar127("punch_velocity_sensing", PPunchVelocitySensing);

        xml->enterbranch("AMPLITUDE_ENVELOPE");
        AmpEnvelope->Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("AMPLITUDE_LFO");
        AmpLfo->Deserialize(xml);
        xml->exitbranch();

        xml->exitbranch();
    }

    if (xml->enterbranch("FREQUENCY_PARAMETERS"))
    {
        Pfixedfreq = xml->getpar127("fixed_freq", Pfixedfreq);
        PfixedfreqET = xml->getpar127("fixed_freq_et", PfixedfreqET);
        PDetune = xml->getpar("detune", PDetune, 0, 16383);
        PCoarseDetune = xml->getpar("coarse_detune", PCoarseDetune, 0, 16383);
        PDetuneType = xml->getpar127("detune_type", PDetuneType);

        xml->enterbranch("FREQUENCY_ENVELOPE");
        FreqEnvelope->Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FREQUENCY_LFO");
        FreqLfo->Deserialize(xml);
        xml->exitbranch();
        xml->exitbranch();
    }

    if (xml->enterbranch("FILTER_PARAMETERS"))
    {
        PFilterVelocityScale = xml->getpar127("velocity_sensing_amplitude", PFilterVelocityScale);
        PFilterVelocityScaleFunction = xml->getpar127("velocity_sensing", PFilterVelocityScaleFunction);

        xml->enterbranch("FILTER");
        GlobalFilter->Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FILTER_ENVELOPE");
        FilterEnvelope->Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FILTER_LFO");
        FilterLfo->Deserialize(xml);
        xml->exitbranch();
        xml->exitbranch();
    }
}
