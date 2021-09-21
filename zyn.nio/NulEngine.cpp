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
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif

#include <zyn.common/globals.h>

NulEngine::NulEngine(
    unsigned int sampleRate,
    unsigned int bufferSize)
    : AudioOutput(sampleRate, bufferSize)
{
    _name = "NULL";
}

void *NulEngine::_AudioThread(
    void *arg)
{
    return (static_cast<NulEngine *>(arg))->AudioThread();
}

void *NulEngine::AudioThread()
{
    while (_thread)
    {
        NextSample();

        auto now = std::chrono::system_clock::now();
        auto remaining = std::chrono::milliseconds();

        if (playing_until.time_since_epoch().count() == 0)
        {
            playing_until = now;
        }
        else
        {
            remaining = std::chrono::duration_cast<std::chrono::milliseconds>(playing_until - now);
            if (remaining.count() > 10000) //Don't sleep() less than 10ms.
                //This will add latency...
            {
#ifndef _WIN32
                usleep(remaining.count() - 10000);
#else
                Sleep(remaining.count() - 10000);
#endif
            }

            if (remaining.count() < 0)
            {
                std::cerr << "WARNING - too late" << std::endl;
            }
        }

        playing_until += std::chrono::milliseconds(this->BufferSize() * 1000 / this->SampleRate());
        if (remaining.count() < 0)
        {
            playing_until -= std::chrono::milliseconds(remaining);
        }
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

void NulEngine::SetAudioEnabled(
    bool nval)
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
