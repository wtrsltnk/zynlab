/*
  ZynAddSubFX - a software synthesizer

  AudioOut.h - Audio Output superclass
  Copyright (C) 2009-2010 Mark McCurry
  Author: Mark McCurry

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

#include "AudioInput.h"
#include "SafeQueue.h"

#include <cstring>
#include <iostream>

using namespace std;

AudioInput::AudioInput(
    unsigned int sampleRate,
    unsigned int bufferSize)
    : _sampleRate(sampleRate),
      _bufferSize(bufferSize)
{}

AudioInput::~AudioInput() = default;

unsigned int AudioInput::SampleRate() const
{
    return _sampleRate;
}

unsigned int AudioInput::BufferSize() const
{
    return _bufferSize;
}

void AudioInput::SetAudioEnabled(
    bool nval)
{
    if (nval)
    {
        Start();
    }
    else
    {
        Stop();
    }
}
