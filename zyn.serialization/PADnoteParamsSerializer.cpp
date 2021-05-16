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

#include "EnvelopeParamsSerializer.h"
#include "FilterParamsSerializer.h"
#include "LFOParamsSerializer.h"
#include "OscilGenSerializer.h"
#include "ResonanceSerializer.h"

char const *PADnoteParametersSerializer::BRANCH_NAME = "PAD_SYNTH_PARAMETERS";

PADnoteParametersSerializer::PADnoteParametersSerializer(PADnoteParameters *parameters)
    : _parameters(parameters)
{}

PADnoteParametersSerializer::~PADnoteParametersSerializer() = default;

void PADnoteParametersSerializer::Serialize(IPresetsSerializer *xml)
{
    xml->setPadSynth(true);

    xml->addparbool("stereo", _parameters->PStereo);
    xml->addpar("mode", _parameters->Pmode);
    xml->addpar("bandwidth", _parameters->PBandwidth);
    xml->addpar("bandwidth_scale", _parameters->Pbwscale);

    xml->beginbranch("HARMONIC_PROFILE");
    {
        xml->addpar("base_type", _parameters->Php.base.type);
        xml->addpar("base_par1", _parameters->Php.base.par1);
        xml->addpar("frequency_multiplier", _parameters->Php.freqmult);
        xml->addpar("modulator_par1", _parameters->Php.modulator.par1);
        xml->addpar("modulator_frequency", _parameters->Php.modulator.freq);
        xml->addpar("width", _parameters->Php.width);
        xml->addpar("amplitude_multiplier_type", _parameters->Php.amp.type);
        xml->addpar("amplitude_multiplier_mode", _parameters->Php.amp.mode);
        xml->addpar("amplitude_multiplier_par1", _parameters->Php.amp.par1);
        xml->addpar("amplitude_multiplier_par2", _parameters->Php.amp.par2);
        xml->addparbool("autoscale", _parameters->Php.autoscale);
        xml->addpar("one_half", _parameters->Php.onehalf);
    }
    xml->endbranch();

    xml->beginbranch("OSCIL");
    {
        OscilGenSerializer(_parameters->oscilgen).Serialize(xml);
    }
    xml->endbranch();

    xml->beginbranch("RESONANCE");
    {
        ResonanceSerializer(_parameters->resonance).Serialize(xml);
    }
    xml->endbranch();

    xml->beginbranch("HARMONIC_POSITION");
    {
        xml->addpar("type", _parameters->Phrpos.type);
        xml->addpar("parameter1", _parameters->Phrpos.par1);
        xml->addpar("parameter2", _parameters->Phrpos.par2);
        xml->addpar("parameter3", _parameters->Phrpos.par3);
    }
    xml->endbranch();

    xml->beginbranch("SAMPLE_QUALITY");
    {
        xml->addpar("samplesize", _parameters->Pquality.samplesize);
        xml->addpar("basenote", _parameters->Pquality.basenote);
        xml->addpar("octaves", _parameters->Pquality.oct);
        xml->addpar("samples_per_octave", _parameters->Pquality.smpoct);
    }
    xml->endbranch();

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    {
        xml->addpar("volume", _parameters->PVolume);
        xml->addpar("panning", _parameters->PPanning);
        xml->addpar("velocity_sensing", _parameters->PAmpVelocityScaleFunction);
        xml->addpar("punch_strength", _parameters->PPunchStrength);
        xml->addpar("punch_time", _parameters->PPunchTime);
        xml->addpar("punch_stretch", _parameters->PPunchStretch);
        xml->addpar("punch_velocity_sensing", _parameters->PPunchVelocitySensing);

        xml->beginbranch("AMPLITUDE_ENVELOPE");
        {
            EnvelopeParamsSerializer(_parameters->AmpEnvelope).Serialize(xml);
        }
        xml->endbranch();

        xml->beginbranch("AMPLITUDE_LFO");
        {
            LFOParamsSerializer(_parameters->AmpLfo).Serialize(xml);
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    {
        xml->addpar("fixed_freq", _parameters->Pfixedfreq);
        xml->addpar("fixed_freq_et", _parameters->PfixedfreqET);
        xml->addpar("detune", _parameters->PDetune);
        xml->addpar("coarse_detune", _parameters->PCoarseDetune);
        xml->addpar("detune_type", _parameters->PDetuneType);

        xml->beginbranch("FREQUENCY_ENVELOPE");
        {
            EnvelopeParamsSerializer(_parameters->FreqEnvelope).Serialize(xml);
        }
        xml->endbranch();

        xml->beginbranch("FREQUENCY_LFO");
        {
            LFOParamsSerializer(_parameters->FreqLfo).Serialize(xml);
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FILTER_PARAMETERS");
    {
        xml->addpar("velocity_sensing_amplitude", _parameters->PFilterVelocityScale);
        xml->addpar("velocity_sensing", _parameters->PFilterVelocityScaleFunction);

        xml->beginbranch("FILTER");
        {
            FilterParamsSerializer(_parameters->GlobalFilter).Serialize(xml);
        }
        xml->endbranch();

        xml->beginbranch("FILTER_ENVELOPE");
        {
            EnvelopeParamsSerializer(_parameters->FilterEnvelope).Serialize(xml);
        }
        xml->endbranch();

        xml->beginbranch("FILTER_LFO");
        {
            LFOParamsSerializer(_parameters->FilterLfo).Serialize(xml);
        }
        xml->endbranch();
    }
    xml->endbranch();
}

void PADnoteParametersSerializer::Deserialize(IPresetsSerializer *xml)
{
    _parameters->PStereo = xml->getparbool("stereo", _parameters->PStereo);
    _parameters->Pmode = xml->getpar127("mode", 0);
    _parameters->PBandwidth = xml->getpar("bandwidth", _parameters->PBandwidth, 0, 1000);
    _parameters->Pbwscale = xml->getpar127("bandwidth_scale", _parameters->Pbwscale);

    if (xml->enterbranch("HARMONIC_PROFILE"))
    {
        _parameters->Php.base.type = xml->getpar127("base_type", _parameters->Php.base.type);
        _parameters->Php.base.par1 = xml->getpar127("base_par1", _parameters->Php.base.par1);
        _parameters->Php.freqmult = xml->getpar127("frequency_multiplier", _parameters->Php.freqmult);
        _parameters->Php.modulator.par1 = xml->getpar127("modulator_par1", _parameters->Php.modulator.par1);
        _parameters->Php.modulator.freq = xml->getpar127("modulator_frequency", _parameters->Php.modulator.freq);
        _parameters->Php.width = xml->getpar127("width", _parameters->Php.width);
        _parameters->Php.amp.type = xml->getpar127("amplitude_multiplier_type", _parameters->Php.amp.type);
        _parameters->Php.amp.mode = xml->getpar127("amplitude_multiplier_mode", _parameters->Php.amp.mode);
        _parameters->Php.amp.par1 = xml->getpar127("amplitude_multiplier_par1", _parameters->Php.amp.par1);
        _parameters->Php.amp.par2 = xml->getpar127("amplitude_multiplier_par2", _parameters->Php.amp.par2);
        _parameters->Php.autoscale = xml->getparbool("autoscale", _parameters->Php.autoscale);
        _parameters->Php.onehalf = xml->getpar127("one_half", _parameters->Php.onehalf);
        xml->exitbranch();
    }

    if (xml->enterbranch("OSCIL"))
    {
        OscilGenSerializer(_parameters->oscilgen).Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("RESONANCE"))
    {
        ResonanceSerializer(_parameters->resonance).Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("HARMONIC_POSITION"))
    {
        _parameters->Phrpos.type = xml->getpar127("type", _parameters->Phrpos.type);
        _parameters->Phrpos.par1 = xml->getpar("parameter1", _parameters->Phrpos.par1, 0, 255);
        _parameters->Phrpos.par2 = xml->getpar("parameter2", _parameters->Phrpos.par2, 0, 255);
        _parameters->Phrpos.par3 = xml->getpar("parameter3", _parameters->Phrpos.par3, 0, 255);
        xml->exitbranch();
    }

    if (xml->enterbranch("SAMPLE_QUALITY"))
    {
        _parameters->Pquality.samplesize = xml->getpar127("samplesize", _parameters->Pquality.samplesize);
        _parameters->Pquality.basenote = xml->getpar127("basenote", _parameters->Pquality.basenote);
        _parameters->Pquality.oct = xml->getpar127("octaves", _parameters->Pquality.oct);
        _parameters->Pquality.smpoct = xml->getpar127("samples_per_octave", _parameters->Pquality.smpoct);
        xml->exitbranch();
    }

    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        _parameters->PVolume = xml->getpar127("volume", _parameters->PVolume);
        _parameters->PPanning = xml->getpar127("panning", _parameters->PPanning);
        _parameters->PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", _parameters->PAmpVelocityScaleFunction);
        _parameters->PPunchStrength = xml->getpar127("punch_strength", _parameters->PPunchStrength);
        _parameters->PPunchTime = xml->getpar127("punch_time", _parameters->PPunchTime);
        _parameters->PPunchStretch = xml->getpar127("punch_stretch", _parameters->PPunchStretch);
        _parameters->PPunchVelocitySensing = xml->getpar127("punch_velocity_sensing", _parameters->PPunchVelocitySensing);

        xml->enterbranch("AMPLITUDE_ENVELOPE");
        EnvelopeParamsSerializer(_parameters->AmpEnvelope).Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("AMPLITUDE_LFO");
        LFOParamsSerializer(_parameters->AmpLfo).Deserialize(xml);
        xml->exitbranch();

        xml->exitbranch();
    }

    if (xml->enterbranch("FREQUENCY_PARAMETERS"))
    {
        _parameters->Pfixedfreq = xml->getpar127("fixed_freq", _parameters->Pfixedfreq);
        _parameters->PfixedfreqET = xml->getpar127("fixed_freq_et", _parameters->PfixedfreqET);
        _parameters->PDetune = xml->getpar("detune", _parameters->PDetune, 0, 16383);
        _parameters->PCoarseDetune = xml->getpar("coarse_detune", _parameters->PCoarseDetune, 0, 16383);
        _parameters->PDetuneType = xml->getpar127("detune_type", _parameters->PDetuneType);

        xml->enterbranch("FREQUENCY_ENVELOPE");
        EnvelopeParamsSerializer(_parameters->FreqEnvelope).Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FREQUENCY_LFO");
        LFOParamsSerializer(_parameters->FreqLfo).Deserialize(xml);
        xml->exitbranch();
        xml->exitbranch();
    }

    if (xml->enterbranch("FILTER_PARAMETERS"))
    {
        _parameters->PFilterVelocityScale = xml->getpar127("velocity_sensing_amplitude", _parameters->PFilterVelocityScale);
        _parameters->PFilterVelocityScaleFunction = xml->getpar127("velocity_sensing", _parameters->PFilterVelocityScaleFunction);

        xml->enterbranch("FILTER");
        FilterParamsSerializer(_parameters->GlobalFilter).Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FILTER_ENVELOPE");
        EnvelopeParamsSerializer(_parameters->FilterEnvelope).Deserialize(xml);
        xml->exitbranch();

        xml->enterbranch("FILTER_LFO");
        LFOParamsSerializer(_parameters->FilterLfo).Deserialize(xml);
        xml->exitbranch();
        xml->exitbranch();
    }
}
