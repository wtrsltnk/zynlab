/*
  ZynAddSubFX - a software synthesizer

  EnvelopeParams.cpp - Parameters for Envelope
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

#include "EnvelopeParamsSerializer.h"

EnvelopeParamsSerializer::EnvelopeParamsSerializer(
    EnvelopeParams *parameters)
    : _parameters(parameters)
{}

EnvelopeParamsSerializer::~EnvelopeParamsSerializer() = default;

void EnvelopeParamsSerializer::Serialize(
    IPresetsSerializer *xml)
{
    xml->addparbool("free_mode", _parameters->Pfreemode);
    xml->addpar("env_points", _parameters->Penvpoints);
    xml->addpar("env_sustain", _parameters->Penvsustain);
    xml->addpar("env_stretch", _parameters->Penvstretch);
    xml->addparbool("forced_release", _parameters->Pforcedrelease);
    xml->addparbool("linear_envelope", _parameters->Plinearenvelope);
    xml->addpar("A_dt", _parameters->PA_dt);
    xml->addpar("D_dt", _parameters->PD_dt);
    xml->addpar("R_dt", _parameters->PR_dt);
    xml->addpar("A_val", _parameters->PA_val);
    xml->addpar("D_val", _parameters->PD_val);
    xml->addpar("S_val", _parameters->PS_val);
    xml->addpar("R_val", _parameters->PR_val);

    if ((_parameters->Pfreemode != 0) || (!xml->minimal))
    {
        for (int i = 0; i < _parameters->Penvpoints; ++i)
        {
            xml->beginbranch("POINT", i);
            if (i != 0)
            {
                xml->addpar("dt", _parameters->Penvdt[i]);
            }
            xml->addpar("val", _parameters->Penvval[i]);
            xml->endbranch();
        }
    }
}

void EnvelopeParamsSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    _parameters->Pfreemode = xml->getparbool("free_mode", _parameters->Pfreemode);
    _parameters->Penvpoints = xml->getpar127("env_points", _parameters->Penvpoints);
    _parameters->Penvsustain = xml->getpar127("env_sustain", _parameters->Penvsustain);
    _parameters->Penvstretch = xml->getpar127("env_stretch", _parameters->Penvstretch);
    _parameters->Pforcedrelease = xml->getparbool("forced_release", _parameters->Pforcedrelease);
    _parameters->Plinearenvelope = xml->getparbool("linear_envelope", _parameters->Plinearenvelope);

    _parameters->PA_dt = xml->getpar127("A_dt", _parameters->PA_dt);
    _parameters->PD_dt = xml->getpar127("D_dt", _parameters->PD_dt);
    _parameters->PR_dt = xml->getpar127("R_dt", _parameters->PR_dt);
    _parameters->PA_val = xml->getpar127("A_val", _parameters->PA_val);
    _parameters->PD_val = xml->getpar127("D_val", _parameters->PD_val);
    _parameters->PS_val = xml->getpar127("S_val", _parameters->PS_val);
    _parameters->PR_val = xml->getpar127("R_val", _parameters->PR_val);

    for (int i = 0; i < _parameters->Penvpoints; ++i)
    {
        if (xml->enterbranch("POINT", i) == 0)
        {
            continue;
        }
        if (i != 0)
        {
            _parameters->Penvdt[i] = xml->getpar127("dt", _parameters->Penvdt[i]);
        }
        _parameters->Penvval[i] = xml->getpar127("val", _parameters->Penvval[i]);
        xml->exitbranch();
    }

    if (!_parameters->Pfreemode)
    {
        EnvelopeParams::ConvertToFree(_parameters);
    }
}
