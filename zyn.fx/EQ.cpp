/*
  ZynAddSubFX - a software synthesizer

  EQ.cpp - EQ effect
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

#include "EQ.h"
#include <zyn.dsp/AnalogFilter.h>

#include <cmath>

EQ::EQ(bool insertion_, float *efxoutl_, float *efxoutr_, SystemSettings *synth_)
    : Effect(insertion_, efxoutl_, efxoutr_, nullptr, 0, synth_)
{
    for (auto &i : filter)
    {
        i.Ptype = 0;
        i.Pfreq = 64;
        i.Pgain = 64;
        i.Pq = 64;
        i.Pstages = 0;
        i.l = new AnalogFilter(6, 1000.0f, 1.0f, 0, this->_synth);
        i.r = new AnalogFilter(6, 1000.0f, 1.0f, 0, this->_synth);
    }
    //default values
    Pvolume = 50;

    SetPreset(Ppreset);
    Cleanup();
}

// Cleanup the effect
void EQ::Cleanup()
{
    for (auto &i : filter)
    {
        i.l->cleanup();
        i.r->cleanup();
    }
}

//Effect output
void EQ::out(const Stereo<float *> &smp)
{
    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        efxoutl[i] = smp._left[i] * volume;
        efxoutr[i] = smp._right[i] * volume;
    }

    for (auto &i : filter)
    {
        if (i.Ptype == 0)
        {
            continue;
        }
        i.l->filterout(efxoutl);
        i.r->filterout(efxoutr);
    }
}

//Parameter control
void EQ::setvolume(unsigned char _Pvolume)
{
    Pvolume = _Pvolume;
    outvolume = powf(0.005f, (1.0f - Pvolume / 127.0f)) * 10.0f;
    volume = (!insertion) ? 1.0f : outvolume;
}

void EQ::SetPreset(unsigned char npreset)
{
    const int PRESET_SIZE = 1;
    const int NUM_PRESETS = 2;
    unsigned char presets[NUM_PRESETS][PRESET_SIZE] = {
        {67}, //EQ 1
        {67}  //EQ 2
    };

    if (npreset >= NUM_PRESETS)
    {
        npreset = NUM_PRESETS - 1;
    }
    for (int n = 0; n < PRESET_SIZE; ++n)
    {
        ChangeParameter(n, presets[npreset][n]);
    }
    Ppreset = npreset;
}

void EQ::ChangeParameter(int npar, unsigned char value)
{
    switch (npar)
    {
        case 0:
        {
            setvolume(value);
            break;
        }
    }
    if (npar < 10)
    {
        return;
    }

    int nb = (npar - 10) / 5; //number of the band (filter)
    if (nb >= MAX_EQ_BANDS)
    {
        return;
    }
    int bp = npar % 5; //band paramenter

    float tmp;
    switch (bp)
    {
        case 0:
        {
            filter[nb].Ptype = value;
            if (value > 9)
            {
                filter[nb].Ptype = 0; //has to be changed if more filters will be added
            }
            if (filter[nb].Ptype != 0)
            {
                filter[nb].l->settype(value - 1);
                filter[nb].r->settype(value - 1);
            }
            break;
        }
        case 1:
        {
            filter[nb].Pfreq = value;
            tmp = 600.0f * powf(30.0f, (value - 64.0f) / 64.0f);
            filter[nb].l->setfreq(tmp);
            filter[nb].r->setfreq(tmp);
            break;
        }
        case 2:
        {
            filter[nb].Pgain = value;
            tmp = 30.0f * (value - 64.0f) / 64.0f;
            filter[nb].l->setgain(tmp);
            filter[nb].r->setgain(tmp);
            break;
        }
        case 3:
        {
            filter[nb].Pq = value;
            tmp = powf(30.0f, (value - 64.0f) / 64.0f);
            filter[nb].l->setq(tmp);
            filter[nb].r->setq(tmp);
            break;
        }
        case 4:
        {
            filter[nb].Pstages = value;
            if (value >= MAX_FILTER_STAGES)
            {
                filter[nb].Pstages = MAX_FILTER_STAGES - 1;
            }
            filter[nb].l->setstages(value);
            filter[nb].r->setstages(value);
            break;
        }
    }
}

unsigned char EQ::GetParameter(int npar) const
{
    switch (npar)
    {
        case 0:
        {
            return Pvolume;
        }
    }

    if (npar < 10)
    {
        return 0;
    }

    int nb = (npar - 10) / 5; //number of the band (filter)
    if (nb >= MAX_EQ_BANDS)
    {
        return 0;
    }

    int bp = npar % 5; //band paramenter
    switch (bp)
    {
        case 0:
        {
            return filter[nb].Ptype;
        }
        case 1:
        {
            return filter[nb].Pfreq;
        }
        case 2:
        {
            return filter[nb].Pgain;
        }
        case 3:
        {
            return filter[nb].Pq;
        }
        case 4:
        {
            return filter[nb].Pstages;
        }
        default:
        {
            return 0; //in case of bogus parameter number
        }
    }
}

float EQ::GetFrequencyResponse(float freq)
{
    float resp = 1.0f;
    for (auto &i : filter)
    {
        if (i.Ptype == 0)
        {
            continue;
        }
        resp *= i.l->H(freq);
    }

    return rap2dB(resp * outvolume);
}
