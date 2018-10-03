/*
  ZynAddSubFX - a software synthesizer

  Echo.cpp - Echo effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Copyright (C) 2009-2010 Mark McCurry
  Author: Nasca Octavian Paul
          Mark McCurry

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

#include "Echo.h"
#include <cmath>

#define MAX_DELAY 2

Echo::Echo(bool insertion_, float *efxoutl_, float *efxoutr_, SystemSettings *synth_)
    : Effect(insertion_, efxoutl_, efxoutr_, nullptr, 0, synth_),
      Pvolume(50),
      Pdelay(60),
      Plrdelay(100),
      Pfb(40),
      Phidamp(60),
      delayTime(1),
      lrdelay(0),
      avgDelay(0),
      delay(new float[(int)(MAX_DELAY * synth_->samplerate)],
            new float[(int)(MAX_DELAY * synth_->samplerate)]),
      old(0.0f),
      pos(0),
      delta(1),
      ndelta(1)
{
    initdelays();
    SetPreset(Ppreset);
}

Echo::~Echo()
{
    delete[] delay._left;
    delete[] delay._right;
}

//Cleanup the effect
void Echo::Cleanup()
{
    memset(delay._left, 0, MAX_DELAY * this->_synth->samplerate * sizeof(float));
    memset(delay._right, 0, MAX_DELAY * this->_synth->samplerate * sizeof(float));
    old = Stereo<float>(0.0f);
}

inline int max(int a, int b)
{
    return a > b ? a : b;
}

//Initialize the delays
void Echo::initdelays()
{
    Cleanup();
    //number of seconds to delay left chan
    float dl = avgDelay - lrdelay;

    //number of seconds to delay right chan
    float dr = avgDelay + lrdelay;

    ndelta._left = max(1, (int)(dl * this->_synth->samplerate));
    ndelta._right = max(1, (int)(dr * this->_synth->samplerate));
}

//Effect output
void Echo::out(const Stereo<float *> &input)
{
    for (int i = 0; i < this->_synth->buffersize; ++i)
    {
        float ldl = delay._left[pos._left];
        float rdl = delay._right[pos._right];
        ldl = ldl * (1.0f - lrcross) + rdl * lrcross;
        rdl = rdl * (1.0f - lrcross) + ldl * lrcross;

        efxoutl[i] = ldl * 2.0f;
        efxoutr[i] = rdl * 2.0f;

        ldl = input._left[i] * pangainL - ldl * fb;
        rdl = input._right[i] * pangainR - rdl * fb;

        //LowPass Filter
        old._left = delay._left[(pos._left + delta._left) % (MAX_DELAY * this->_synth->samplerate)] =
            ldl * hidamp + old._left * (1.0f - hidamp);
        old._right = delay._right[(pos._right + delta._right) % (MAX_DELAY * this->_synth->samplerate)] =
            rdl * hidamp + old._right * (1.0f - hidamp);

        //increment
        ++pos._left; // += delta.l;
        ++pos._right; // += delta.r;

        //ensure that pos is still in bounds
        pos._left %= MAX_DELAY * this->_synth->samplerate;
        pos._right %= MAX_DELAY * this->_synth->samplerate;

        //adjust delay if needed
        delta._left = (15 * delta._left + ndelta._left) / 16;
        delta._right = (15 * delta._right + ndelta._right) / 16;
    }
}

//Parameter control
void Echo::setvolume(unsigned char _Pvolume)
{
    Pvolume = _Pvolume;

    if (insertion == 0)
    {
        outvolume = powf(0.01f, (1.0f - Pvolume / 127.0f)) * 4.0f;
        volume = 1.0f;
    }
    else
        volume = outvolume = Pvolume / 127.0f;
    if (Pvolume == 0)
        Cleanup();
}

void Echo::setdelay(unsigned char _Pdelay)
{
    Pdelay = _Pdelay;
    avgDelay = (Pdelay / 127.0f * 1.5f); //0 .. 1.5 sec
    initdelays();
}

void Echo::setlrdelay(unsigned char _Plrdelay)
{
    float tmp;
    Plrdelay = _Plrdelay;
    tmp =
        (powf(2.0f, fabsf(Plrdelay - 64.0f) / 64.0f * 9.0f) - 1.0f) / 1000.0f;
    if (Plrdelay < 64.0f)
        tmp = -tmp;
    lrdelay = tmp;
    initdelays();
}

void Echo::setfb(unsigned char _Pfb)
{
    Pfb = _Pfb;
    fb = Pfb / 128.0f;
}

void Echo::sethidamp(unsigned char _Phidamp)
{
    Phidamp = _Phidamp;
    hidamp = 1.0f - Phidamp / 127.0f;
}

void Echo::SetPreset(unsigned char npreset)
{
    const int PRESET_SIZE = 7;
    const int NUM_PRESETS = 9;
    unsigned char presets[NUM_PRESETS][PRESET_SIZE] = {
        {67, 64, 35, 64, 30, 59, 0},    //Echo 1
        {67, 64, 21, 64, 30, 59, 0},    //Echo 2
        {67, 75, 60, 64, 30, 59, 10},   //Echo 3
        {67, 60, 44, 64, 30, 0, 0},     //Simple Echo
        {67, 60, 102, 50, 30, 82, 48},  //Canyon
        {67, 64, 44, 17, 0, 82, 24},    //Panning Echo 1
        {81, 60, 46, 118, 100, 68, 18}, //Panning Echo 2
        {81, 60, 26, 100, 127, 67, 36}, //Panning Echo 3
        {62, 64, 28, 64, 100, 90, 55}   //Feedback Echo
    };

    if (npreset >= NUM_PRESETS)
        npreset = NUM_PRESETS - 1;
    for (int n = 0; n < PRESET_SIZE; ++n)
        ChangeParameter(n, presets[npreset][n]);
    if (insertion)
        setvolume(presets[npreset][0] / 2); //lower the volume if this is insertion effect
    Ppreset = npreset;
}

void Echo::ChangeParameter(int npar, unsigned char value)
{
    switch (npar)
    {
        case 0:
            setvolume(value);
            break;
        case 1:
            SetPanning(value);
            break;
        case 2:
            setdelay(value);
            break;
        case 3:
            setlrdelay(value);
            break;
        case 4:
            SetLRCross(value);
            break;
        case 5:
            setfb(value);
            break;
        case 6:
            sethidamp(value);
            break;
    }
}

unsigned char Echo::GetParameter(int npar) const
{
    switch (npar)
    {
        case 0:
            return Pvolume;
        case 1:
            return Ppanning;
        case 2:
            return Pdelay;
        case 3:
            return Plrdelay;
        case 4:
            return Plrcross;
        case 5:
            return Pfb;
        case 6:
            return Phidamp;
        default:
            return 0; // in case of bogus parameter number
    }
}
