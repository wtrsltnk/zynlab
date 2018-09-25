/*
  ZynAddSubFX - a software synthesizer

  SdlEngine.cpp - Audio output for PortAudio
  Copyright (C) 2002 Nasca Octavian Paul
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

#include "SdlEngine.h"

#include <cmath>
#include <iostream>

using namespace std;

SdlEngine::SdlEngine(SystemSettings *s)
    : AudioOutput(s), _dev(0)
{
    _name = "SDL2";
}

SdlEngine::~SdlEngine()
{
    Stop();
}

void SdlEngine::my_audio_callback(void *userdata, Uint8 *stream, int len)
{
    static_cast<SdlEngine *>(userdata)->process((float *)stream, len / 4);
}

const double freq = 440;
const int FREQUENCY = 44100;

bool SdlEngine::Start()
{
    if (IsAudioEnabled())
    {
        return true;
    }

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec desiredSpec;

    desiredSpec.freq = 44100;
    desiredSpec.format = AUDIO_F32;
    desiredSpec.channels = 2;
    desiredSpec.samples = 256;
    desiredSpec.callback = my_audio_callback;
    desiredSpec.userdata = this;

    SDL_AudioSpec obtainedSpec;

//    // you might want to look for errors here
//    SDL_OpenAudio(&desiredSpec, &obtainedSpec);

//    // start play audio
//    SDL_PauseAudio(0);

    this->_dev = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);

    SDL_PauseAudioDevice(this->_dev, 0);

    return true;
}

void SdlEngine::Stop()
{
    if (!IsAudioEnabled())
    {
        return;
    }

//    SDL_PauseAudio(1);
    SDL_PauseAudioDevice(this->_dev, 1);
    SDL_CloseAudio();
    this->_dev = 0;
}

bool SdlEngine::IsAudioEnabled() const
{
    return this->_dev;
}

static double v = 0;
void SdlEngine::process(float *stream, int len)
{
    const Stereo<float *> smp = NextSample();

    int i = 0;
    while (i < len)
    {
        stream[i++] = *(smp._left);
        stream[i++] = *(smp._right);
    }
}
