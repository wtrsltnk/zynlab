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

#ifndef AUDIOINPUT_H
#define AUDIOINPUT_H

#include "Engine.h"
#include <zyn.common/Stereo.h>
#include <zyn.common/globals.h>

class AudioInput : public virtual Engine
{
public:
    AudioInput(
        unsigned int sampleRate,
        unsigned int _bufferSize);

    virtual ~AudioInput();

    unsigned int SampleRate() const;

    unsigned int BufferSize() const;

    virtual void SetAudioEnabled(
        bool nval);

    virtual bool IsAudioEnabled() const = 0;

protected:
    unsigned int _sampleRate;
    unsigned int _bufferSize;
};

#endif // AUDIOINPUT_H
