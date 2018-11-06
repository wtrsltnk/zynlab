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

#ifndef AUDIOOUTPUT_H
#define AUDIOOUTPUT_H

#include "Engine.h"
#include <zyn.common/Stereo.h>
#include <zyn.common/globals.h>

class AudioOutput : public virtual Engine
{
public:
    AudioOutput(unsigned int sampleRate, unsigned int _bufferSize);
    virtual ~AudioOutput();

    /**Sets the Sample Rate of this Output
         * (used for getNext()).*/
    void SetSamplerate(unsigned int _samplerate);

    /**Gets the Samples required per Out of this driver
         * not a realtime opperation */
    unsigned int SampleRate() const;

    /**Sets the Frame Size for output*/
    void SetBufferSize(unsigned int _bufferSize);

    /**Gets the Frame Size for output*/
    unsigned int BufferSize() const;

    virtual void SetAudioEnabled(bool nval);
    virtual bool IsAudioEnabled() const = 0;

protected:
    /**Get the next sample for output.
         * (has nsamples sampled at a rate of samplerate)*/
    const Stereo<float *> NextSample();

    unsigned int _sampleRate;
    unsigned int _bufferSize;
};

#endif // AUDIOOUTPUT_H
