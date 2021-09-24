/*
  ZynAddSubFX - a software synthesizer

  globals.h - it contains program settings and the program capabilities
              like number of parts, of effects
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

#include "globals.h"
#include <iostream>

SimpleNote::SimpleNote() : note(0), velocity(0), lengthInSec(0), channel(0) {}

SimpleNote::SimpleNote(
    unsigned int n,
    unsigned int v,
    float l,
    unsigned int c)
    : note(n),
      velocity(v),
      lengthInSec(l),
      channel(c)
{}

SimpleNote::~SimpleNote() = default;

INoteSource::~INoteSource() = default;

IMixer::~IMixer() = default;

IBankManager::~IBankManager() = default;

bool IBankManager::InstrumentBank::operator<(const InstrumentBank &b) const
{
    return name < b.name;
}

bool SystemSettings::SetSampleRate(
    unsigned int rate)
{
    samplerate = rate;
    if (samplerate < 4000)
    {
        std::cerr << "ERROR:Incorrect sample rate: " << samplerate
                  << std::endl;

        return false;
    }

    return true;
}

bool SystemSettings::SetBufferSize(
    unsigned int size)
{
    buffersize = size;

    if (buffersize < 2)
    {
        return false;
    }

    return true;
}

void SystemSettings::SetOscilSize(
    unsigned int size)
{
    unsigned int tmp = 0;
    oscilsize = tmp = size;
    if (oscilsize < MAX_AD_HARMONICS * 2)
    {
        oscilsize = MAX_AD_HARMONICS * 2;
    }

    oscilsize = static_cast<unsigned int>(powf(2, ceil(logf(oscilsize - 1.0f) / logf(2.0f))));

    if (tmp != oscilsize)
    {
        std::cerr << "synth.oscilsize is wrong (must be 2^n) or too small. Adjusting to "
                  << oscilsize << "." << std::endl;
    }
}
