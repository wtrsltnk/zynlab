/*

  Phaser.cpp  - Phasing and Approximate digital model of an analog JFET phaser.
  Analog modeling implemented by Ryan Billing aka Transmogrifox.
  ZynAddSubFX - a software synthesizer

  Phaser.cpp - Phaser effect
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Copyright (C) 2009-2010 Ryan Billing
  Copyright (C) 2010-2010 Mark McCurry
  Author: Nasca Octavian Paul
          Ryan Billing
          Mark McCurry

  DSP analog modeling theory & practice largely influenced by various CCRMA publications, particularly works by Julius O. Smith.

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

#include "Phaser.h"
#include <algorithm>
#include <cmath>

using namespace std;

#define PHASER_LFO_SHAPE 2
#define ONE_ 0.99999f  // To prevent LFO ever reaching 1.0f for filter stability purposes
#define ZERO_ 0.00001f // Same idea as above.

Phaser::Phaser(const int &insertion_, float *efxoutl_, float *efxoutr_, SystemSettings *synth_)
    : Effect(insertion_, efxoutl_, efxoutr_, nullptr, 0, synth_), lfo(synth_), old(nullptr), xn1(nullptr),
      yn1(nullptr), diff(0.0f), oldgain(0.0f), fb(0.0f)
{
    analog_setup();
    SetPreset(Ppreset);
    Cleanup();
}

void Phaser::analog_setup()
{
    //model mismatch between JFET devices
    offset[0] = -0.2509303f;
    offset[1] = 0.9408924f;
    offset[2] = 0.998f;
    offset[3] = -0.3486182f;
    offset[4] = -0.2762545f;
    offset[5] = -0.5215785f;
    offset[6] = 0.2509303f;
    offset[7] = -0.9408924f;
    offset[8] = -0.998f;
    offset[9] = 0.3486182f;
    offset[10] = 0.2762545f;
    offset[11] = 0.5215785f;

    barber = false; //Deactivate barber pole phasing by default

    mis = 1.0f;
    Rmin = 625.0f;   // 2N5457 typical on resistance at Vgs = 0
    Rmax = 22000.0f; // Resistor parallel to FET
    Rmx = Rmin / Rmax;
    Rconst = 1.0f + Rmx; // Handle parallel resistor relationship
    C = 0.00000005f;     // 50 nF
    CFs = 2.0f * this->_synth->samplerate_f * C;
    invperiod = 1.0f / this->_synth->buffersize_f;
}

Phaser::~Phaser()
{
    delete[] old._left;
    delete[] xn1._left;
    delete[] yn1._left;
    delete[] old._right;
    delete[] xn1._right;
    delete[] yn1._right;
}

/*
 * Effect output
 */
void Phaser::out(const Stereo<float *> &input)
{
    if (Panalog)
    {
        AnalogPhase(input);
    }
    else
    {
        normalPhase(input);
    }
}

void Phaser::AnalogPhase(const Stereo<float *> &input)
{
    Stereo<float> gain(0.0f), lfoVal(0.0f), mod(0.0f), g(0.0f), b(0.0f), hpf(0.0f);

    lfo.effectlfoout(&lfoVal._left, &lfoVal._right);
    mod._left = lfoVal._left * width + (depth - 0.5f);
    mod._right = lfoVal._right * width + (depth - 0.5f);

    mod._left = limit(mod._left, ZERO_, ONE_);
    mod._right = limit(mod._right, ZERO_, ONE_);

    if (Phyper)
    {
        //Triangle wave squared is approximately sin on bottom, tri on top
        //Result is exponential sweep more akin to filter in synth with
        //exponential generator circuitry.
        mod._left *= mod._left;
        mod._right *= mod._right;
    }

    //g.l,g.r is Vp - Vgs. Typical FET drain-source resistance follows constant/[1-sqrt(Vp - Vgs)]
    mod._left = sqrtf(1.0f - mod._left);
    mod._right = sqrtf(1.0f - mod._right);

    diff._right = (mod._right - oldgain._right) * invperiod;
    diff._left = (mod._left - oldgain._left) * invperiod;

    g = oldgain;
    oldgain = mod;

    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        g._left += diff._left; // Linear interpolation between LFO samples
        g._right += diff._right;

        Stereo<float> xn(input._left[i] * pangainL, input._right[i] * pangainR);

        if (barber)
        {
            g._left = fmodf((g._left + 0.25f), ONE_);
            g._right = fmodf((g._right + 0.25f), ONE_);
        }

        xn._left = applyPhase(xn._left, g._left, fb._left, hpf._left, yn1._left, xn1._left);
        xn._right = applyPhase(xn._right, g._right, fb._right, hpf._right, yn1._right, xn1._right);

        fb._left = xn._left * feedback;
        fb._right = xn._right * feedback;
        efxoutl[i] = xn._left;
        efxoutr[i] = xn._right;
    }

    if (Poutsub)
    {
        invSignal(efxoutl, this->_synth->buffersize);
        invSignal(efxoutr, this->_synth->buffersize);
    }
}

