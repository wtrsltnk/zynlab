/*
  ZynAddSubFX - a software synthesizer

  Limiter.h - Limiter effect

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

#ifndef LIMITER_H
#define LIMITER_H

#include "Effect.h"

/**Limiter Effect: keeps the peaks under a ceiling without look-ahead (so without delay):
 * a fast peak follower turns the sound down, a soft clipper catches what slips through its attack*/
class Limiter : public Effect
{
public:
    Limiter(bool insertion, float *efxoutl_, float *efxoutr_);
    virtual ~Limiter();

    void out(const Stereo<float *> &smp);
    void SetPreset(unsigned char npreset);
    void ChangeParameter(int npar, unsigned char value);
    unsigned char GetParameter(int npar) const;
    void Cleanup(void);
    float GetGainReduction(void);

    static float DriveDb(unsigned char value);
    static float CeilingDb(unsigned char value);
    static float AttackMs(unsigned char value);
    static float ReleaseMs(unsigned char value);

private:
    //Parameters
    unsigned char Pvolume;     //Volume or E/R
    unsigned char Pdrive;      //0..24 dB
    unsigned char Pceiling;    //-12..0 dB
    unsigned char Pattack;     //0.05..5 ms
    unsigned char Prelease;    //10..1000 ms
    unsigned char Pstereolink; //0 = each channel on its own, 127 = both on the louder one

    void setvolume(unsigned char _Pvolume);
    float softclip(float x) const;

    //Real Parameters
    float drive, ceiling, link;
    float attackcoeff, releasecoeff;

    //State: the smoothed gain reduction per channel, in dB
    float reductionl, reductionr;
    float maxreduction; //the largest gain reduction of the last buffer, for a meter
};

#endif
