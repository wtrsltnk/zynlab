/*
  ZynAddSubFX - a software synthesizer

  Util.cpp - Miscellaneous functions
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

#include "Util.h"
#include <cassert>
#include <cmath>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <vector>

#ifdef HAVE_SCHEDULER
#include <sched.h>
#endif

prng_t prng_state = 0x1234;

/*
 * Transform the velocity according the scaling parameter (velocity sensing)
 */
float VelF(float velocity, unsigned char scaling)
{
    float x;
    x = powf(VELOCITY_MAX_SCALE, (64.0f - scaling) / 64.0f);
    if ((scaling == 127) || (velocity > 0.99f))
        return 1.0f;

    return powf(velocity, x);
}

/*
 * Get the detune in cents
 */
float getdetune(unsigned char type,
                unsigned short int coarsedetune,
                unsigned short int finedetune)
{
    float det = 0.0f, octdet = 0.0f, cdet = 0.0f, findet = 0.0f;
    //Get Octave
    int octave = coarsedetune / 1024;
    if (octave >= 8)
        octave -= 16;
    octdet = octave * 1200.0f;

    //Coarse and fine detune
    int cdetune = coarsedetune % 1024;
    if (cdetune > 512)
        cdetune -= 1024;

    int fdetune = finedetune - 8192;

    switch (type)
    {
            //	case 1: is used for the default (see below)
        case 2:
            cdet = std::fabs(cdetune * 10.0f);
            findet = std::fabs(fdetune / 8192.0f) * 10.0f;
            break;
        case 3:
            cdet = static_cast<int>(std::fabs(cdetune * 100));
            findet = powf(10, std::fabs(fdetune / 8192.0f) * 3.0f) / 10.0f - 0.1f;
            break;
        case 4:
            cdet = std::fabs(cdetune * 701.95500087f); //perfect fifth
            findet =
                (powf(2, std::fabs(fdetune / 8192.0f) * 12.0f) - 1.0f) / 4095 * 1200;
            break;
        //case ...: need to update N_DETUNE_TYPES, if you'll add more
        default:
            cdet = std::fabs(cdetune * 50.0f);
            findet = std::fabs(fdetune / 8192.0f) * 35.0f; //almost like "Paul's Sound Designer 2"
            break;
    }
    if (finedetune < 8192)
        findet = -findet;
    if (cdetune < 0)
        cdet = -cdet;

    det = octdet + cdet + findet;
    return det;
}

bool fileexists(const char *filename)
{
    struct stat tmp;
    int result = stat(filename, &tmp);
    return (result >= 0);
}

std::string legalizeFilename(std::string filename)
{
    for (char &i : filename)
    {
        char c = i;
        if (!(isdigit(c) || isalpha(c) || (c == '-') || (c == ' ')))
            i = '_';
    }
    return filename;
}

void invSignal(float *sig, size_t len)
{
    for (size_t i = 0; i < len; ++i)
        sig[i] *= -1.0f;
}

float SystemSettings::numRandom()
{
    return RND;
}

float interpolate(const float *data, size_t len, float pos)
{
    assert(len > static_cast<size_t>(pos + 1));
    const int l_pos = static_cast<int>(pos), r_pos = l_pos + 1;
    const float leftness = pos - l_pos;
    return data[l_pos] * leftness + data[r_pos] * (1.0f - leftness);
}

float cinterpolate(const float *data, size_t len, float pos)
{
    const unsigned int l_pos = static_cast<unsigned int>(pos) % len, r_pos = (l_pos + 1) % len;
    const float leftness = pos - l_pos;
    return data[l_pos] * leftness + data[r_pos] * (1.0f - leftness);
}

SystemSettings::SystemSettings()
    : samplerate(44100), buffersize(256), oscilsize(1024)
{
    alias();
    denormalkillbuf = new float[this->buffersize];
    for (unsigned int i = 0; i < this->buffersize; ++i)
    {
        denormalkillbuf[i] = (RND - 0.5f) * 1e-16f;
    }
}

SystemSettings& SystemSettings::Instance()
{
    static SystemSettings instance;

    return instance;
}

SystemSettings::~SystemSettings()
{
    delete[] this->denormalkillbuf;
}
