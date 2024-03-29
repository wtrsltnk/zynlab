/*
  ZynAddSubFX - a software synthesizer

  Effect.cpp - this class is inherited by the all effects(Reverb, Echo, ..)
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Copyright 2011, Alan Calvert
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

#include "Effect.h"
#include <zyn.dsp/FilterParams.h>

#include <cmath>

Effect::Effect(bool insertion_, float *efxoutl_, float *efxoutr_,
               FilterParams *filterpars_, unsigned char Ppreset_)
    : Ppreset(Ppreset_),
      efxoutl(efxoutl_),
      efxoutr(efxoutr_),
      filterpars(filterpars_),
      insertion(insertion_)
{}

Effect::~Effect() = default;

void Effect::CrossOver(float &a, float &b, float crossover)
{
    float tmpa = a;
    float tmpb = b;
    a = tmpa * (1.0f - crossover) + tmpb * crossover;
    b = tmpb * (1.0f - crossover) + tmpa * crossover;
}

void Effect::SetPanning(char Ppanning_)
{
    Ppanning = Ppanning_;
    float t = (Ppanning > 0) ? static_cast<float>(Ppanning - 1) / 126.0f : 0.0f;
    pangainL = cosf(t * PI / 2.0f);
    pangainR = cosf((1.0f - t) * PI / 2.0f);
}

void Effect::SetLRCross(char Plrcross_)
{
    Plrcross = Plrcross_;
    lrcross = static_cast<float>(Plrcross) / 127.0f;
}
