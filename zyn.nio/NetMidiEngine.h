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
#ifndef NETMIDIENGINE_H
#define NETMIDIENGINE_H

#include "MidiInput.h"
#include <thread>
#include <zyn.net/HttpServer.h>

class NetMidiEngine : public MidiInput
{
public:
    NetMidiEngine();
    virtual ~NetMidiEngine();

    virtual bool IsMidiIn() { return true; }
    virtual bool IsAudioOut() { return false; }

    virtual bool Start();
    virtual void Stop();

    virtual bool IsMidiEnabled() const;

    static int onRecieveRequest(const net::Request &request, net::Response &response);

private:
    net::HttpServer _server;
    std::thread _thread;
    bool _isRunning;
};

#endif // NETMIDIENGINE_H
