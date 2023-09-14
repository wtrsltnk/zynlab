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

LFOParams::LFOParams(
    unsigned char freq,
    unsigned char intensity,
    unsigned char startphase,
    unsigned char LFOtype,
    unsigned char randomness,
    unsigned char delay,
    unsigned char continous,
    unsigned char fel)
{
    switch (fel)
    {
        case 0:
        {
            setpresettype("Plfofrequency");
            break;
        }
        case 1:
        {
            setpresettype("Plfoamplitude");
            break;
        }
        case 2:
        {
            setpresettype("Plfofilter");
            break;
        }
    }
    Dfreq = freq;
    Dintensity = intensity;
    Dstartphase = startphase;
    DLFOtype = LFOtype;
    Drandomness = randomness;
    Ddelay = delay;
    Dcontinous = continous;
    Pkind = fel;
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
    _presets.clear();

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
