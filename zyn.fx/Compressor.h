/*
  ZynAddSubFX - a software synthesizer

  Compressor.h - Compressor effect

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

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "Effect.h"

/**Compressor Effect: a feed-forward compressor with a soft knee, working in dB*/
class Compressor : public Effect
{
public:
    Compressor(bool insertion, float *efxoutl_, float *efxoutr_);
    virtual ~Compressor();

    void out(const Stereo<float *> &smp);
    void SetPreset(unsigned char npreset);
    void ChangeParameter(int npar, unsigned char value);
    unsigned char GetParameter(int npar) const;
    void Cleanup(void);
    float GetGainReduction(void);

    static float ThresholdDb(unsigned char value);
    static float Ratio(unsigned char value);
    static float AttackMs(unsigned char value);
    static float ReleaseMs(unsigned char value);
    static float KneeDb(unsigned char value);
    static float MakeUpDb(unsigned char value);

private:
    //Parameters
    unsigned char Pvolume;    //Volume or E/R
    unsigned char Pthreshold; //-60..0 dB
    unsigned char Pratio;     //1:1..20:1, 127 is infinite
    unsigned char Pattack;    //0.1..100 ms
    unsigned char Prelease;   //10..2000 ms
    unsigned char Pknee;      //0..24 dB
    unsigned char Pmakeup;    //0..30 dB
    unsigned char Pstereolink; //0 = each channel on its own, 127 = both on the louder one

    void setvolume(unsigned char _Pvolume);
    float gainReductionDb(float level) const;

    //Real Parameters
    float threshold, slope, knee, makeup, link;
    float attackcoeff, releasecoeff;

    //State: the smoothed gain reduction per channel, in dB
    float reductionl, reductionr;
    float maxreduction; //the largest gain reduction of the last buffer, for a meter
};

#endif
