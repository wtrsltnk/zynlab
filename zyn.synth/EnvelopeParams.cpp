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

#include "EnvelopeParams.h"
#include <cmath>

EnvelopeParams::EnvelopeParams(unsigned char envstretch, unsigned char forcedrelease)
{
    Penvstretch = envstretch;
    Pforcedrelease = forcedrelease;
    Plinearenvelope = 0;
    PA_dt = 10;
    PD_dt = 10;
    PR_dt = 10;
    PA_val = 64;
    PD_val = 64;
    PS_val = 64;
    PR_val = 64;
    Pfreemode = 1;

    for (int i = 0; i < MAX_ENVELOPE_POINTS; ++i)
    {
        Penvdt[i] = 32;
        Penvval[i] = 64;
    }
    Penvdt[0] = 0; //no used
    Penvsustain = 1;
    Penvpoints = 1;
    Envmode = 1;

    store2defaults();
}

EnvelopeParams::~EnvelopeParams() = default;

void EnvelopeParams::Defaults()
{
    Penvstretch = Denvstretch;
    Pforcedrelease = Dforcedrelease;
    Plinearenvelope = Dlinearenvelope;
    PA_dt = DA_dt;
    PD_dt = DD_dt;
    PR_dt = DR_dt;
    PA_val = DA_val;
    PD_val = DD_val;
    PS_val = DS_val;
    PR_val = DR_val;
    Pfreemode = 0;
    EnvelopeParams::ConvertToFree(this);
}

void EnvelopeParams::store2defaults()
{
    Denvstretch = Penvstretch;
    Dforcedrelease = Pforcedrelease;
    Dlinearenvelope = Plinearenvelope;
    DA_dt = PA_dt;
    DD_dt = PD_dt;
    DR_dt = PR_dt;
    DA_val = PA_val;
    DD_val = PD_val;
    DS_val = PS_val;
    DR_val = PR_val;
}

float EnvelopeParams::getdt(char i)
{
    return (powf(2.0f, Penvdt[static_cast<int>(i)] / 127.0f * 12.0f) - 1.0f) * 10.0f; //miliseconds
}

/*
 * ADSR/ASR... initialisations
 */
EnvelopeParams *EnvelopeParams::ADSRinit(
    unsigned char envstretch,
    unsigned char forcedrelease,
    unsigned char A_dt,
    unsigned char D_dt,
    unsigned char S_val,
    unsigned char R_dt)
{
    auto result = new EnvelopeParams(envstretch, forcedrelease);

    result->setpresettype("Penvamplitude");
    result->Envmode = 1;
    result->PA_dt = A_dt;
    result->PD_dt = D_dt;
    result->PS_val = S_val;
    result->PR_dt = R_dt;
    result->Pfreemode = 0;
    EnvelopeParams::ConvertToFree(result);
    result->store2defaults();

    return result;
}

EnvelopeParams *EnvelopeParams::ADSRinit_dB(
    unsigned char envstretch,
    unsigned char forcedrelease,
    unsigned char A_dt,
    unsigned char D_dt,
    unsigned char S_val,
    unsigned char R_dt)
{
    auto result = new EnvelopeParams(envstretch, forcedrelease);

    result->setpresettype("Penvamplitude");
    result->Envmode = 2;
    result->PA_dt = A_dt;
    result->PD_dt = D_dt;
    result->PS_val = S_val;
    result->PR_dt = R_dt;
    result->Pfreemode = 0;
    EnvelopeParams::ConvertToFree(result);
    result->store2defaults();

    return result;
}

EnvelopeParams *EnvelopeParams::ASRinit(
    unsigned char envstretch,
    unsigned char forcedrelease,
    unsigned char A_val,
    unsigned char A_dt,
    unsigned char R_val,
    unsigned char R_dt)
{
    auto result = new EnvelopeParams(envstretch, forcedrelease);

    result->setpresettype("Penvfrequency");
    result->Envmode = 3;
    result->PA_val = A_val;
    result->PA_dt = A_dt;
    result->PR_val = R_val;
    result->PR_dt = R_dt;
    result->Pfreemode = 0;
    EnvelopeParams::ConvertToFree(result);
    result->store2defaults();

    return result;
}

