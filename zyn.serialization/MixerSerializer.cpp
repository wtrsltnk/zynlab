/*
  ZynAddSubFX - a software synthesizer

  Mixer.cpp - It sends Midi Messages to Parts, receives samples from parts,
             process them with system/insertion effects and mix them
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

#include "MixerSerializer.h"

#include "EffectMgrSerializer.h"
#include "MicrotonalSerializer.h"
#include "TrackSerializer.h"

char const *MixerSerializer::BRANCH_NAME = "MASTER";

MixerSerializer::MixerSerializer(Mixer *parameters)
    : _parameters(parameters)
{}

MixerSerializer::~MixerSerializer() = default;

void MixerSerializer::Serialize(IPresetsSerializer *xml)
{
    xml->addpar("volume", _parameters->Pvolume);
    xml->addpar("key_shift", _parameters->Pkeyshift);
    xml->addparbool("nrpn_receive", _parameters->ctl.NRPN.receive);

    xml->beginbranch("MICROTONAL");
    MicrotonalSerializer(&_parameters->microtonal).Serialize(xml);
    xml->endbranch();

    for (int npart = 0; npart < NUM_MIXER_TRACKS; ++npart)
    {
        xml->beginbranch("PART", npart);
        TrackSerializer(&_parameters->_tracks[npart]).Serialize(xml);
        xml->endbranch();
    }

    xml->beginbranch("SYSTEM_EFFECTS");
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        xml->beginbranch("SYSTEM_EFFECT", nefx);
        xml->beginbranch("EFFECT");
        EffectManagerSerializer(&_parameters->sysefx[nefx]).Serialize(xml);
        xml->endbranch();

        for (int pefx = 0; pefx < NUM_MIXER_TRACKS; ++pefx)
        {
            xml->beginbranch("VOLUME", pefx);
            xml->addpar("vol", _parameters->Psysefxvol[nefx][pefx]);
            xml->endbranch();
        }

        for (int tonefx = nefx + 1; tonefx < NUM_SYS_EFX; ++tonefx)
        {
            xml->beginbranch("SENDTO", tonefx);
            xml->addpar("send_vol", _parameters->Psysefxsend[nefx][tonefx]);
            xml->endbranch();
        }

        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("INSERTION_EFFECTS");
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        xml->beginbranch("INSERTION_EFFECT", nefx);
        xml->addpar("part", _parameters->Pinsparts[nefx]);

        xml->beginbranch("EFFECT");
        EffectManagerSerializer(&_parameters->insefx[nefx]).Serialize(xml);
        xml->endbranch();
        xml->endbranch();
    }

    xml->endbranch();
}

void MixerSerializer::Deserialize(IPresetsSerializer *xml)
{
    _parameters->SetVolume(static_cast<unsigned char>(xml->getpar127("volume", _parameters->Pvolume)));
    _parameters->SetKeyShift(static_cast<unsigned char>(xml->getpar127("key_shift", _parameters->Pkeyshift)));
    _parameters->ctl.NRPN.receive = static_cast<unsigned char>(xml->getparbool("nrpn_receive", _parameters->ctl.NRPN.receive));

    _parameters->_tracks[0].Penabled = 0;
    for (int npart = 0; npart < NUM_MIXER_TRACKS; ++npart)
    {
        if (xml->enterbranch("PART", npart) == 0)
        {
            continue;
        }
        TrackSerializer(&_parameters->_tracks[npart]).Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("MICROTONAL"))
    {
        MicrotonalSerializer(&_parameters->microtonal).Deserialize(xml);
        xml->exitbranch();
    }

    _parameters->sysefx[0].changeeffect(0);
    if (xml->enterbranch("SYSTEM_EFFECTS"))
    {
        for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
        {
            if (xml->enterbranch("SYSTEM_EFFECT", nefx) == 0)
            {
                continue;
            }
            if (xml->enterbranch("EFFECT"))
            {
                EffectManagerSerializer(&_parameters->sysefx[nefx]).Deserialize(xml);
                xml->exitbranch();
            }

            for (int partefx = 0; partefx < NUM_MIXER_TRACKS; ++partefx)
            {
                if (xml->enterbranch("VOLUME", partefx) == 0)
                {
                    continue;
                }
                _parameters->SetSystemEffectVolume(partefx, nefx, static_cast<unsigned char>(xml->getpar127("vol", _parameters->Psysefxvol[partefx][nefx])));
                xml->exitbranch();
            }

            for (int tonefx = nefx + 1; tonefx < NUM_SYS_EFX; ++tonefx)
            {
                if (xml->enterbranch("SENDTO", tonefx) == 0)
                {
                    continue;
                }
                _parameters->SetSystemSendEffectVolume(nefx, tonefx, static_cast<unsigned char>(xml->getpar127("send_vol", _parameters->Psysefxsend[nefx][tonefx])));
                xml->exitbranch();
            }
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("INSERTION_EFFECTS"))
    {
        for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
        {
            if (xml->enterbranch("INSERTION_EFFECT", nefx) == 0)
            {
                continue;
            }
            _parameters->Pinsparts[nefx] = static_cast<short>(xml->getpar("part", _parameters->Pinsparts[nefx], -2, NUM_MIXER_TRACKS));
            if (xml->enterbranch("EFFECT"))
            {
                EffectManagerSerializer(&_parameters->insefx[nefx]).Deserialize(xml);
                xml->exitbranch();
            }
            xml->exitbranch();
        }

        xml->exitbranch();
    }
}
