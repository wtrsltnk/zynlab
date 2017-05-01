/*
  ZynAddSubFX - a software synthesizer

  RtEngine.cpp - Midi input through RtMidi for Windows
  Copyright (C) 2014 Wouter Saaltink
  Author: Wouter Saaltink

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
#ifndef RTENGINE_H
#define RTENGINE_H

#include "EngineManager.h"
#include "MidiInput.h"
#include "RtMidi.h"

class RtEngine : public MidiInput
{
public:
    RtEngine();
    virtual ~RtEngine();

    virtual bool IsMidiIn() { return true; }
    virtual bool IsAudioOut() { return false; }

    virtual bool Start();
    virtual void Stop();

    virtual void setMidiEn(bool nval);
    virtual bool getMidiEn() const;

    static void callback(double timeStamp, std::vector<unsigned char> *message, void *userData);
private:
    RtMidiIn* midiin;
};

#endif // RTENGINE_H
