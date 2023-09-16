/*
  ZynAddSubFX - a software synthesizer

  Part.cpp - Part implementation
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

#include "TrackSerializer.h"

#include "ADnoteParamsSerializer.h"
#include "ControllerSerializer.h"
#include "EffectMgrSerializer.h"
#include "PADnoteParamsSerializer.h"
#include "SMPLnoteParamsSerializer.h"
#include "SUBnoteParamsSerializer.h"
#include <zyn.fx/EffectMgr.h>

char const *TrackSerializer::BRANCH_NAME = "INSTRUMENT";

TrackSerializer::TrackSerializer(
    Track *parameters)
    : _parameters(parameters)
{}

TrackSerializer::~TrackSerializer() = default;

#define INSTRUMENT_5_KIT_1_VALUE

void TrackSerializer::SerializeInstrument(
    IPresetsSerializer *xml)
{
    xml->beginbranch("INFO");
    xml->addparstr("name", reinterpret_cast<char *>(_parameters->Pname));
    xml->addparstr("author", reinterpret_cast<char *>(_parameters->info.Pauthor));
    xml->addparstr("comments", reinterpret_cast<char *>(_parameters->info.Pcomments));
    xml->addpar("type", _parameters->info.Ptype);
    xml->endbranch();

    xml->beginbranch("INSTRUMENT_KIT");
    xml->addpar("kit_mode", _parameters->Pkitmode);
    xml->addparbool("drum_mode", _parameters->Pdrummode);

    for (int i = 0; i < NUM_TRACK_INSTRUMENTS; ++i)
    {
        xml->beginbranch("INSTRUMENT_KIT_ITEM", i);
        xml->addparbool("enabled", _parameters->Instruments[i].Penabled);
        if (_parameters->Instruments[i].Penabled != 0)
        {
            xml->addparstr("name", reinterpret_cast<char *>(_parameters->Instruments[i].Pname));

            xml->addparbool("muted", _parameters->Instruments[i].Pmuted);
            xml->addpar("min_key", _parameters->Instruments[i].Pminkey);
            xml->addpar("max_key", _parameters->Instruments[i].Pmaxkey);

            xml->addpar("send_to_instrument_effect", _parameters->Instruments[i].Psendtoparteffect);

            xml->addparbool("add_enabled", _parameters->Instruments[i].Padenabled);
            if ((_parameters->Instruments[i].Padenabled != 0) && (_parameters->Instruments[i].adpars != nullptr))
            {
                xml->beginbranch(ADnoteParametersSerializer::BRANCH_NAME);
                ADnoteParametersSerializer(_parameters->Instruments[i].adpars).Serialize(xml);
                xml->endbranch();
            }

            xml->addparbool("sub_enabled", _parameters->Instruments[i].Psubenabled);
            if ((_parameters->Instruments[i].Psubenabled != 0) && (_parameters->Instruments[i].subpars != nullptr))
            {
                xml->beginbranch(SUBnoteParametersSerializer::BRANCH_NAME);
                SUBnoteParametersSerializer(_parameters->Instruments[i].subpars).Serialize(xml);
                xml->endbranch();
            }

            xml->addparbool("smpl_enabled", _parameters->Instruments[i].Psmplenabled);
            if ((_parameters->Instruments[i].Psmplenabled != 0) && (_parameters->Instruments[i].smplpars != nullptr))
            {
                xml->beginbranch(SMPLnoteParametersSerializer::BRANCH_NAME);
                SMPLnoteParametersSerializer(_parameters->Instruments[i].smplpars).Serialize(xml);
                xml->endbranch();
            }

            xml->addparbool("pad_enabled", _parameters->Instruments[i].Ppadenabled);
            if ((_parameters->Instruments[i].Ppadenabled != 0) && (_parameters->Instruments[i].padpars != nullptr))
            {
                xml->beginbranch(PADnoteParametersSerializer::BRANCH_NAME);
                PADnoteParametersSerializer(_parameters->Instruments[i].padpars).Serialize(xml);
                xml->endbranch();
            }
        }
        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("INSTRUMENT_EFFECTS");
    for (int nefx = 0; nefx < NUM_TRACK_EFX; ++nefx)
    {
        xml->beginbranch("INSTRUMENT_EFFECT", nefx);
        xml->beginbranch("EFFECT");
        EffectManagerSerializer(&_parameters->partefx[nefx]).Serialize(xml);
        xml->endbranch();

        xml->addpar("route", _parameters->Pefxroute[nefx]);
        _parameters->partefx[nefx].setdryonly(_parameters->Pefxroute[nefx] == 2);
        xml->addparbool("bypass", _parameters->Pefxbypass[nefx]);
        xml->endbranch();
    }
    xml->endbranch();
}

void TrackSerializer::Serialize(
    IPresetsSerializer *xml)
{
    // parameters
    xml->addparbool("enabled", _parameters->Penabled);
    if ((_parameters->Penabled == 0) && (xml->minimal))
    {
        return;
    }

    xml->addpar("volume", _parameters->Pvolume);
    xml->addpar("panning", _parameters->Ppanning);

    xml->addpar("min_key", _parameters->Pminkey);
    xml->addpar("max_key", _parameters->Pmaxkey);
    xml->addpar("key_shift", _parameters->Pkeyshift);
    xml->addpar("rcv_chn", _parameters->Prcvchn);

    xml->addpar("velocity_sensing", _parameters->Pvelsns);
    xml->addpar("velocity_offset", _parameters->Pveloffs);

    xml->addparbool("note_on", _parameters->Pnoteon);
    xml->addparbool("poly_mode", _parameters->Ppolymode);
    xml->addpar("legato_mode", _parameters->Plegatomode);
    xml->addpar("key_limit", _parameters->Pkeylimit);

    xml->beginbranch("INSTRUMENT");
    SerializeInstrument(xml);
    xml->endbranch();

    xml->beginbranch("CONTROLLER");
    ControllerSerializer(&_parameters->ctl).Serialize(xml);
    xml->endbranch();
}

void TrackSerializer::DeserializeInstrument(
    IPresetsSerializer *xml)
{
    if (xml->enterbranch("INFO"))
    {
        xml->getparstr("name", reinterpret_cast<char *>(_parameters->Pname), TRACK_MAX_NAME_LEN);
        xml->getparstr("author", reinterpret_cast<char *>(_parameters->info.Pauthor), MAX_INFO_TEXT_SIZE);
        xml->getparstr("comments", reinterpret_cast<char *>(_parameters->info.Pcomments), MAX_INFO_TEXT_SIZE);
        _parameters->info.Ptype = static_cast<unsigned char>(xml->getpar("type", _parameters->info.Ptype, 0, 16));

        xml->exitbranch();
    }

    if (xml->enterbranch("INSTRUMENT_KIT"))
    {
        _parameters->Pkitmode = static_cast<unsigned char>(xml->getpar127("kit_mode", _parameters->Pkitmode));
        _parameters->Pdrummode = static_cast<unsigned char>(xml->getparbool("drum_mode", _parameters->Pdrummode));

        _parameters->setkititemstatus(0, 0);
        for (int i = 0; i < NUM_TRACK_INSTRUMENTS; ++i)
        {
            if (xml->enterbranch("INSTRUMENT_KIT_ITEM", i) == 0)
            {
                continue;
            }
            _parameters->setkititemstatus(i, xml->getparbool("enabled", _parameters->Instruments[i].Penabled));
            if (_parameters->Instruments[i].Penabled == 0)
            {
                xml->exitbranch();
                continue;
            }

            xml->getparstr("name", reinterpret_cast<char *>(_parameters->Instruments[i].Pname), TRACK_MAX_NAME_LEN);

            _parameters->Instruments[i].Pmuted = static_cast<unsigned char>(xml->getparbool("muted", _parameters->Instruments[i].Pmuted));
            _parameters->Instruments[i].Pminkey = static_cast<unsigned char>(xml->getpar127("min_key", _parameters->Instruments[i].Pminkey));
            _parameters->Instruments[i].Pmaxkey = static_cast<unsigned char>(xml->getpar127("max_key", _parameters->Instruments[i].Pmaxkey));

            _parameters->Instruments[i].Psendtoparteffect = static_cast<unsigned char>(xml->getpar127("send_to_instrument_effect", _parameters->Instruments[i].Psendtoparteffect));

            _parameters->Instruments[i].Padenabled = static_cast<unsigned char>(xml->getparbool("add_enabled", _parameters->Instruments[i].Padenabled));
            if (xml->enterbranch(ADnoteParametersSerializer::BRANCH_NAME))
            {
                ADnoteParametersSerializer(_parameters->Instruments[i].adpars).Deserialize(xml);
                xml->exitbranch();
            }

            _parameters->Instruments[i].Psubenabled = static_cast<unsigned char>(xml->getparbool("sub_enabled", _parameters->Instruments[i].Psubenabled));
            if (xml->enterbranch(SUBnoteParametersSerializer::BRANCH_NAME))
            {
                SUBnoteParametersSerializer(_parameters->Instruments[i].subpars).Deserialize(xml);
                xml->exitbranch();
            }

            _parameters->Instruments[i].Psmplenabled = static_cast<unsigned char>(xml->getparbool("smpl_enabled", _parameters->Instruments[i].Psmplenabled));
            if (xml->enterbranch(SMPLnoteParametersSerializer::BRANCH_NAME))
            {
                SMPLnoteParametersSerializer(_parameters->Instruments[i].smplpars).Deserialize(xml);
                xml->exitbranch();
            }

            _parameters->Instruments[i].Ppadenabled = static_cast<unsigned char>(xml->getparbool("pad_enabled", _parameters->Instruments[i].Ppadenabled));
            if (xml->enterbranch(PADnoteParametersSerializer::BRANCH_NAME))
            {
                PADnoteParametersSerializer(_parameters->Instruments[i].padpars).Deserialize(xml);
                xml->exitbranch();
            }

            xml->exitbranch();
        }

        xml->exitbranch();
    }

    if (xml->enterbranch("INSTRUMENT_EFFECTS"))
    {
        for (int nefx = 0; nefx < NUM_TRACK_EFX; ++nefx)
        {
            if (xml->enterbranch("INSTRUMENT_EFFECT", nefx) == 0)
                continue;
            if (xml->enterbranch("EFFECT"))
            {
                EffectManagerSerializer(&_parameters->partefx[nefx]).Deserialize(xml);
                xml->exitbranch();
            }

            _parameters->Pefxroute[nefx] = static_cast<unsigned char>(xml->getpar("route", _parameters->Pefxroute[nefx], 0, NUM_TRACK_EFX));
            _parameters->partefx[nefx].setdryonly(_parameters->Pefxroute[nefx] == 2);
            _parameters->Pefxbypass[nefx] = xml->getparbool("bypass", _parameters->Pefxbypass[nefx]);
            xml->exitbranch();
        }
        xml->exitbranch();
    }
}

void TrackSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    _parameters->Penabled = static_cast<unsigned char>(xml->getparbool("enabled", _parameters->Penabled));

    _parameters->SetVolume(static_cast<unsigned char>(xml->getpar127("volume", _parameters->Pvolume)));
    _parameters->setPpanning(static_cast<unsigned char>(xml->getpar127("panning", _parameters->Ppanning)));

    _parameters->Pminkey = static_cast<unsigned char>(xml->getpar127("min_key", _parameters->Pminkey));
    _parameters->Pmaxkey = static_cast<unsigned char>(xml->getpar127("max_key", _parameters->Pmaxkey));
    _parameters->Pkeyshift = static_cast<unsigned char>(xml->getpar127("key_shift", _parameters->Pkeyshift));
    _parameters->Prcvchn = static_cast<unsigned char>(xml->getpar127("rcv_chn", _parameters->Prcvchn));

    _parameters->Pvelsns = static_cast<unsigned char>(xml->getpar127("velocity_sensing", _parameters->Pvelsns));
    _parameters->Pveloffs = static_cast<unsigned char>(xml->getpar127("velocity_offset", _parameters->Pveloffs));

    _parameters->Pnoteon = static_cast<unsigned char>(xml->getparbool("note_on", _parameters->Pnoteon));
    _parameters->Ppolymode = static_cast<unsigned char>(xml->getparbool("poly_mode", _parameters->Ppolymode));
    _parameters->Plegatomode = static_cast<unsigned char>(xml->getparbool("legato_mode", _parameters->Plegatomode)); // older versions
    if (!_parameters->Plegatomode)
    {
        _parameters->Plegatomode = static_cast<unsigned char>(xml->getpar127("legato_mode", _parameters->Plegatomode));
    }
    _parameters->Pkeylimit = static_cast<unsigned char>(xml->getpar127("key_limit", _parameters->Pkeylimit));

    if (xml->enterbranch("INSTRUMENT"))
    {
        DeserializeInstrument(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("CONTROLLER"))
    {
        ControllerSerializer(&_parameters->ctl).Deserialize(xml);
        xml->exitbranch();
    }
}
