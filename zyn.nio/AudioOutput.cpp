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

#include "AudioOutput.h"
#include "AudioOutputManager.h"
#include "SafeQueue.h"

#include <cstring>
#include <iostream>

using namespace std;

AudioOutput::AudioOutput(unsigned int sampleRate, unsigned int bufferSize)
    : _sampleRate(sampleRate), _bufferSize(bufferSize)
{}

AudioOutput::~AudioOutput() = default;

void AudioOutput::SetSamplerate(unsigned int sampleRate)
{
    _sampleRate = sampleRate;
}

unsigned int AudioOutput::SampleRate()
{
    return _sampleRate;
}

void AudioOutput::SetBufferSize(unsigned int bufferSize)
{
    _bufferSize = bufferSize;
}

unsigned int AudioOutput::BufferSize()
{
    return _bufferSize;
}

const Stereo<float *> AudioOutput::NextSample()
{
    return AudioOutputManager::getInstance()
        .NextSample(_bufferSize);
}

void AudioOutput::SetAudioEnabled(bool nval)
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
