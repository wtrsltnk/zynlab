/*
  ZynAddSubFX - a software synthesizer

  Limiter.cpp - Limiter effect

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

#include "Limiter.h"

#include "EffectPresets.h"
#include <cmath>

//The peak follower aims this far under the ceiling; the soft clipper uses the room in between
static const float ClipRoomDb = 1.0f;

Limiter::Limiter(bool insertion_, float *efxoutl_, float *efxoutr_)
    : Effect(insertion_, efxoutl_, efxoutr_, nullptr, 0),
      Pvolume(127),
      Pdrive(0),
      Pceiling(127),
      Pattack(0),
      Prelease(0),
      Pstereolink(127)
{
    SetPreset(Ppreset);
    Cleanup();
}

Limiter::~Limiter() = default;

//Cleanup the effect
void Limiter::Cleanup()
{
    reductionl = 0.0f;
    reductionr = 0.0f;
    maxreduction = 0.0f;
}

//The gain reduction of the last buffer, in dB (0 or more); the drive is not counted
float Limiter::GetGainReduction()
{
    return maxreduction;
}

//The parameter values in their units
float Limiter::DriveDb(unsigned char value)
{
    return 24.0f * value / 127.0f;
}

float Limiter::CeilingDb(unsigned char value)
{
    return -12.0f + 12.0f * value / 127.0f;
}

float Limiter::AttackMs(unsigned char value)
{
    return 0.05f * powf(100.0f, value / 127.0f);
}

float Limiter::ReleaseMs(unsigned char value)
{
    return 10.0f * powf(100.0f, value / 127.0f);
}

//Straight up to ClipRoomDb under the ceiling, above that bending smoothly towards the ceiling, never past it
float Limiter::softclip(float x) const
{
    float knee = ceiling * dB2rap(-ClipRoomDb);
    float a = fabsf(x);
    if (a <= knee)
    {
        return x;
    }
    float room = ceiling - knee;
    float y = knee + room * tanhf((a - knee) / room);
    return x < 0.0f ? -y : y;
}

//Effect output
void Limiter::out(const Stereo<float *> &smp)
{
    //the time constants follow the sample rate, which may have been set after the parameters
    float samplerate = SystemSettings::Instance().samplerate_f;
    attackcoeff = expf(-1000.0f / (AttackMs(Pattack) * samplerate));
    releasecoeff = expf(-1000.0f / (ReleaseMs(Prelease) * samplerate));

    float targetdb = rap2dB(ceiling) - ClipRoomDb;

    maxreduction = 0.0f;
    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        float inl = smp._left[i] * drive;
        float inr = smp._right[i] * drive;

        float l = fabsf(inl);
        float r = fabsf(inr);
        float louder = l > r ? l : r;
        l += link * (louder - l);
        r += link * (louder - r);

        //how far the peak is over the target, in dB
        float targetl = rap2dB(l + 1e-9f) - targetdb;
        float targetr = rap2dB(r + 1e-9f) - targetdb;
        if (targetl < 0.0f)
        {
            targetl = 0.0f;
        }
        if (targetr < 0.0f)
        {
            targetr = 0.0f;
        }

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

        efxoutl[i] = softclip(inl * dB2rap(-reductionl));
        efxoutr[i] = softclip(inr * dB2rap(-reductionr));

        float reduction = reductionl > reductionr ? reductionl : reductionr;
        if (reduction > maxreduction)
        {
            maxreduction = reduction;
        }
    }
}

//Parameter control
void Limiter::setvolume(unsigned char _Pvolume)
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

void Limiter::SetPreset(unsigned char npreset)
{
    const int PRESET_SIZE = 7;
    const int NUM_PRESETS = 4;
    unsigned char presets[NUM_PRESETS][PRESET_SIZE] = {
        //Safe: only catches the peaks: +0 dB, ceiling -1 dB, 1 ms, 100 ms
        {127, 64, 0, 116, 83, 64, 127},
        //Gentle: +3 dB, ceiling -1 dB, 1 ms, 300 ms
        {127, 64, 16, 116, 83, 94, 127},
        //Loud: +6 dB, ceiling -1 dB, 0.5 ms, 60 ms
        {127, 64, 32, 116, 64, 49, 127},
        //Very loud: +10 dB, ceiling -1 dB, 0.2 ms, 30 ms
        {127, 64, 53, 116, 38, 30, 127}};

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

void Limiter::ChangeParameter(int npar, unsigned char value)
{
    switch (npar)
    {
        case EffectPresets::Volume:
            setvolume(value);
            break;
        case EffectPresets::Panning:
            SetPanning(value); //kept for the parameter list, a limiter doesn't pan
            break;
        case LimiterPresets::LimiterDrive:
            Pdrive = value;
            drive = dB2rap(DriveDb(value));
            break;
        case LimiterPresets::LimiterCeiling:
            Pceiling = value;
            ceiling = dB2rap(CeilingDb(value));
            break;
        case LimiterPresets::LimiterAttack:
            Pattack = value;
            break;
        case LimiterPresets::LimiterRelease:
            Prelease = value;
            break;
        case LimiterPresets::LimiterStereoLink:
            Pstereolink = value;
            link = value / 127.0f;
            break;
    }
}

unsigned char Limiter::GetParameter(int npar) const
{
    switch (npar)
    {
        case EffectPresets::Volume:
            return Pvolume;
        case EffectPresets::Panning:
            return Ppanning;
        case LimiterPresets::LimiterDrive:
            return Pdrive;
        case LimiterPresets::LimiterCeiling:
            return Pceiling;
        case LimiterPresets::LimiterAttack:
            return Pattack;
        case LimiterPresets::LimiterRelease:
            return Prelease;
        case LimiterPresets::LimiterStereoLink:
            return Pstereolink;
        default:
            return 0; //in case of bogus parameter number
    }
}
