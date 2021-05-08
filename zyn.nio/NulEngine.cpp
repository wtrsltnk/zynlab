/*
  ZynAddSubFX - a software synthesizer

  OSSaudiooutput.C - Audio output for Open Sound System
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

#include "NulEngine.h"

#include <iostream>
#include <unistd.h>
#include <zyn.common/globals.h>

NulEngine::NulEngine(
    unsigned int sampleRate,
    unsigned int bufferSize)
    : AudioOutput(sampleRate, bufferSize)
{
    _name = "NULL";
    playing_until.tv_sec = 0;
    playing_until.tv_usec = 0;
}

void *NulEngine::_AudioThread(void *arg)
{
    return (static_cast<NulEngine *>(arg))->AudioThread();
}

void *NulEngine::AudioThread()
{
    while (_thread)
    {
        NextSample();

        struct timeval now;
        int remaining = 0;
        gettimeofday(&now, nullptr);
        if ((playing_until.tv_usec == 0) && (playing_until.tv_sec == 0))
        {
            playing_until.tv_usec = now.tv_usec;
            playing_until.tv_sec = now.tv_sec;
        }
        else
        {
            remaining = (playing_until.tv_usec - now.tv_usec) + (playing_until.tv_sec - now.tv_sec) * 1000000;
            if (remaining > 10000) //Don't sleep() less than 10ms.
                //This will add latency...
            {
                usleep(remaining - 10000);
            }
            if (remaining < 0)
            {
                std::cerr << "WARNING - too late" << std::endl;
            }
        }
        playing_until.tv_usec += this->BufferSize() * 1000000 / this->SampleRate();
        if (remaining < 0)
        {
            playing_until.tv_usec -= remaining;
        }
        playing_until.tv_sec += playing_until.tv_usec / 1000000;
        playing_until.tv_usec %= 1000000;
    }

    return nullptr;
}

NulEngine::~NulEngine() = default;

bool NulEngine::Start()
{
    SetAudioEnabled(true);
    return IsAudioEnabled();
}

void NulEngine::Stop()
{
    SetAudioEnabled(false);
}

void NulEngine::SetAudioEnabled(bool nval)
{
    if (nval)
    {
        if (!IsAudioEnabled())
        {
            _thread = new std::thread(_AudioThread, this);
        }
    }
    else if (IsAudioEnabled())
    {
        auto thread = _thread;

        _thread = nullptr;

        thread->join();

        delete thread;
    }
}

bool NulEngine::IsAudioEnabled() const
{
    return _thread;
}
