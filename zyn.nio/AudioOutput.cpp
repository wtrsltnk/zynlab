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

#include <iostream>
#include <cstring>

using namespace std;

AudioOutput::AudioOutput(SystemSettings* s) : _synth(s), samplerate(s->samplerate), bufferSize(s->buffersize) { }

AudioOutput::~AudioOutput() { }

void AudioOutput::setSamplerate(int _samplerate)
{
    samplerate = _samplerate;
}

int AudioOutput::getSampleRate()
{
    return samplerate;
}

void AudioOutput::setBufferSize(int _bufferSize)
{
    bufferSize = _bufferSize;
}

const Stereo<float *> AudioOutput::getNext()
{
    return AudioOutputManager::getInstance().tick(bufferSize);
}
