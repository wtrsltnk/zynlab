/*
  ZynAddSubFX - a software synthesizer

  EffectManager.cpp - Effect manager, an interface betwen the program and effects
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

#include "EffectMgrSerializer.h"

#include "FilterParamsSerializer.h"
#include <zyn.fx/Effect.h>

EffectManagerSerializer::EffectManagerSerializer(
    EffectManager *parameters)
    : _parameters(parameters)
{}

EffectManagerSerializer::~EffectManagerSerializer() = default;

void EffectManagerSerializer::Serialize(
    IPresetsSerializer *xml)
{
    xml->addpar("type", _parameters->geteffect());

    if (_parameters->_effect == nullptr || !_parameters->geteffect())
    {
        return;
    }
    xml->addpar("preset", _parameters->_effect->Ppreset);

    xml->beginbranch("EFFECT_PARAMETERS");
    for (int n = 0; n < 128; ++n)
    {
        int par = _parameters->geteffectpar(n);
        if (par == 0)
        {
            continue;
        }
        xml->beginbranch("par_no", n);
        xml->addpar("par", par);
        xml->endbranch();
    }
    if (_parameters->_filterpars)
    {
        xml->beginbranch("FILTER");
        FilterParamsSerializer(_parameters->_filterpars).Serialize(xml);
        xml->endbranch();
    }
    xml->endbranch();
}

void EffectManagerSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    _parameters->changeeffect(xml->getpar127("type", _parameters->geteffect()));

    if (_parameters->_effect == nullptr || !_parameters->geteffect())
    {
        return;
    }

    _parameters->_effect->Ppreset = xml->getpar127("preset", _parameters->_effect->Ppreset);

    if (xml->enterbranch("EFFECT_PARAMETERS"))
    {
        for (int n = 0; n < 128; ++n)
        {
            _parameters->seteffectpar_nolock(n, 0); //erase effect parameter
            if (xml->enterbranch("par_no", n) == 0)
            {
                continue;
            }
            int par = _parameters->geteffectpar(n);
            _parameters->seteffectpar_nolock(n, xml->getpar127("par", par));
            xml->exitbranch();
        }
        if (_parameters->_filterpars)
        {
            if (xml->enterbranch("FILTER"))
            {
                FilterParamsSerializer(_parameters->_filterpars).Deserialize(xml);
                xml->exitbranch();
            }
        }
        xml->exitbranch();
    }
    _parameters->cleanup();
}
