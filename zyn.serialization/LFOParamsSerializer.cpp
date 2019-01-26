/*
  ZynAddSubFX - a software synthesizer

  LFOParams.cpp - Parameters for LFO
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

#include "LFOParamsSerializer.h"

LFOParamsSerializer::LFOParamsSerializer(LFOParams *parameters)
    : _parameters(parameters)
{
}

LFOParamsSerializer::~LFOParamsSerializer() = default;

void LFOParamsSerializer::Serialize(IPresetsSerializer *xml)
{
    xml->addparreal("freq", _parameters->Pfreq);
    xml->addpar("intensity", _parameters->Pintensity);
    xml->addpar("start_phase", _parameters->Pstartphase);
    xml->addpar("lfo_type", _parameters->PLFOtype);
    xml->addpar("randomness_amplitude", _parameters->Prandomness);
    xml->addpar("randomness_frequency", _parameters->Pfreqrand);
    xml->addpar("delay", _parameters->Pdelay);
    xml->addpar("stretch", _parameters->Pstretch);
    xml->addparbool("continous", _parameters->Pcontinous);
}

void LFOParamsSerializer::Deserialize(IPresetsSerializer *xml)
{
    _parameters->Pfreq = xml->getparreal("freq", _parameters->Pfreq, 0.0f, 1.0f);
    _parameters->Pintensity = xml->getpar127("intensity", _parameters->Pintensity);
    _parameters->Pstartphase = xml->getpar127("start_phase", _parameters->Pstartphase);
    _parameters->PLFOtype = xml->getpar127("lfo_type", _parameters->PLFOtype);
    _parameters->Prandomness = xml->getpar127("randomness_amplitude", _parameters->Prandomness);
    _parameters->Pfreqrand = xml->getpar127("randomness_frequency", _parameters->Pfreqrand);
    _parameters->Pdelay = xml->getpar127("delay", _parameters->Pdelay);
    _parameters->Pstretch = xml->getpar127("stretch", _parameters->Pstretch);
    _parameters->Pcontinous = xml->getparbool("continous", _parameters->Pcontinous) == 1 ? 1 : 0;
}
