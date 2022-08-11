/*
  ZynAddSubFX - a software synthesizer

  Microtonal.cpp - Tuning settings and microtonal capabilities
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

#include "MicrotonalSerializer.h"
#include <cmath>

MicrotonalSerializer::MicrotonalSerializer(
    Microtonal *parameters)
    : _parameters(parameters)
{}

MicrotonalSerializer::~MicrotonalSerializer() = default;

void MicrotonalSerializer::Serialize(
    IPresetsSerializer *xml)
{
    xml->addparstr("name", (char *)_parameters->Pname);
    xml->addparstr("comment", (char *)_parameters->Pcomment);

    xml->addparbool("invert_up_down", _parameters->Pinvertupdown);
    xml->addpar("invert_up_down_center", _parameters->Pinvertupdowncenter);

    xml->addparbool("enabled", _parameters->Penabled);
    xml->addpar("global_fine_detune", _parameters->Pglobalfinedetune);

    xml->addpar("a_note", _parameters->PAnote);
    xml->addparreal("a_freq", _parameters->PAfreq);

    if ((_parameters->Penabled == 0) && (xml->minimal))
    {
        return;
    }

    xml->beginbranch("SCALE");
    xml->addpar("scale_shift", _parameters->Pscaleshift);
    xml->addpar("first_key", _parameters->Pfirstkey);
    xml->addpar("last_key", _parameters->Plastkey);
    xml->addpar("middle_note", _parameters->Pmiddlenote);

    xml->beginbranch("OCTAVE");
    xml->addpar("octave_size", _parameters->octavesize);
    for (int i = 0; i < _parameters->octavesize; ++i)
    {
        xml->beginbranch("DEGREE", i);
        if (_parameters->octave[i].type == 1)
        {
            xml->addparreal("cents", _parameters->octave[i].tuning);
        }
        if (_parameters->octave[i].type == 2)
        {
            xml->addpar("numerator", _parameters->octave[i].x1);
            xml->addpar("denominator", _parameters->octave[i].x2);
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("KEYBOARD_MAPPING");
    xml->addpar("map_size", _parameters->Pmapsize);
    xml->addpar("mapping_enabled", _parameters->Pmappingenabled);
    for (int i = 0; i < _parameters->Pmapsize; ++i)
    {
        xml->beginbranch("KEYMAP", i);
        xml->addpar("degree", _parameters->Pmapping[i]);
        xml->endbranch();
    }

    xml->endbranch();
    xml->endbranch();
}

void MicrotonalSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    xml->getparstr("name", (char *)_parameters->Pname, MICROTONAL_MAX_NAME_LEN);
    xml->getparstr("comment", (char *)_parameters->Pcomment, MICROTONAL_MAX_NAME_LEN);

    _parameters->Pinvertupdown = xml->getparbool("invert_up_down", _parameters->Pinvertupdown);
    _parameters->Pinvertupdowncenter = xml->getpar127("invert_up_down_center", _parameters->Pinvertupdowncenter);

    _parameters->Penabled = xml->getparbool("enabled", _parameters->Penabled);
    _parameters->Pglobalfinedetune = xml->getpar127("global_fine_detune", _parameters->Pglobalfinedetune);

    _parameters->PAnote = xml->getpar127("a_note", _parameters->PAnote);
    _parameters->PAfreq = xml->getparreal("a_freq", _parameters->PAfreq, 1.0f, 10000.0f);

    if (xml->enterbranch("SCALE"))
    {
        _parameters->Pscaleshift = xml->getpar127("scale_shift", _parameters->Pscaleshift);
        _parameters->Pfirstkey = xml->getpar127("first_key", _parameters->Pfirstkey);
        _parameters->Plastkey = xml->getpar127("last_key", _parameters->Plastkey);
        _parameters->Pmiddlenote = xml->getpar127("middle_note", _parameters->Pmiddlenote);

        if (xml->enterbranch("OCTAVE"))
        {
            _parameters->octavesize = xml->getpar127("octave_size", _parameters->octavesize);
            for (int i = 0; i < _parameters->octavesize; ++i)
            {
                if (xml->enterbranch("DEGREE", i) == 0)
                {
                    continue;
                }
                _parameters->octave[i].x2 = 0;
                _parameters->octave[i].tuning = xml->getparreal("cents", _parameters->octave[i].tuning);
                _parameters->octave[i].x1 = xml->getpar127("numerator", _parameters->octave[i].x1);
                _parameters->octave[i].x2 = xml->getpar127("denominator", _parameters->octave[i].x2);

                if (_parameters->octave[i].x2 != 0)
                {
                    _parameters->octave[i].type = 2;
                }
                else
                {
                    _parameters->octave[i].type = 1;
                    //populate fields for display
                    float x = logf(_parameters->octave[i].tuning) / LOG_2 * 1200.0f;
                    _parameters->octave[i].x1 = static_cast<unsigned int>(std::floor(x));
                    _parameters->octave[i].x2 = static_cast<unsigned int>(std::floor(std::fmod(x, 1.0) * 1e6));
                }

                xml->exitbranch();
            }
            xml->exitbranch();
        }

        if (xml->enterbranch("KEYBOARD_MAPPING"))
        {
            _parameters->Pmapsize = xml->getpar127("map_size", _parameters->Pmapsize);
            _parameters->Pmappingenabled = xml->getpar127("mapping_enabled", _parameters->Pmappingenabled);
            for (int i = 0; i < _parameters->Pmapsize; ++i)
            {
                if (xml->enterbranch("KEYMAP", i) == 0)
                {
                    continue;
                }
                _parameters->Pmapping[i] = xml->getpar127("degree", _parameters->Pmapping[i]);
                xml->exitbranch();
            }
            xml->exitbranch();
        }
        xml->exitbranch();
    }
}