float Phaser::applyPhase(float x, float g, float fb,
                         float &hpf, float *yn1, float *xn1)
{
    for (int j = 0; j < Pstages; ++j)
    { //Phasing routine
        mis = 1.0f + offsetpct * offset[j];

        //This is symmetrical.
        //FET is not, so this deviates slightly, however sym dist. is
        //better sounding than a real FET.
        float d = (1.0f + 2.0f * (0.25f + g) * hpf * hpf * distortion) * mis;
        Rconst = 1.0f + mis * Rmx;

        // This is 1/R. R is being modulated to control filter fc.
        float b = (Rconst - g) / (d * Rmin);
        float gain = (CFs - b) / (CFs + b);
        yn1[j] = gain * (x + yn1[j]) - xn1[j];

        //high pass filter:
        //Distortion depends on the high-pass part of the AP stage.
        hpf = yn1[j] + (1.0f - gain) * xn1[j];

        xn1[j] = x;
        x = yn1[j];
        if (j == 1)
            x += fb; //Insert feedback after first phase stage
    }
    return x;
}
void Phaser::normalPhase(const Stereo<float *> &input)
{
    Stereo<float> gain(0.0f), lfoVal(0.0f);

    lfo.effectlfoout(&lfoVal._left, &lfoVal._right);
    gain._left = (expf(lfoVal._left * PHASER_LFO_SHAPE) - 1) / (expf(PHASER_LFO_SHAPE) - 1.0f);
    gain._right = (expf(lfoVal._right * PHASER_LFO_SHAPE) - 1) / (expf(PHASER_LFO_SHAPE) - 1.0f);

    gain._left = 1.0f - phase * (1.0f - depth) - (1.0f - phase) * gain._left * depth;
    gain._right = 1.0f - phase * (1.0f - depth) - (1.0f - phase) * gain._right * depth;

    gain._left = limit(gain._left, ZERO_, ONE_);
    gain._right = limit(gain._right, ZERO_, ONE_);

    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        float x = static_cast<float>(i) / this->_synth->buffersize_f;
        float x1 = 1.0f - x;

        //TODO think about making panning an external feature
        Stereo<float> xn(input._left[i] * pangainL + fb._left, input._right[i] * pangainR + fb._right);

        Stereo<float> g(gain._left * x + oldgain._left * x1, gain._right * x + oldgain._right * x1);

        xn._left = applyPhase(xn._left, g._left, old._left);
        xn._right = applyPhase(xn._right, g._right, old._right);

        //Left/Right crossing
        CrossOver(xn._left, xn._right, lrcross);

        fb._left = xn._left * feedback;
        fb._right = xn._right * feedback;
        efxoutl[i] = xn._left;
        efxoutr[i] = xn._right;
    }

    oldgain = gain;

    if (Poutsub)
    {
        invSignal(efxoutl, this->_synth->buffersize);
        invSignal(efxoutr, this->_synth->buffersize);
    }
}

float Phaser::applyPhase(float x, float g, float *old)
{
    for (int j = 0; j < Pstages * 2; ++j)
    { //Phasing routine
        float tmp = old[j];
        old[j] = g * tmp + x;
        x = tmp - g * old[j];
    }
    return x;
}

/*
 * Cleanup the effect
 */
void Phaser::Cleanup()
{
    fb = oldgain = Stereo<float>(0.0f);
    for (int i = 0; i < Pstages * 2; ++i)
    {
        old._left[i] = 0.0f;
        old._right[i] = 0.0f;
    }
    for (int i = 0; i < Pstages; ++i)
    {
        xn1._left[i] = 0.0f;
        yn1._left[i] = 0.0f;
        xn1._right[i] = 0.0f;
        yn1._right[i] = 0.0f;
    }
}

/*
 * Parameter control
 */
void Phaser::setwidth(unsigned char Pwidth)
{
    this->Pwidth = Pwidth;
    width = (static_cast<float>(Pwidth) / 127.0f);
}

void Phaser::setfb(unsigned char Pfb)
{
    this->Pfb = Pfb;
    feedback = static_cast<float>(Pfb - 64) / 64.2f;
}

void Phaser::setvolume(unsigned char Pvolume)
{
    this->Pvolume = Pvolume;
    outvolume = Pvolume / 127.0f;
    if (insertion == 0)
        volume = 1.0f;
    else
        volume = outvolume;
}

void Phaser::setdistortion(unsigned char Pdistortion)
{
    this->Pdistortion = Pdistortion;
    distortion = static_cast<float>(Pdistortion) / 127.0f;
}

void Phaser::setoffset(unsigned char Poffset)
{
    this->Poffset = Poffset;
    offsetpct = static_cast<float>(Poffset) / 127.0f;
}

