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
#include <iostream>

using namespace std;

SdlEngine::SdlEngine()
    : _dev(0)
{
    name = "SDL2";
}


SdlEngine::~SdlEngine()
{
    Stop();
}

void SdlEngine::my_audio_callback(void *userdata, Uint8 *stream, int len)
{
    SdlEngine* thiz = static_cast<SdlEngine*>(userdata);
    thiz->process(stream, len);
}

bool SdlEngine::Start()
{
    if(getAudioEn()) return true;

    SDL_Init(SDL_INIT_AUDIO);

    SDL_AudioSpec want, have;

    SDL_memset(&want, 0, sizeof(want)); /* or SDL_zero(want) */
    want.freq = 48000;
    want.format = AUDIO_F32;
    want.channels = 2;
    want.samples = synth->buffersize;
    want.size = 0;
    want.userdata = (void*)this;
    want.callback = my_audio_callback; /* you wrote this function elsewhere -- see SDL_AudioSpec for details */

    this->_dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

//    this->setBufferSize(have.size);

    SDL_PauseAudioDevice(this->_dev, 0);

    return true;
}

void SdlEngine::Stop()
{
    if(!getAudioEn()) return;

    SDL_PauseAudioDevice(this->_dev, 1);
    SDL_CloseAudio();
    std::cout << SDL_GetAudioStatus() << std::endl;
    this->_dev = 0;
}

void SdlEngine::setAudioEn(bool nval)
{
    if(nval)
        Start();
    else
        Stop();
}

bool SdlEngine::getAudioEn() const
{
    return this->_dev;
}

int SdlEngine::process(Uint8 *stream, int len)
{
    const Stereo<float *> smp = getNext();
    Uint8 *l = (Uint8*)(smp.l);
    Uint8 *r = (Uint8*)(smp.r);
    for(unsigned i = 0; i < len; ++i) {
        *stream++ = smp.l[i];
        *stream++ = smp.r[i];
    }

    return 0;
}
