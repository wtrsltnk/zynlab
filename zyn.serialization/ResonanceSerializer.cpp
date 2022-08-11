/*
  ZynAddSubFX - a software synthesizer

  Resonance.cpp - Resonance
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

#include "ResonanceSerializer.h"
#include <cmath>

ResonanceSerializer::ResonanceSerializer(
    Resonance *parameters)
    : _parameters(parameters)
{}

ResonanceSerializer::~ResonanceSerializer() = default;

void ResonanceSerializer::Serialize(
    IPresetsSerializer *xml)
{
    xml->addparbool("enabled", _parameters->Penabled);

    if ((_parameters->Penabled == 0) && (xml->minimal))
    {
        return;
    }

    xml->addpar("max_db", _parameters->PmaxdB);
    xml->addpar("center_freq", _parameters->Pcenterfreq);
    xml->addpar("octaves_freq", _parameters->Poctavesfreq);
    xml->addparbool("protect_fundamental_frequency", _parameters->Pprotectthefundamental);
    xml->addpar("resonance_points", N_RES_POINTS);
    for (int i = 0; i < N_RES_POINTS; ++i)
    {
        xml->beginbranch("RESPOINT", i);
        xml->addpar("val", _parameters->Prespoints[i]);
        xml->endbranch();
    }
}

void ResonanceSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    _parameters->Penabled = xml->getparbool("enabled", _parameters->Penabled);

    _parameters->PmaxdB = xml->getpar127("max_db", _parameters->PmaxdB);
    _parameters->Pcenterfreq = xml->getpar127("center_freq", _parameters->Pcenterfreq);
    _parameters->Poctavesfreq = xml->getpar127("octaves_freq", _parameters->Poctavesfreq);
    _parameters->Pprotectthefundamental = xml->getparbool("protect_fundamental_frequency", _parameters->Pprotectthefundamental);
    for (int i = 0; i < N_RES_POINTS; ++i)
    {
        if (xml->enterbranch("RESPOINT", i) == 0)
        {
            continue;
        }
        _parameters->Prespoints[i] = xml->getpar127("val", _parameters->Prespoints[i]);
        xml->exitbranch();
    }
}
