/*
  ZynAddSubFX - a software synthesizer

  Compressor.cpp - Compressor effect

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

#include "Compressor.h"

#include "EffectPresets.h"
#include <cmath>

Compressor::Compressor(bool insertion_, float *efxoutl_, float *efxoutr_)
    : Effect(insertion_, efxoutl_, efxoutr_, nullptr, 0),
      Pvolume(127),
      Pthreshold(0),
      Pratio(0),
      Pattack(0),
      Prelease(0),
      Pknee(0),
      Pmakeup(0),
      Pstereolink(127)
{
    SetPreset(Ppreset);
    Cleanup();
}

Compressor::~Compressor() = default;

//Cleanup the effect
void Compressor::Cleanup()
{
    reductionl = 0.0f;
    reductionr = 0.0f;
    maxreduction = 0.0f;
}

//The gain reduction of the last buffer, in dB (0 or more)
float Compressor::GetGainReduction()
{
    return maxreduction;
}

//The parameter values in their units
float Compressor::ThresholdDb(unsigned char value)
{
    return -60.0f + 60.0f * value / 127.0f;
}

float Compressor::Ratio(unsigned char value)
{
    return value >= 127 ? INFINITY : powf(20.0f, value / 126.0f);
}

float Compressor::AttackMs(unsigned char value)
{
    return 0.1f * powf(1000.0f, value / 127.0f);
}

float Compressor::ReleaseMs(unsigned char value)
{
    return 10.0f * powf(200.0f, value / 127.0f);
}

float Compressor::KneeDb(unsigned char value)
{
    return 24.0f * value / 127.0f;
}

float Compressor::MakeUpDb(unsigned char value)
{
    return 30.0f * value / 127.0f;
}

//How many dB a level (in dB) has to come down: the static curve, with a soft knee around the threshold
float Compressor::gainReductionDb(float level) const
{
    float over = level - threshold;
    if (2.0f * over <= -knee)
    {
        return 0.0f;
    }
    if (2.0f * over < knee)
    {
        float t = over + knee / 2.0f;
        return slope * t * t / (2.0f * knee);
    }
    return slope * over;
}

//Effect output
void Compressor::out(const Stereo<float *> &smp)
{
    //the time constants follow the sample rate, which may have been set after the parameters
    float samplerate = SystemSettings::Instance().samplerate_f;
    attackcoeff = expf(-1000.0f / (AttackMs(Pattack) * samplerate));
    releasecoeff = expf(-1000.0f / (ReleaseMs(Prelease) * samplerate));

    maxreduction = 0.0f;
    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        float l = fabsf(smp._left[i]);
        float r = fabsf(smp._right[i]);
        float louder = l > r ? l : r;
        l += link * (louder - l);
        r += link * (louder - r);

        float targetl = gainReductionDb(rap2dB(l + 1e-9f));
        float targetr = gainReductionDb(rap2dB(r + 1e-9f));

        //a rising reduction follows the attack, a falling one the release
        float coeffl = targetl > reductionl ? attackcoeff : releasecoeff;
        float coeffr = targetr > reductionr ? attackcoeff : releasecoeff;
        reductionl = targetl + coeffl * (reductionl - targetl);
        reductionr = targetr + coeffr * (reductionr - targetr);
        if (reductionl < 1e-6f)
        {
            reductionl = 0.0f;
        }
        if (reductionr < 1e-6f)
        {
            reductionr = 0.0f;
        }

        efxoutl[i] = smp._left[i] * dB2rap(makeup - reductionl);
        efxoutr[i] = smp._right[i] * dB2rap(makeup - reductionr);

        float reduction = reductionl > reductionr ? reductionl : reductionr;
        if (reduction > maxreduction)
        {
            maxreduction = reduction;
        }
    }
}

//Parameter control
void Compressor::setvolume(unsigned char _Pvolume)
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

void Compressor::SetPreset(unsigned char npreset)
{
    const int PRESET_SIZE = 9;
    const int NUM_PRESETS = 6;
    unsigned char presets[NUM_PRESETS][PRESET_SIZE] = {
        //Gentle: -18 dB, 2:1, 15 ms, 150 ms, knee 6 dB, +3 dB
        {127, 64, 89, 29, 92, 65, 32, 13, 127},
        //Vocal: -24 dB, 4:1, 5 ms, 80 ms, knee 6 dB, +6 dB
        {127, 64, 76, 58, 72, 50, 32, 25, 127},
        //Drums: -20 dB, 6:1, 20 ms (lets the attack through), 60 ms, hard knee, +5 dB
        {127, 64, 85, 75, 97, 43, 0, 21, 127},
        //Bass: -22 dB, 4:1, 10 ms, 200 ms, knee 12 dB, +5 dB
        {127, 64, 80, 58, 85, 72, 64, 21, 127},
        //Mix bus: -12 dB, 2:1, 30 ms, 300 ms, knee 6 dB, +2 dB
        {127, 64, 102, 29, 105, 82, 32, 8, 127},
        //Parallel: half dry, half squashed: -40 dB, 10:1, 1 ms, 100 ms, +12 dB
        {64, 64, 42, 97, 42, 55, 0, 51, 127}};

    if (npreset >= NUM_PRESETS)
    {
        npreset = NUM_PRESETS - 1;
    }
    for (int n = 0; n < PRESET_SIZE; ++n)
    {
        ChangeParameter(n, presets[npreset][n]);
    }
    Ppreset = npreset;
    Cleanup();
}

void Compressor::ChangeParameter(int npar, unsigned char value)
{
    switch (npar)
    {
        case EffectPresets::Volume:
            setvolume(value);
            break;
        case EffectPresets::Panning:
            SetPanning(value); //kept for the parameter list, a compressor doesn't pan
            break;
        case CompressorPresets::CompressorThreshold:
            Pthreshold = value;
            threshold = ThresholdDb(value);
            break;
        case CompressorPresets::CompressorRatio:
            Pratio = value;
            slope = 1.0f - 1.0f / Ratio(value);
            break;
        case CompressorPresets::CompressorAttack:
            Pattack = value;
            break;
        case CompressorPresets::CompressorRelease:
            Prelease = value;
            break;
        case CompressorPresets::CompressorKnee:
            Pknee = value;
            knee = KneeDb(value);
            break;
        case CompressorPresets::CompressorMakeUp:
            Pmakeup = value;
            makeup = MakeUpDb(value);
            break;
        case CompressorPresets::CompressorStereoLink:
            Pstereolink = value;
            link = value / 127.0f;
            break;
    }
}

unsigned char Compressor::GetParameter(int npar) const
{
    switch (npar)
    {
        case EffectPresets::Volume:
            return Pvolume;
        case EffectPresets::Panning:
            return Ppanning;
        case CompressorPresets::CompressorThreshold:
            return Pthreshold;
        case CompressorPresets::CompressorRatio:
            return Pratio;
        case CompressorPresets::CompressorAttack:
            return Pattack;
        case CompressorPresets::CompressorRelease:
            return Prelease;
        case CompressorPresets::CompressorKnee:
            return Pknee;
        case CompressorPresets::CompressorMakeUp:
            return Pmakeup;
        case CompressorPresets::CompressorStereoLink:
            return Pstereolink;
        default:
            return 0; //in case of bogus parameter number
    }
}
