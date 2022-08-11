/*
  ZynAddSubFX - a software synthesizer

  NetMidiEngine.h - Midi input through RtMidi for Windows
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
#include "NetMidiEngine.h"
#include "EngineManager.h"
#include "MidiInputManager.h"

#include <iostream>
#include <regex>
#include <string>
#include <zyn.net/HttpRequest.h>
#include <zyn.net/HttpResponse.h>

NetMidiEngine::NetMidiEngine()
    : _isRunning(false)
{
    this->_name = "NET";
    _server.Init();
}

NetMidiEngine::~NetMidiEngine()
{
    this->Stop();
}

bool NetMidiEngine::Start()
{
    _isRunning = true;
    if (!_server.Start())
    {
        return false;
    }

    _thread = std::thread([this]() {
        while (_isRunning)
        {
            this->_server.WaitForRequests(this->onRecieveRequest);
        }
    });

    return true;
}

void NetMidiEngine::Stop()
{
    _isRunning = false;
    _server.Stop();
}

bool NetMidiEngine::IsMidiEnabled() const
{
    return _server.IsStarted();
}

std::map<std::string, std::string> ParseFormData(
    const std::string &query)
{
    std::map<std::string, std::string> data;
    std::regex pattern("([\\w+%]+)=([^&]*)");
    auto words_begin = std::sregex_iterator(query.begin(), query.end(), pattern);
    auto words_end = std::sregex_iterator();

    for (std::sregex_iterator i = words_begin; i != words_end; i++)
    {
        std::string key = (*i)[1].str();
        std::string value = (*i)[2].str();
        data[key] = value;
    }

    return data;
}

int NetMidiEngine::onRecieveRequest(
    const net::Request &request,
    net::Response &response)
{
    auto formData = ParseFormData(request._payload);

    MidiEvent ev;

    if (request._uri.find("/note-on") == 0)
    {
        int channel = std::stoi(formData["channel"]);
        int note = std::stoi(formData["note"]);
        int velocity = std::stoi(formData["velocity"]);

        ev.type = MidiEventTypes::M_NOTE;
        ev.channel = channel;
        ev.num = note;
        ev.value = velocity;
        MidiInputManager::Instance().PutEvent(ev);

        response._response = "note-on";
    }
    else if (request._uri.find("/note-off") == 0)
    {
        int channel = std::stoi(formData["channel"]);
        int note = std::stoi(formData["note"]);

        ev.type = MidiEventTypes::M_NOTE;
        ev.channel = channel;
        ev.num = note;
        ev.value = 0;
        MidiInputManager::Instance().PutEvent(ev);

        response._response = "note-off";
    }

    response._response =
        "<html>"
        "<body>"
        "<form method=\"post\">"
        "<label><input type=\"text\" name=\"channel\" value=\"0\"/>channel</label>"
        "<label><input type=\"text\" name=\"note\" value=\"65\"/>note</label>"
        "<label><input type=\"text\" name=\"velocity\" value=\"127\"/>velcity</label>"
        "<button type=\"submit\" formaction=\"/note-on\">on</button>"
        "<button type=\"submit\" formaction=\"/note-off\">off</button>"
        "</form>"
        "</body>"
        "</html>";

    return 200;
}
