/*
  ZynAddSubFX - a software synthesizer

  RtEngine.h - Midi input through RtMidi for Windows
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

#include "RtEngine.h"
#include "EngineManager.h"
#include "MidiInputManager.h"

#include <iostream>

RtEngine::RtEngine()
    : midiin(nullptr)
{
    this->_name = "RT";
}

RtEngine::~RtEngine()
{
    this->Stop();
}

bool RtEngine::Start()
{
    std::cout << "Starting RtEngine...";

    try
    {
        this->midiin = new RtMidiIn(RtMidi::WINDOWS_MM, "zynlab");
        this->midiin->setCallback(RtEngine::callback, this);
        this->midiin->openPort(1);

        std::cout << "succeeded." << std::endl;

        return true;
    }
    catch (const std::exception &ex)
    {
        delete this->midiin;
        this->midiin = nullptr;

        std::cout << "failed." << std::endl;
        return false;
    }
}

void RtEngine::Stop()
{
    delete this->midiin;
    this->midiin = nullptr;
}

bool RtEngine::IsMidiEnabled() const
{
    return (this->midiin != nullptr);
}

void RtEngine::callback(double /*timeStamp*/, std::vector<unsigned char> *message, void * /*userData*/)
{
    MidiEvent ev;
    unsigned char chan = message->at(0) & 0x0f;
    switch (message->at(0) & 0xf0)
    {
        case 0x80: //Note Off
        {
            ev.type = MidiEventTypes::M_NOTE;
            ev.channel = chan;
            ev.num = message->at(1);
            ev.value = 0;
            break;
        }
        case 0x90: //Note On
        {
            ev.type = MidiEventTypes::M_NOTE;
            ev.channel = chan;
            ev.num = message->at(1);
            ev.value = message->at(2);
            break;
        }
        case 0xA0: /* pressure, aftertouch */
        {
            ev.type = MidiEventTypes::M_PRESSURE;
            ev.channel = chan;
            ev.num = message->at(1);
            ev.value = message->at(2);
            break;
        }
        case 0xb0: //Controller
        {
            ev.type = MidiEventTypes::M_CONTROLLER;
            ev.channel = chan;
            ev.num = message->at(1);
            ev.value = message->at(2);
            break;
        }
        case 0xe0: //Pitch Wheel
        {
            ev.type = MidiEventTypes::M_CONTROLLER;
            ev.channel = chan;
            ev.num = C_pitchwheel;
            ev.value = (message->at(1) + message->at(2) * 128) - 8192;
            break;
        }
        case 0xc0: //Program Change
        {
            ev.type = MidiEventTypes::M_PGMCHANGE;
            ev.channel = chan;
            ev.num = message->at(1);
            ev.value = 0;
            break;
        }
        default:
        {
            return;
        }
    }

    MidiInputManager::Instance().PutEvent(ev);
}
