/*
  ZynAddSubFX - a software synthesizer

  Distorsion.cpp - Distorsion effect
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

#include "Distorsion.h"

#include "EffectPresets.h"
#include <cmath>
#include <zyn.common/WaveShapeSmps.h>
#include <zyn.dsp/AnalogFilter.h>

Distorsion::Distorsion(bool insertion_, float *efxoutl_, float *efxoutr_)
    : Effect(insertion_, efxoutl_, efxoutr_, nullptr, 0),
      Pvolume(50),
      Pdrive(90),
      Plevel(64),
      Ptype(0),
      Pnegate(0),
      Plpf(127),
      Phpf(0),
      Pstereo(0),
      Pprefiltering(0)
{
    lpfl = new AnalogFilter(2, 22000, 1, 0);
    lpfr = new AnalogFilter(2, 22000, 1, 0);
    hpfl = new AnalogFilter(3, 20, 1, 0);
    hpfr = new AnalogFilter(3, 20, 1, 0);
    SetPreset(Ppreset);
    Cleanup();
}

Distorsion::~Distorsion()
{
    delete lpfl;
    delete lpfr;
    delete hpfl;
    delete hpfr;
}

//Cleanup the effect
void Distorsion::Cleanup()
{
    lpfl->cleanup();
    hpfl->cleanup();
    lpfr->cleanup();
    hpfr->cleanup();
}

//Apply the filters
void Distorsion::applyfilters(float *efxoutl, float *efxoutr)
{
    lpfl->filterout(efxoutl);
    hpfl->filterout(efxoutl);
    if (Pstereo != 0)
    { //stereo
        lpfr->filterout(efxoutr);
        hpfr->filterout(efxoutr);
    }
}

//Effect output
void Distorsion::out(const Stereo<float *> &smp)
{
    float inputvol = powf(5.0f, (Pdrive - 32.0f) / 127.0f);
    if (Pnegate)
    {
        inputvol *= -1.0f;
    }

    if (Pstereo) //Stereo
    {
        for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
        {
            efxoutl[i] = smp._left[i] * inputvol * pangainL;
            efxoutr[i] = smp._right[i] * inputvol * pangainR;
        }
    }
    else //Mono
    {
        for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
        {
            efxoutl[i] = (smp._left[i] * pangainL + smp._right[i] * pangainR) * inputvol;
        }
    }

    if (Pprefiltering)
    {
        applyfilters(efxoutl, efxoutr);
    }

    waveShapeSmps(SystemSettings::Instance().buffersize, efxoutl, Ptype + 1, Pdrive);
    if (Pstereo)
    {
        waveShapeSmps(SystemSettings::Instance().buffersize, efxoutr, Ptype + 1, Pdrive);
    }

    if (!Pprefiltering)
    {
        applyfilters(efxoutl, efxoutr);
    }

    if (!Pstereo)
    {
        memcpy(efxoutr, efxoutl, SystemSettings::Instance().bufferbytes);
    }

    float level = dB2rap(60.0f * Plevel / 127.0f - 40.0f);
    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        float lout = efxoutl[i];
        float rout = efxoutr[i];
        float l = lout * (1.0f - lrcross) + rout * lrcross;
        float r = rout * (1.0f - lrcross) + lout * lrcross;
        lout = l;
        rout = r;

        efxoutl[i] = lout * 2.0f * level;
        efxoutr[i] = rout * 2.0f * level;
    }
}

//Parameter control
void Distorsion::setvolume(unsigned char _Pvolume)
{
    Pvolume = _Pvolume;

    if (insertion == 0)
    {
        outvolume = powf(0.01f, (1.0f - Pvolume / 127.0f)) * 4.0f;
        volume = 1.0f;
    }
    else
    {
        volume = outvolume = Pvolume / 127.0f;
    }
    if (Pvolume == 0)
    {
        Cleanup();
    }
}

void Distorsion::setlpf(unsigned char _Plpf)
{
    Plpf = _Plpf;
    float fr = expf(powf(Plpf / 127.0f, 0.5f) * logf(25000.0f)) + 40.0f;
    lpfl->setfreq(fr);
    lpfr->setfreq(fr);
}

void Distorsion::sethpf(unsigned char _Phpf)
{
    Phpf = _Phpf;
    float fr = expf(powf(Phpf / 127.0f, 0.5f) * logf(25000.0f)) + 20.0f;
    hpfl->setfreq(fr);
    hpfr->setfreq(fr);
}

void Distorsion::SetPreset(unsigned char npreset)
{
    const int PRESET_SIZE = 11;
    const int NUM_PRESETS = 6;
    unsigned char presets[NUM_PRESETS][PRESET_SIZE] = {
        //Overdrive 1
        {127, 64, 35, 56, 70, 0, 0, 96, 0, 0, 0},
        //Overdrive 2
        {127, 64, 35, 29, 75, 1, 0, 127, 0, 0, 0},
        //A. Exciter 1
        {64, 64, 35, 75, 80, 5, 0, 127, 105, 1, 0},
        //A. Exciter 2
        {64, 64, 35, 85, 62, 1, 0, 127, 118, 1, 0},
        //Guitar Amp
        {127, 64, 35, 63, 75, 2, 0, 55, 0, 0, 0},
        //Quantisize
        {127, 64, 35, 88, 75, 4, 0, 127, 0, 1, 0}};

    if (npreset >= NUM_PRESETS)
    {
        npreset = NUM_PRESETS - 1;
    }
    for (int n = 0; n < PRESET_SIZE; ++n)
    {
        ChangeParameter(n, presets[npreset][n]);
    }
    if (!insertion) //lower the volume if this is system effect
    {
        ChangeParameter(EffectPresets::Volume, (int)(presets[npreset][0] / 1.5f));
    }
    Ppreset = npreset;
    Cleanup();
}

void Distorsion::ChangeParameter(int npar, unsigned char value)
{
    switch (npar)
    {
        case EffectPresets::Volume:
            setvolume(value);
            break;
        case EffectPresets::Panning:
            SetPanning(value);
            break;
        case DistorsionPresets::DistorsionChannelRouting:
            SetLRCross(value);
            break;
        case DistorsionPresets::DistorsionDrive:
            Pdrive = value;
            break;
        case DistorsionPresets::DistorsionLevel:
            Plevel = value;
            break;
        case DistorsionPresets::DistorsionType:
            if (value > 13)
            {
                Ptype = 13; //this must be increased if more distorsion types are added
            }
            else
            {
                Ptype = value;
            }
            break;
        case DistorsionPresets::DistorsionNegate:
            if (value > 1)
            {
                Pnegate = 1;
            }
            else
            {
                Pnegate = value;
            }
            break;
        case DistorsionPresets::DistorsionLowPassFilter:
            setlpf(value);
            break;
        case DistorsionPresets::DistorsionHighPassFilter:
            sethpf(value);
            break;
        case DistorsionPresets::DistorsionStereo:
            Pstereo = (value > 1) ? 1 : value;
            break;
        case DistorsionPresets::DistorsionPreFiltering:
            Pprefiltering = value;
            break;
    }
}

unsigned char Distorsion::GetParameter(int npar) const
{
    switch (npar)
    {
        case EffectPresets::Volume:
            return Pvolume;
        case EffectPresets::Panning:
            return Ppanning;
        case DistorsionPresets::DistorsionChannelRouting:
            return Plrcross;
        case DistorsionPresets::DistorsionDrive:
            return Pdrive;
        case DistorsionPresets::DistorsionLevel:
            return Plevel;
        case DistorsionPresets::DistorsionType:
            return Ptype;
        case DistorsionPresets::DistorsionNegate:
            return Pnegate;
        case DistorsionPresets::DistorsionLowPassFilter:
            return Plpf;
        case DistorsionPresets::DistorsionHighPassFilter:
            return Phpf;
        case DistorsionPresets::DistorsionStereo:
            return Pstereo;
        case DistorsionPresets::DistorsionPreFiltering:
            return Pprefiltering;
        default:
            return 0; //in case of bogus parameter number
    }
}
