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
#include <zyn.common/globals.h>

#include <iostream>
#include <unistd.h>

using namespace std;

NulEngine::NulEngine(SystemSettings *s)
    : AudioOutput(s), pThread(nullptr)
{
    name = "NULL";
    playing_until.tv_sec = 0;
    playing_until.tv_usec = 0;
}

void *NulEngine::_AudioThread(void *arg)
{
    return (static_cast<NulEngine *>(arg))->AudioThread();
}

void *NulEngine::AudioThread()
{
    while (pThread)
    {
        nextSample();

        struct timeval now{};
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
                usleep(remaining - 10000);
            if (remaining < 0)
                cerr << "WARNING - too late" << endl;
        }
        playing_until.tv_usec += this->_synth->buffersize * 1000000 / this->_synth->samplerate;
        if (remaining < 0)
            playing_until.tv_usec -= remaining;
        playing_until.tv_sec += playing_until.tv_usec / 1000000;
        playing_until.tv_usec %= 1000000;
    }
    return nullptr;
}

NulEngine::~NulEngine() = default;

bool NulEngine::Start()
{
    setAudioEn(true);
    return getAudioEn();
}

void NulEngine::Stop()
{
    setAudioEn(false);
}

void NulEngine::setAudioEn(bool nval)
{
    if (nval)
    {
        if (!getAudioEn())
        {
            auto *thread = new pthread_t;
            pthread_attr_t attr;
            pthread_attr_init(&attr);
            pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
            pThread = thread;
            pthread_create(pThread, &attr, _AudioThread, this);
        }
    }
    else if (getAudioEn())
    {
        pthread_t *thread = pThread;
        pThread = nullptr;
        pthread_join(*thread, nullptr);
        delete thread;
    }
}

bool NulEngine::getAudioEn() const
{
    return pThread;
}
