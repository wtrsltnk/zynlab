/*
  ZynAddSubFX - a software synthesizer

  SUBnoteParameters.cpp - Parameters for SUBnote (SUBsynth)
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

#include "SUBnoteParamsSerializer.h"

#include "EnvelopeParamsSerializer.h"
#include "FilterParamsSerializer.h"

char const *SUBnoteParametersSerializer::BRANCH_NAME = "SUB_SYNTH_PARAMETERS";

SUBnoteParametersSerializer::SUBnoteParametersSerializer(
    SUBnoteParameters *parameters)
    : _parameters(parameters)
{}

SUBnoteParametersSerializer::~SUBnoteParametersSerializer() = default;

void SUBnoteParametersSerializer::Serialize(
    IPresetsSerializer *xml)
{
    xml->addpar("num_stages", _parameters->Pnumstages);
    xml->addpar("harmonic_mag_type", _parameters->Phmagtype);
    xml->addpar("start", _parameters->Pstart);

    xml->beginbranch("HARMONICS");
    for (int i = 0; i < MAX_SUB_HARMONICS; ++i)
    {
        if ((_parameters->Phmag[i] == 0) && (xml->minimal))
        {
            continue;
        }
        xml->beginbranch("HARMONIC", i);
        xml->addpar("mag", _parameters->Phmag[i]);
        xml->addpar("relbw", _parameters->Phrelbw[i]);
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    {
        xml->addparbool("stereo", _parameters->PStereo);
        xml->addpar("volume", _parameters->PVolume);
        xml->addpar("panning", _parameters->PPanning);
        xml->addpar("velocity_sensing", _parameters->PAmpVelocityScaleFunction);
        xml->beginbranch("AMPLITUDE_ENVELOPE");
        {
            EnvelopeParamsSerializer(_parameters->AmpEnvelope).Serialize(xml);
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    {
        xml->addparbool("fixed_freq", _parameters->Pfixedfreq);
        xml->addpar("fixed_freq_et", _parameters->PfixedfreqET);

        xml->addpar("detune", _parameters->PDetune);
        xml->addpar("coarse_detune", _parameters->PCoarseDetune);
        xml->addpar("overtone_spread_type", _parameters->POvertoneSpread.type);
        xml->addpar("overtone_spread_par1", _parameters->POvertoneSpread.par1);
        xml->addpar("overtone_spread_par2", _parameters->POvertoneSpread.par2);
        xml->addpar("overtone_spread_par3", _parameters->POvertoneSpread.par3);
        xml->addpar("detune_type", _parameters->PDetuneType);

        xml->addpar("bandwidth", _parameters->PBandwidth);
        xml->addpar("bandwidth_scale", _parameters->Pbwscale);

        xml->addparbool("freq_envelope_enabled", _parameters->PFreqEnvelopeEnabled);
        if ((_parameters->PFreqEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FREQUENCY_ENVELOPE");
            {
                EnvelopeParamsSerializer(_parameters->FreqEnvelope).Serialize(xml);
            }
            xml->endbranch();
        }

        xml->addparbool("band_width_envelope_enabled", _parameters->PBandWidthEnvelopeEnabled);
        if ((_parameters->PBandWidthEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("BANDWIDTH_ENVELOPE");
            {
                EnvelopeParamsSerializer(_parameters->BandWidthEnvelope).Serialize(xml);
            }
            xml->endbranch();
        }
    }
    xml->endbranch();

    xml->beginbranch("FILTER_PARAMETERS");
    {
        xml->addparbool("enabled", _parameters->PGlobalFilterEnabled);
        if ((_parameters->PGlobalFilterEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FILTER");
            {
                FilterParamsSerializer(_parameters->GlobalFilter).Serialize(xml);
            }
            xml->endbranch();

            xml->addpar("filter_velocity_sensing", _parameters->PFilterVelocityScaleFunction);
            xml->addpar("filter_velocity_sensing_amplitude", _parameters->PFilterVelocityScale);

            xml->beginbranch("FILTER_ENVELOPE");
            {
                EnvelopeParamsSerializer(_parameters->FilterEnvelope).Serialize(xml);
            }
            xml->endbranch();
        }
    }
    xml->endbranch();
}

void SUBnoteParametersSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    _parameters->Pnumstages = xml->getpar127("num_stages", _parameters->Pnumstages);
    _parameters->Phmagtype = xml->getpar127("harmonic_mag_type", _parameters->Phmagtype);
    _parameters->Pstart = xml->getpar127("start", _parameters->Pstart);

    if (xml->enterbranch("HARMONICS"))
    {
        _parameters->Phmag[0] = 0;
        for (int i = 0; i < MAX_SUB_HARMONICS; ++i)
        {
            if (xml->enterbranch("HARMONIC", i) == 0)
            {
                continue;
            }
            _parameters->Phmag[i] = xml->getpar127("mag", _parameters->Phmag[i]);
            _parameters->Phrelbw[i] = xml->getpar127("relbw", _parameters->Phrelbw[i]);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        _parameters->PStereo = xml->getparbool("stereo", _parameters->PStereo);
        _parameters->PVolume = xml->getpar127("volume", _parameters->PVolume);
        _parameters->PPanning = xml->getpar127("panning", _parameters->PPanning);
        _parameters->PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", _parameters->PAmpVelocityScaleFunction);
        if (xml->enterbranch("AMPLITUDE_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->AmpEnvelope).Deserialize(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("FREQUENCY_PARAMETERS"))
    {
        _parameters->Pfixedfreq = xml->getparbool("fixed_freq", _parameters->Pfixedfreq);
        _parameters->PfixedfreqET = xml->getpar127("fixed_freq_et", _parameters->PfixedfreqET);

        _parameters->PDetune = xml->getpar("detune", _parameters->PDetune, 0, 16383);
        _parameters->PCoarseDetune = xml->getpar("coarse_detune", _parameters->PCoarseDetune, 0, 16383);
        _parameters->POvertoneSpread.type = xml->getpar127("overtone_spread_type", _parameters->POvertoneSpread.type);
        _parameters->POvertoneSpread.par1 = xml->getpar("overtone_spread_par1", _parameters->POvertoneSpread.par1, 0, 255);
        _parameters->POvertoneSpread.par2 = xml->getpar("overtone_spread_par2", _parameters->POvertoneSpread.par2, 0, 255);
        _parameters->POvertoneSpread.par3 = xml->getpar("overtone_spread_par3", _parameters->POvertoneSpread.par3, 0, 255);
        _parameters->updateFrequencyMultipliers();
        _parameters->PDetuneType = xml->getpar127("detune_type", _parameters->PDetuneType);

        _parameters->PBandwidth = xml->getpar127("bandwidth", _parameters->PBandwidth);
        _parameters->Pbwscale = xml->getpar127("bandwidth_scale", _parameters->Pbwscale);

        _parameters->PFreqEnvelopeEnabled = xml->getparbool("freq_envelope_enabled", _parameters->PFreqEnvelopeEnabled);
        if (xml->enterbranch("FREQUENCY_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->FreqEnvelope).Deserialize(xml);
            xml->exitbranch();
        }

        _parameters->PBandWidthEnvelopeEnabled = xml->getparbool("band_width_envelope_enabled", _parameters->PBandWidthEnvelopeEnabled);
        if (xml->enterbranch("BANDWIDTH_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->BandWidthEnvelope).Deserialize(xml);
            xml->exitbranch();
        }

        xml->exitbranch();
    }

    if (xml->enterbranch("FILTER_PARAMETERS"))
    {
        _parameters->PGlobalFilterEnabled = xml->getparbool("enabled", _parameters->PGlobalFilterEnabled);
        if (xml->enterbranch("FILTER"))
        {
            FilterParamsSerializer(_parameters->GlobalFilter).Deserialize(xml);
            xml->exitbranch();
        }

        _parameters->PFilterVelocityScaleFunction = xml->getpar127("filter_velocity_sensing", _parameters->PFilterVelocityScaleFunction);
        _parameters->PFilterVelocityScale = xml->getpar127("filter_velocity_sensing_amplitude", _parameters->PFilterVelocityScale);

        if (xml->enterbranch("FILTER_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->FilterEnvelope).Deserialize(xml);
            xml->exitbranch();
        }

        xml->exitbranch();
    }
}
