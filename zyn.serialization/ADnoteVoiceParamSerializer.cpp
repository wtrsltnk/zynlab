/*
  ZynAddSubFX - a software synthesizer

  ADnoteVoiceParamSerializer.cpp - Serializer for ADnote Voice Parameters
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

#include "ADnoteVoiceParamSerializer.h"

#include "EnvelopeParamsSerializer.h"
#include "FilterParamsSerializer.h"
#include "LFOParamsSerializer.h"
#include "OscilGenSerializer.h"

ADnoteVoiceParamSerializer::ADnoteVoiceParamSerializer(ADnoteVoiceParam *parameters)
    : _parameters(parameters)
{}

ADnoteVoiceParamSerializer::~ADnoteVoiceParamSerializer() = default;

void ADnoteVoiceParamSerializer::Serialize(IPresetsSerializer *xml, bool fmoscilused)
{
    xml->addpar("type", _parameters->Type);

    xml->addpar("unison_size", _parameters->Unison_size);
    xml->addpar("unison_frequency_spread", _parameters->Unison_frequency_spread);
    xml->addpar("unison_stereo_spread", _parameters->Unison_stereo_spread);
    xml->addpar("unison_vibratto", _parameters->Unison_vibratto);
    xml->addpar("unison_vibratto_speed", _parameters->Unison_vibratto_speed);
    xml->addpar("unison_invert_phase", _parameters->Unison_invert_phase);
    xml->addpar("unison_phase_randomness", _parameters->Unison_phase_randomness);

    xml->addpar("delay", _parameters->PDelay);
    xml->addparbool("resonance", _parameters->Presonance);

    xml->addpar("ext_oscil", _parameters->Pextoscil);
    xml->addpar("ext_fm_oscil", _parameters->PextFMoscil);

    xml->addpar("oscil_phase", _parameters->Poscilphase);
    xml->addpar("oscil_fm_phase", _parameters->PFMoscilphase);

    xml->addparbool("filter_enabled", _parameters->PFilterEnabled);
    xml->addparbool("filter_bypass", _parameters->Pfilterbypass);

    xml->addpar("fm_enabled", _parameters->PFMEnabled);

    xml->beginbranch("OSCIL");
    OscilGenSerializer(_parameters->OscilSmp).Serialize(xml);
    xml->endbranch();

    xml->beginbranch("AMPLITUDE_PARAMETERS");
    {
        xml->addpar("panning", _parameters->PPanning);
        xml->addpar("volume", _parameters->PVolume);
        xml->addparbool("volume_minus", _parameters->PVolumeminus);
        xml->addpar("velocity_sensing", _parameters->PAmpVelocityScaleFunction);

        xml->addparbool("amp_envelope_enabled", _parameters->PAmpEnvelopeEnabled);
        if ((_parameters->PAmpEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("AMPLITUDE_ENVELOPE");
            EnvelopeParamsSerializer(_parameters->AmpEnvelope).Serialize(xml);
            xml->endbranch();
        }
        xml->addparbool("amp_lfo_enabled", _parameters->PAmpLfoEnabled);
        if ((_parameters->PAmpLfoEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("AMPLITUDE_LFO");
            LFOParamsSerializer(_parameters->AmpLfo).Serialize(xml);
            xml->endbranch();
        }
    }
    xml->endbranch();

    xml->beginbranch("FREQUENCY_PARAMETERS");
    {
        xml->addparbool("fixed_freq", _parameters->Pfixedfreq);
        xml->addpar("fixed_freq_et", _parameters->PfixedfreqET);
        xml->addpar("detune", _parameters->PDetune);
        xml->addpar("coarse_detune", _parameters->PCoarseDetune);
        xml->addpar("detune_type", _parameters->PDetuneType);

        xml->addparbool("freq_envelope_enabled", _parameters->PFreqEnvelopeEnabled);
        if ((_parameters->PFreqEnvelopeEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FREQUENCY_ENVELOPE");
            EnvelopeParamsSerializer(_parameters->FreqEnvelope).Serialize(xml);
            xml->endbranch();
        }
        xml->addparbool("freq_lfo_enabled", _parameters->PFreqLfoEnabled);
        if ((_parameters->PFreqLfoEnabled != 0) || (!xml->minimal))
        {
            xml->beginbranch("FREQUENCY_LFO");
            LFOParamsSerializer(_parameters->FreqLfo).Serialize(xml);
            xml->endbranch();
        }
    }
    xml->endbranch();

    if ((_parameters->PFilterEnabled != 0) || (!xml->minimal))
    {
        xml->beginbranch("FILTER_PARAMETERS");
        {
            xml->beginbranch("FILTER");
            FilterParamsSerializer(_parameters->VoiceFilter).Serialize(xml);
            xml->endbranch();

            xml->addparbool("filter_envelope_enabled", _parameters->PFilterEnvelopeEnabled);
            if ((_parameters->PFilterEnvelopeEnabled != 0) || (!xml->minimal))
            {
                xml->beginbranch("FILTER_ENVELOPE");
                EnvelopeParamsSerializer(_parameters->FilterEnvelope).Serialize(xml);
                xml->endbranch();
            }

            xml->addparbool("filter_lfo_enabled", _parameters->PFilterLfoEnabled);
            if ((_parameters->PFilterLfoEnabled != 0) || (!xml->minimal))
            {
                xml->beginbranch("FILTER_LFO");
                LFOParamsSerializer(_parameters->FilterLfo).Serialize(xml);
                xml->endbranch();
            }
        }
        xml->endbranch();
    }

    if ((_parameters->PFMEnabled != 0) || fmoscilused || (!xml->minimal))
    {
        xml->beginbranch("FM_PARAMETERS");
        {
            xml->addpar("input_voice", _parameters->PFMVoice);

            xml->addpar("volume", _parameters->PFMVolume);
            xml->addpar("volume_damp", _parameters->PFMVolumeDamp);
            xml->addpar("velocity_sensing", _parameters->PFMVelocityScaleFunction);

            xml->addparbool("amp_envelope_enabled", _parameters->PFMAmpEnvelopeEnabled);
            if ((_parameters->PFMAmpEnvelopeEnabled != 0) || (!xml->minimal))
            {
                xml->beginbranch("AMPLITUDE_ENVELOPE");
                EnvelopeParamsSerializer(_parameters->FMAmpEnvelope).Serialize(xml);
                xml->endbranch();
            }
            xml->beginbranch("MODULATOR");
            {
                xml->addpar("detune", _parameters->PFMDetune);
                xml->addpar("coarse_detune", _parameters->PFMCoarseDetune);
                xml->addpar("detune_type", _parameters->PFMDetuneType);

                xml->addparbool("freq_envelope_enabled", _parameters->PFMFreqEnvelopeEnabled);
                if ((_parameters->PFMFreqEnvelopeEnabled != 0) || (!xml->minimal))
                {
                    xml->beginbranch("FREQUENCY_ENVELOPE");
                    EnvelopeParamsSerializer(_parameters->FMFreqEnvelope).Serialize(xml);
                    xml->endbranch();
                }

                xml->beginbranch("OSCIL");
                OscilGenSerializer(_parameters->FMSmp).Serialize(xml);
                xml->endbranch();
            }
            xml->endbranch();
        }
        xml->endbranch();
    }
}

void ADnoteVoiceParamSerializer::Deserialize(IPresetsSerializer *xml, unsigned nvoice)
{
    _parameters->Enabled = xml->getparbool("enabled", 0);
    _parameters->Unison_size = xml->getpar127("unison_size", _parameters->Unison_size);
    _parameters->Unison_frequency_spread = xml->getpar127("unison_frequency_spread", _parameters->Unison_frequency_spread);
    _parameters->Unison_stereo_spread = xml->getpar127("unison_stereo_spread", _parameters->Unison_stereo_spread);
    _parameters->Unison_vibratto = xml->getpar127("unison_vibratto", _parameters->Unison_vibratto);
    _parameters->Unison_vibratto_speed = xml->getpar127("unison_vibratto_speed", _parameters->Unison_vibratto_speed);
    _parameters->Unison_invert_phase = xml->getpar127("unison_invert_phase", _parameters->Unison_invert_phase);
    _parameters->Unison_phase_randomness = xml->getpar127("unison_phase_randomness", _parameters->Unison_phase_randomness);

    _parameters->Type = xml->getpar127("type", _parameters->Type);
    _parameters->PDelay = xml->getpar127("delay", _parameters->PDelay);
    _parameters->Presonance = xml->getparbool("resonance", _parameters->Presonance);

    _parameters->Pextoscil = xml->getpar("ext_oscil", -1, -1, nvoice - 1);
    _parameters->PextFMoscil = xml->getpar("ext_fm_oscil", -1, -1, nvoice - 1);

    _parameters->Poscilphase = xml->getpar127("oscil_phase", _parameters->Poscilphase);
    _parameters->PFMoscilphase = xml->getpar127("oscil_fm_phase", _parameters->PFMoscilphase);
    _parameters->PFilterEnabled = xml->getparbool("filter_enabled", _parameters->PFilterEnabled);
    _parameters->Pfilterbypass = xml->getparbool("filter_bypass", _parameters->Pfilterbypass);
    _parameters->PFMEnabled = xml->getpar127("fm_enabled", _parameters->PFMEnabled);

    if (xml->enterbranch("OSCIL"))
    {
        OscilGenSerializer(_parameters->OscilSmp).Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        _parameters->PPanning = xml->getpar127("panning", _parameters->PPanning);
        _parameters->PVolume = xml->getpar127("volume", _parameters->PVolume);
        _parameters->PVolumeminus = xml->getparbool("volume_minus", _parameters->PVolumeminus);
        _parameters->PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", _parameters->PAmpVelocityScaleFunction);

        _parameters->PAmpEnvelopeEnabled = xml->getparbool("amp_envelope_enabled", _parameters->PAmpEnvelopeEnabled);
        if (xml->enterbranch("AMPLITUDE_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->AmpEnvelope).Deserialize(xml);
            xml->exitbranch();
        }

        _parameters->PAmpLfoEnabled = xml->getparbool("amp_lfo_enabled", _parameters->PAmpLfoEnabled);
        if (xml->enterbranch("AMPLITUDE_LFO"))
        {
            LFOParamsSerializer(_parameters->AmpLfo).Deserialize(xml);
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
        _parameters->PDetuneType = xml->getpar127("detune_type", _parameters->PDetuneType);
        _parameters->PFreqEnvelopeEnabled = xml->getparbool("freq_envelope_enabled", _parameters->PFreqEnvelopeEnabled);

        if (xml->enterbranch("FREQUENCY_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->FreqEnvelope).Deserialize(xml);
            xml->exitbranch();
        }

        _parameters->PFreqLfoEnabled = xml->getparbool("freq_lfo_enabled", _parameters->PFreqLfoEnabled);

        if (xml->enterbranch("FREQUENCY_LFO"))
        {
            LFOParamsSerializer(_parameters->FreqLfo).Deserialize(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("FILTER_PARAMETERS"))
    {
        if (xml->enterbranch("FILTER"))
        {
            FilterParamsSerializer(_parameters->VoiceFilter).Deserialize(xml);
            xml->exitbranch();
        }

        _parameters->PFilterEnvelopeEnabled = xml->getparbool("filter_envelope_enabled", _parameters->PFilterEnvelopeEnabled);
        if (xml->enterbranch("FILTER_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->FilterEnvelope).Deserialize(xml);
            xml->exitbranch();
        }

        _parameters->PFilterLfoEnabled = xml->getparbool("filter_lfo_enabled", _parameters->PFilterLfoEnabled);
        if (xml->enterbranch("FILTER_LFO"))
        {
            LFOParamsSerializer(_parameters->FilterLfo).Deserialize(xml);
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("FM_PARAMETERS"))
    {
        _parameters->PFMVoice = xml->getpar("input_voice", _parameters->PFMVoice, -1, nvoice - 1);
        _parameters->PFMVolume = xml->getpar127("volume", _parameters->PFMVolume);
        _parameters->PFMVolumeDamp = xml->getpar127("volume_damp", _parameters->PFMVolumeDamp);
        _parameters->PFMVelocityScaleFunction = xml->getpar127("velocity_sensing", _parameters->PFMVelocityScaleFunction);

        _parameters->PFMAmpEnvelopeEnabled = xml->getparbool("amp_envelope_enabled", _parameters->PFMAmpEnvelopeEnabled);
        if (xml->enterbranch("AMPLITUDE_ENVELOPE"))
        {
            EnvelopeParamsSerializer(_parameters->FMAmpEnvelope).Deserialize(xml);
            xml->exitbranch();
        }

        if (xml->enterbranch("MODULATOR"))
        {
            _parameters->PFMDetune = xml->getpar("detune", _parameters->PFMDetune, 0, 16383);
            _parameters->PFMCoarseDetune = xml->getpar("coarse_detune", _parameters->PFMCoarseDetune, 0, 16383);
            _parameters->PFMDetuneType = xml->getpar127("detune_type", _parameters->PFMDetuneType);

            _parameters->PFMFreqEnvelopeEnabled = xml->getparbool("freq_envelope_enabled", _parameters->PFMFreqEnvelopeEnabled);
            if (xml->enterbranch("FREQUENCY_ENVELOPE"))
            {
                EnvelopeParamsSerializer(_parameters->FMFreqEnvelope).Deserialize(xml);
                xml->exitbranch();
            }

            if (xml->enterbranch("OSCIL"))
            {
                OscilGenSerializer(_parameters->FMSmp).Deserialize(xml);
                xml->exitbranch();
            }

            xml->exitbranch();
        }
        xml->exitbranch();
    }
}
