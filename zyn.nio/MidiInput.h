/*
  ZynAddSubFX - a software synthesizer

  MidiIn.h - This class is inherited by all the Midi input classes
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Copyright (C) 2009-2010 Mark McCurry
  Author: Nasca Octavian Paula
          Mark McCurry

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

#ifndef MIDIINPUT_H
#define MIDIINPUT_H

#include "Engine.h"

/**This class is inherited by all the Midi input classes*/
class MidiInput : public virtual Engine
{
public:
    virtual ~MidiInput();

    /**Enables or disables driver based upon value*/
    virtual void SetMidiEnabled(
        bool nval);

    /**Returns if driver is initialized*/
    virtual bool IsMidiEnabled() const = 0;

    static void ProcessMidiEvent(
        unsigned char head,
        unsigned char num,
        unsigned char value);
};

#endif // MIDIINPUT_H
