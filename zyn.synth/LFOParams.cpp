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

#include "LFOParams.h"
#include <cmath>

int LFOParams::time;

LFOParams::LFOParams(unsigned char Pfreq_,
                     unsigned char Pintensity_,
                     unsigned char Pstartphase_,
                     unsigned char PLFOtype_,
                     unsigned char Prandomness_,
                     unsigned char Pdelay_,
                     unsigned char Pcontinous_,
                     unsigned char fel_)
{
    switch (fel_)
    {
        case 0:
            setpresettype("Plfofrequency");
            break;
        case 1:
            setpresettype("Plfoamplitude");
            break;
        case 2:
            setpresettype("Plfofilter");
            break;
    }
    Dfreq = Pfreq_;
    Dintensity = Pintensity_;
    Dstartphase = Pstartphase_;
    DLFOtype = PLFOtype_;
    Drandomness = Prandomness_;
    Ddelay = Pdelay_;
    Dcontinous = Pcontinous_;
    fel = fel_;
    time = 0;

    Defaults();
}

LFOParams::~LFOParams() = default;

void LFOParams::Defaults()
{
    Pfreq = Dfreq / 127.0f;
    Pintensity = Dintensity;
    Pstartphase = Dstartphase;
    PLFOtype = DLFOtype;
    Prandomness = Drandomness;
    Pdelay = Ddelay;
    Pcontinous = Dcontinous;
    Pfreqrand = 0;
    Pstretch = 64;
}

void LFOParams::InitPresets()
{
    AddPreset("freq", &Pfreq);
    AddPreset("intensity", &Pintensity);
    AddPreset("start_phase", &Pstartphase);
    AddPreset("lfo_type", &PLFOtype);
    AddPreset("randomness_amplitude", &Prandomness);
    AddPreset("randomness_frequency", &Pfreqrand);
    AddPreset("delay", &Pdelay);
    AddPreset("stretch", &Pstretch);
    AddPresetAsBool("continous", &Pcontinous);
}

void LFOParams::Serialize(IPresetsSerializer *xml)
{
    xml->addparreal("freq", Pfreq);
    xml->addpar("intensity", Pintensity);
    xml->addpar("start_phase", Pstartphase);
    xml->addpar("lfo_type", PLFOtype);
    xml->addpar("randomness_amplitude", Prandomness);
    xml->addpar("randomness_frequency", Pfreqrand);
    xml->addpar("delay", Pdelay);
    xml->addpar("stretch", Pstretch);
    xml->addparbool("continous", Pcontinous);
}

void LFOParams::Deserialize(IPresetsSerializer *xml)
{
    Pfreq = xml->getparreal("freq", Pfreq, 0.0f, 1.0f);
    Pintensity = xml->getpar127("intensity", Pintensity);
    Pstartphase = xml->getpar127("start_phase", Pstartphase);
    PLFOtype = xml->getpar127("lfo_type", PLFOtype);
    Prandomness = xml->getpar127("randomness_amplitude", Prandomness);
    Pfreqrand = xml->getpar127("randomness_frequency", Pfreqrand);
    Pdelay = xml->getpar127("delay", Pdelay);
    Pstretch = xml->getpar127("stretch", Pstretch);
    Pcontinous = xml->getparbool("continous", Pcontinous) == 1 ? 1 : 0;
}
