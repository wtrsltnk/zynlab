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

#include "ADnoteParamsSerializer.h"

#include "ADnoteVoiceParamSerializer.h"
#include "EnvelopeParamsSerializer.h"
#include "FilterParamsSerializer.h"
#include "LFOParamsSerializer.h"
#include "ResonanceSerializer.h"

char const *ADnoteParametersSerializer::BRANCH_NAME = "ADD_SYNTH_PARAMETERS";

ADnoteParametersSerializer::ADnoteParametersSerializer(ADnoteParameters *parameters)
    : _parameters(parameters)
{}

ADnoteParametersSerializer::~ADnoteParametersSerializer() = default;

void ADnoteParametersSerializer::Serialize(IPresetsSerializer *xml)
{
    xml->addparbool("stereo", _parameters->PStereo);

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    {
        xml->addpar("volume", _parameters->PVolume);
        xml->addpar("panning", _parameters->PPanning);
        xml->addpar("velocity_sensing", _parameters->PAmpVelocityScaleFunction);
        xml->addpar("punch_strength", _parameters->PPunchStrength);
        xml->addpar("punch_time", _parameters->PPunchTime);
        xml->addpar("punch_stretch", _parameters->PPunchStretch);
        xml->addpar("punch_velocity_sensing", _parameters->PPunchVelocitySensing);
        xml->addpar("harmonic_randomness_grouping", _parameters->Hrandgrouping);

        xml->beginbranch("AMPLITUDE_ENVELOPE");
        EnvelopeParamsSerializer(_parameters->AmpEnvelope).Serialize(xml);
        xml->endbranch();

        xml->beginbranch("AMPLITUDE_LFO");
        LFOParamsSerializer(_parameters->AmpLfo).Serialize(xml);
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    {
        xml->addpar("detune", _parameters->PDetune);

        xml->addpar("coarse_detune", _parameters->PCoarseDetune);
        xml->addpar("detune_type", _parameters->PDetuneType);

        xml->addpar("bandwidth", _parameters->PBandwidth);

        xml->beginbranch("FREQUENCY_ENVELOPE");
        EnvelopeParamsSerializer(_parameters->FreqEnvelope).Serialize(xml);
        xml->endbranch();

        xml->beginbranch("FREQUENCY_LFO");
        LFOParamsSerializer(_parameters->FreqLfo).Serialize(xml);
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FILTER_PARAMETERS");
    {
        xml->addpar("velocity_sensing_amplitude", _parameters->PFilterVelocityScale);
        xml->addpar("velocity_sensing", _parameters->PFilterVelocityScaleFunction);

        xml->beginbranch("FILTER");
        FilterParamsSerializer(_parameters->GlobalFilter).Serialize(xml);
        xml->endbranch();

        xml->beginbranch("FILTER_ENVELOPE");
        EnvelopeParamsSerializer(_parameters->FilterEnvelope).Serialize(xml);
        xml->endbranch();

        xml->beginbranch("FILTER_LFO");
        LFOParamsSerializer(_parameters->FilterLfo).Serialize(xml);
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("RESONANCE");
    ResonanceSerializer(_parameters->Reson).Serialize(xml);
    xml->endbranch();

    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        xml->beginbranch("VOICE", nvoice);
        bool oscilused = false, fmoscilused = false; //if the oscil or fmoscil are used by another voice

        for (auto &i : _parameters->VoicePar)
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

        xml->addparbool("enabled", _parameters->VoicePar[nvoice].Enabled);

        if (_parameters->VoicePar[nvoice].Enabled || oscilused || fmoscilused || !xml->minimal)
        {
            ADnoteVoiceParamSerializer(&_parameters->VoicePar[nvoice]).Serialize(xml, fmoscilused);
        }

        xml->endbranch();
    }
}

void ADnoteParametersSerializer::Deserialize(IPresetsSerializer *xml)
{
    _parameters->PStereo = xml->getparbool("stereo", _parameters->PStereo);

    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        _parameters->PVolume = xml->getpar127("volume", _parameters->PVolume);
        _parameters->PPanning = xml->getpar127("panning", _parameters->PPanning);
        _parameters->PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", _parameters->PAmpVelocityScaleFunction);

        _parameters->PPunchStrength = xml->getpar127("punch_strength", _parameters->PPunchStrength);
        _parameters->PPunchTime = xml->getpar127("punch_time", _parameters->PPunchTime);
        _parameters->PPunchStretch = xml->getpar127("punch_stretch", _parameters->PPunchStretch);
        _parameters->PPunchVelocitySensing = xml->getpar127("punch_velocity_sensing", _parameters->PPunchVelocitySensing);
        _parameters->Hrandgrouping = xml->getpar127("harmonic_randomness_grouping", _parameters->Hrandgrouping);

        if (xml->enterbranch("AMPLITUDE_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->AmpEnvelope).Deserialize(xml);
            xml->exitbranch();
        }

        if (xml->enterbranch("AMPLITUDE_LFO"))
        {
            LFOParamsSerializer(_parameters->AmpLfo).Deserialize(xml);
            xml->exitbranch();
        }

        xml->exitbranch();
    }

    if (xml->enterbranch("FREQUENCY_PARAMETERS"))
    {
        _parameters->PDetune = xml->getpar("detune", _parameters->PDetune, 0, 16383);
        _parameters->PCoarseDetune = xml->getpar("coarse_detune", _parameters->PCoarseDetune, 0, 16383);
        _parameters->PDetuneType = xml->getpar127("detune_type", _parameters->PDetuneType);
        _parameters->PBandwidth = xml->getpar127("bandwidth", _parameters->PBandwidth);

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

    if (xml->enterbranch("RESONANCE"))
    {
        ResonanceSerializer(_parameters->Reson).Deserialize(xml);
        xml->exitbranch();
    }

    for (int nvoice = 0; nvoice < NUM_VOICES; ++nvoice)
    {
        _parameters->VoicePar[nvoice].Enabled = 0;
        if (xml->enterbranch("VOICE", nvoice) == 0)
        {
            continue;
        }
        ADnoteVoiceParamSerializer(&_parameters->VoicePar[nvoice]).Deserialize(xml, static_cast<unsigned char>(nvoice));
        xml->exitbranch();
    }
}
