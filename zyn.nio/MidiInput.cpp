/*
  ZynAddSubFX - a software synthesizer

  MidiIn.C - This class is inherited by all the Midi input classes
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

#include "MidiInput.h"
#include "MidiInputManager.h"
#include <zyn.common/globals.h>

MidiInput::~MidiInput() = default;

void MidiInput::SetMidiEnabled(
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

void MidiInput::ProcessMidiEvent(
    unsigned char head,
    unsigned char num,
    unsigned char value)
{
    MidiEvent ev;
    unsigned char chan = head & 0x0f;
    switch (head & 0xf0)
    {
        case 0x80: //Note Off
            ev.type = MidiEventTypes::M_NOTE;
            ev.channel = chan;
            ev.num = num;
            ev.value = 0;
            MidiInputManager::Instance().PutEvent(ev);
            break;
        case 0x90: //Note On
            ev.type = MidiEventTypes::M_NOTE;
            ev.channel = chan;
            ev.num = num;
            ev.value = value;
            MidiInputManager::Instance().PutEvent(ev);
            break;
        case 0xA0: /* pressure, aftertouch */
            ev.type = MidiEventTypes::M_PRESSURE;
            ev.channel = chan;
            ev.num = num;
            ev.value = value;
            MidiInputManager::Instance().PutEvent(ev);
            break;
        case 0xb0: //Controller
            ev.type = MidiEventTypes::M_CONTROLLER;
            ev.channel = chan;
            ev.num = num;
            ev.value = value;
            MidiInputManager::Instance().PutEvent(ev);
            break;
        case 0xc0: //Program Change
            ev.type = MidiEventTypes::M_PGMCHANGE;
            ev.channel = chan;
            ev.num = num;
            ev.value = 0;
            MidiInputManager::Instance().PutEvent(ev);
            break;
        case 0xe0: //Pitch Wheel
            ev.type = MidiEventTypes::M_CONTROLLER;
            ev.channel = chan;
            ev.num = C_pitchwheel;
            ev.value = (num + value * 128) - 8192;
            MidiInputManager::Instance().PutEvent(ev);
            break;
    }
}
