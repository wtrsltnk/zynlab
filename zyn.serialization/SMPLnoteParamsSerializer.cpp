/*
  ZynAddSubFX - a software synthesizer

  LFOParams.h - Parameters for LFO
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

#include "SMPLnoteParamsSerializer.h"

char const *SMPLnoteParametersSerializer::BRANCH_NAME = "SMPL_SYNTH_PARAMETERS";

SMPLnoteParametersSerializer::SMPLnoteParametersSerializer(
    SampleNoteParameters *parameters)
    : _parameters(parameters)
{}

SMPLnoteParametersSerializer::~SMPLnoteParametersSerializer() = default;

void SMPLnoteParametersSerializer::Serialize(
    IPresetsSerializer *xml)
{
    xml->beginbranch("AMPLITUDE_PARAMETERS");
    {
        xml->addpar("volume", _parameters->PVolume);
        xml->addpar("panning", _parameters->PPanning);
        xml->addpar("velocity_sensing", _parameters->PAmpVelocityScaleFunction);
    }
    xml->endbranch();

    xml->beginbranch("SAMPLES");
    {
        for (int i = SAMPLE_NOTE_MIN; i <= SAMPLE_NOTE_MAX; i++)
        {
            if (_parameters->PwavData.find(i) == _parameters->PwavData.end())
            {
                continue;
            }
            xml->beginbranch("SAMPLE", static_cast<int>(i));
            xml->addparstr("name", _parameters->PwavData[i]->name);
            xml->addparunsigned("channels", _parameters->PwavData[i]->channels);
            xml->addparunsigned("sample_per_channel", _parameters->PwavData[i]->samplesPerChannel);
            xml->addparstr("sample_data", WavData::toBase64(_parameters->PwavData[i]));
            xml->endbranch();
        }
    }
    xml->endbranch();
}

void SMPLnoteParametersSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    if (xml->enterbranch("AMPLITUDE_PARAMETERS"))
    {
        _parameters->PVolume = xml->getpar127("volume", _parameters->PVolume);
        _parameters->PPanning = xml->getpar127("panning", _parameters->PPanning);
        _parameters->PAmpVelocityScaleFunction = xml->getpar127("velocity_sensing", _parameters->PAmpVelocityScaleFunction);
        xml->exitbranch();
    }

    if (xml->enterbranch("SAMPLES"))
    {
        for (int i = SAMPLE_NOTE_MIN; i <= SAMPLE_NOTE_MAX; i++)
        {
            if (xml->enterbranch("SAMPLE", i) == 0)
            {
                continue;
            }
            _parameters->PwavData.insert(std::make_pair(i, new WavData()));

            _parameters->PwavData[i]->name = xml->getparstr("name", "");
            _parameters->PwavData[i]->channels = xml->getparunsigned("channels", 2, 1, 8);
            _parameters->PwavData[i]->samplesPerChannel = xml->getparunsigned("sample_per_channel", 0, 0, UINT_MAX);
            auto base64Data = xml->getparstr("sample_data", "");
            WavData::fromBase64(base64Data, _parameters->PwavData[i]);
            xml->exitbranch();
        }
        xml->exitbranch();
    }
}