EnvelopeParams *EnvelopeParams::ADSRinit_filter(
    unsigned char envstretch,
    unsigned char forcedrelease,
    unsigned char A_val,
    unsigned char A_dt,
    unsigned char D_val,
    unsigned char D_dt,
    unsigned char R_dt,
    unsigned char R_val)
{
    auto result = new EnvelopeParams(envstretch, forcedrelease);

    result->setpresettype("Penvfilter");
    result->Envmode = 4;
    result->PA_val = A_val;
    result->PA_dt = A_dt;
    result->PD_val = D_val;
    result->PD_dt = D_dt;
    result->PR_dt = R_dt;
    result->PR_val = R_val;
    result->Pfreemode = 0;
    EnvelopeParams::ConvertToFree(result);
    result->store2defaults();

    return result;
}

EnvelopeParams *EnvelopeParams::ASRinit_bw(
    unsigned char envstretch,
    unsigned char forcedrelease,
    unsigned char A_val,
    unsigned char A_dt,
    unsigned char R_val,
    unsigned char R_dt)
{
    auto result = new EnvelopeParams(envstretch, forcedrelease);

    result->setpresettype("Penvbandwidth");
    result->Envmode = 5;
    result->PA_val = A_val;
    result->PA_dt = A_dt;
    result->PR_val = R_val;
    result->PR_dt = R_dt;
    result->Pfreemode = 0;
    EnvelopeParams::ConvertToFree(result);
    result->store2defaults();

    return result;
}

/*
 * Convert the Envelope to freemode
 */
void EnvelopeParams::ConvertToFree(EnvelopeParams *envelope)
{
    switch (envelope->Envmode)
    {
        case 1:
            envelope->Penvpoints = 4;
            envelope->Penvsustain = 2;
            envelope->Penvval[0] = 0;
            envelope->Penvdt[1] = envelope->PA_dt;
            envelope->Penvval[1] = 127;
            envelope->Penvdt[2] = envelope->PD_dt;
            envelope->Penvval[2] = envelope->PS_val;
            envelope->Penvdt[3] = envelope->PR_dt;
            envelope->Penvval[3] = 0;
            break;
        case 2:
            envelope->Penvpoints = 4;
            envelope->Penvsustain = 2;
            envelope->Penvval[0] = 0;
            envelope->Penvdt[1] = envelope->PA_dt;
            envelope->Penvval[1] = 127;
            envelope->Penvdt[2] = envelope->PD_dt;
            envelope->Penvval[2] = envelope->PS_val;
            envelope->Penvdt[3] = envelope->PR_dt;
            envelope->Penvval[3] = 0;
            break;
        case 3:
            envelope->Penvpoints = 3;
            envelope->Penvsustain = 1;
            envelope->Penvval[0] = envelope->PA_val;
            envelope->Penvdt[1] = envelope->PA_dt;
            envelope->Penvval[1] = 64;
            envelope->Penvdt[2] = envelope->PR_dt;
            envelope->Penvval[2] = envelope->PR_val;
            break;
        case 4:
            envelope->Penvpoints = 4;
            envelope->Penvsustain = 2;
            envelope->Penvval[0] = envelope->PA_val;
            envelope->Penvdt[1] = envelope->PA_dt;
            envelope->Penvval[1] = envelope->PD_val;
            envelope->Penvdt[2] = envelope->PD_dt;
            envelope->Penvval[2] = 64;
            envelope->Penvdt[3] = envelope->PR_dt;
            envelope->Penvval[3] = envelope->PR_val;
            break;
        case 5:
            envelope->Penvpoints = 3;
            envelope->Penvsustain = 1;
            envelope->Penvval[0] = envelope->PA_val;
            envelope->Penvdt[1] = envelope->PA_dt;
            envelope->Penvval[1] = 64;
            envelope->Penvdt[2] = envelope->PR_dt;
            envelope->Penvval[2] = envelope->PR_val;
            break;
    }
}