void Phaser::setstages(unsigned char Pstages)
{
    delete[] old._left;
    delete[] xn1._left;
    delete[] yn1._left;
    delete[] old._right;
    delete[] xn1._right;
    delete[] yn1._right;

    this->Pstages = static_cast<unsigned char>(std::min(MAX_PHASER_STAGES, static_cast<int>(Pstages)));

    old = Stereo<float *>(new float[Pstages * 2],
                          new float[Pstages * 2]);

    xn1 = Stereo<float *>(new float[Pstages],
                          new float[Pstages]);

    yn1 = Stereo<float *>(new float[Pstages],
                          new float[Pstages]);

    Cleanup();
}

void Phaser::setphase(unsigned char Pphase)
{
    this->Pphase = Pphase;
    phase = (Pphase / 127.0f);
}

void Phaser::setdepth(unsigned char Pdepth)
{
    this->Pdepth = Pdepth;
    depth = static_cast<float>(Pdepth) / 127.0f;
}

void Phaser::SetPreset(unsigned char npreset)
{
    const int PRESET_SIZE = 15;
    const int NUM_PRESETS = 12;
    unsigned char presets[NUM_PRESETS][PRESET_SIZE] = {
        //Phaser
        //0   1    2    3  4   5     6   7   8    9 10   11 12  13 14
        {64, 64, 36, 0, 0, 64, 110, 64, 1, 0, 0, 20,
         0, 0,
         0},
        {64, 64, 35, 0, 0, 88, 40, 64, 3, 0, 0, 20, 0, 0,
         0},
        {64, 64, 31, 0, 0, 66, 68, 107, 2, 0, 0, 20, 0, 0,
         0},
        {39, 64, 22, 0, 0, 66, 67, 10, 5, 0, 1, 20, 0, 0,
         0},
        {64, 64, 20, 0, 1, 110, 67, 78, 10, 0, 0, 20, 0, 0,
         0},
        {64, 64, 53, 100, 0, 58, 37, 78, 3, 0, 0, 20, 0, 0,
         0},
        //APhaser
        //0   1    2   3   4   5     6   7   8    9 10   11 12  13 14
        {64, 64, 14, 0, 1, 64, 64, 40, 4, 10, 0, 110, 1, 20,
         1},
        {64, 64, 14, 5, 1, 64, 70, 40, 6, 10, 0, 110, 1, 20,
         1},
        {64, 64, 9, 0, 0, 64, 60, 40, 8, 10, 0, 40, 0, 20,
         1},
        {64, 64, 14, 10, 0, 64, 45, 80, 7, 10, 1, 110, 1, 20,
         1},
        {25, 64, 127, 10, 0, 64, 25, 16, 8, 100, 0, 25, 0, 20,
         1},
        {64, 64, 1, 10, 1, 64, 70, 40, 12, 10, 0, 110, 1, 20,
         1}};
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

void Phaser::ChangeParameter(int npar, unsigned char value)
{
    switch (npar)
    {
        case 0:
            setvolume(value);
            break;
        case 1:
            SetPanning(static_cast<char>(value));
            break;
        case 2:
            lfo.Pfreq = value;
            lfo.updateparams();
            break;
        case 3:
            lfo.Prandomness = value;
            lfo.updateparams();
            break;
        case 4:
            lfo.PLFOtype = value;
            lfo.updateparams();
            barber = (2 == value);
            break;
        case 5:
            lfo.Pstereo = value;
            lfo.updateparams();
            break;
        case 6:
            setdepth(value);
            break;
        case 7:
            setfb(value);
            break;
        case 8:
            setstages(value);
            break;
        case 9:
            SetLRCross(static_cast<char>(value));
            setoffset(value);
            break;
        case 10:
            Poutsub = static_cast<unsigned char>(std::min(static_cast<int>(value), 1));
            break;
        case 11:
            setphase(value);
            setwidth(value);
            break;
        case 12:
            Phyper = static_cast<unsigned char>(std::min(static_cast<int>(value), 1));
            break;
        case 13:
            setdistortion(value);
            break;
        case 14:
            Panalog = value;
            break;
    }
}

unsigned char Phaser::GetParameter(int npar) const
{
    switch (npar)
    {
        case 0:
            return Pvolume;
        case 1:
            return static_cast<unsigned char>(Ppanning);
        case 2:
            return lfo.Pfreq;
        case 3:
            return lfo.Prandomness;
        case 4:
            return lfo.PLFOtype;
        case 5:
            return lfo.Pstereo;
        case 6:
            return Pdepth;
        case 7:
            return Pfb;
        case 8:
            return Pstages;
        case 9:
            return static_cast<unsigned char>(Plrcross);
        case 10:
            return Poutsub;
        case 11:
            return static_cast<unsigned char>(Pphase);
        case 12:
            return Phyper;
        case 13:
            return Pdistortion;
        case 14:
            return Panalog;
        default:
            return 0;
    }
}