void EnvelopeParams::InitPresets()
{
    AddPresetAsBool("free_mode", &Pfreemode);
    AddPreset("env_points", &Penvpoints);
    AddPreset("env_sustain", &Penvsustain);
    AddPreset("env_stretch", &Penvstretch);
    AddPresetAsBool("forced_release", &Pforcedrelease);
    AddPresetAsBool("linear_envelope", &Plinearenvelope);
    AddPreset("A_dt", &PA_dt);
    AddPreset("D_dt", &PD_dt);
    AddPreset("R_dt", &PR_dt);
    AddPreset("A_val", &PA_val);
    AddPreset("D_val", &PD_val);
    AddPreset("S_val", &PS_val);
    AddPreset("R_val", &PR_val);

    for (int i = 0; i < Penvpoints; ++i)
    {
        Preset point("POINT");
        if (i != 0) point.AddPreset("dt", &Penvdt[i]);
        point.AddPreset("val", &Penvval[i]);
        AddContainer(i, point);
    }
}

void EnvelopeParams::Serialize(IPresetsSerializer *xml)
{
    xml->addparbool("free_mode", Pfreemode);
    xml->addpar("env_points", Penvpoints);
    xml->addpar("env_sustain", Penvsustain);
    xml->addpar("env_stretch", Penvstretch);
    xml->addparbool("forced_release", Pforcedrelease);
    xml->addparbool("linear_envelope", Plinearenvelope);
    xml->addpar("A_dt", PA_dt);
    xml->addpar("D_dt", PD_dt);
    xml->addpar("R_dt", PR_dt);
    xml->addpar("A_val", PA_val);
    xml->addpar("D_val", PD_val);
    xml->addpar("S_val", PS_val);
    xml->addpar("R_val", PR_val);

    if ((Pfreemode != 0) || (!xml->minimal))
    {
        for (int i = 0; i < Penvpoints; ++i)
        {
            xml->beginbranch("POINT", i);
            if (i != 0)
                xml->addpar("dt", Penvdt[i]);
            xml->addpar("val", Penvval[i]);
            xml->endbranch();
        }
    }
}

void EnvelopeParams::Deserialize(IPresetsSerializer *xml)
{
    Pfreemode = xml->getparbool("free_mode", Pfreemode);
    Penvpoints = xml->getpar127("env_points", Penvpoints);
    Penvsustain = xml->getpar127("env_sustain", Penvsustain);
    Penvstretch = xml->getpar127("env_stretch", Penvstretch);
    Pforcedrelease = xml->getparbool("forced_release", Pforcedrelease);
    Plinearenvelope = xml->getparbool("linear_envelope", Plinearenvelope);

    PA_dt = xml->getpar127("A_dt", PA_dt);
    PD_dt = xml->getpar127("D_dt", PD_dt);
    PR_dt = xml->getpar127("R_dt", PR_dt);
    PA_val = xml->getpar127("A_val", PA_val);
    PD_val = xml->getpar127("D_val", PD_val);
    PS_val = xml->getpar127("S_val", PS_val);
    PR_val = xml->getpar127("R_val", PR_val);

    for (int i = 0; i < Penvpoints; ++i)
    {
        if (xml->enterbranch("POINT", i) == 0)
            continue;
        if (i != 0)
            Penvdt[i] = xml->getpar127("dt", Penvdt[i]);
        Penvval[i] = xml->getpar127("val", Penvval[i]);
        xml->exitbranch();
    }

    if (!Pfreemode)
    {
        EnvelopeParams::ConvertToFree(this);
    }
}
