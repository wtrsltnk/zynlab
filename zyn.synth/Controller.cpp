/*
  ZynAddSubFX - a software synthesizer

  Controller.cpp - (Midi) Controllers implementation
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

#include "Controller.h"
#include <cmath>

Controller::Controller() {}

Controller::~Controller() = default;

void Controller::Init()
{
    Defaults();
    resetall();
}

void Controller::Defaults()
{
    setpitchwheelbendrange(200); //2 halftones
    expression.receive = 1;
    panning.depth = 64;
    filtercutoff.depth = 64;
    filterq.depth = 64;
    bandwidth.depth = 64;
    bandwidth.exponential = 0;
    modwheel.depth = 80;
    modwheel.exponential = 0;
    fmamp.receive = 1;
    volume.receive = 1;
    sustain.receive = 1;
    NRPN.receive = 1;

    portamento.portamento = 0;
    portamento.used = 0;
    portamento.proportional = 0;
    portamento.propRate = 80;
    portamento.propDepth = 90;
    portamento.receive = 1;
    portamento.time = 64;
    portamento.updowntimestretch = 64;
    portamento.pitchthresh = 3;
    portamento.pitchthreshtype = 1;
    portamento.noteusing = -1;
    resonancecenter.depth = 64;
    resonancebandwidth.depth = 64;

    initportamento(440.0f, 440.0f, false); // Now has a third argument
    setportamento(0);
}

void Controller::resetall()
{
    setpitchwheel(0); //center
    setexpression(127);
    setpanning(64);
    setfiltercutoff(64);
    setfilterq(64);
    setbandwidth(64);
    setmodwheel(64);
    setfmamp(127);
    setvolume(127);
    setsustain(0);
    setresonancecenter(64);
    setresonancebw(64);

    //reset the NRPN
    NRPN.parhi = -1;
    NRPN.parlo = -1;
    NRPN.valhi = -1;
    NRPN.vallo = -1;
}

void Controller::setpitchwheel(int value)
{
    pitchwheel.data = value;
    float cents = value / 8192.0f;
    cents *= pitchwheel.bendrange;
    pitchwheel.relfreq = powf(2, cents / 1200.0f);
    //fprintf(stderr,"%ld %ld -> %.3f\n",pitchwheel.bendrange,pitchwheel.data,pitchwheel.relfreq);fflush(stderr);
}

void Controller::setpitchwheelbendrange(unsigned short int value)
{
    pitchwheel.bendrange = value;
}

void Controller::setexpression(int value)
{
    expression.data = value;
    if (expression.receive != 0)
        expression.relvolume = value / 127.0f;
    else
        expression.relvolume = 1.0f;
}

void Controller::setpanning(int value)
{
    panning.data = value;
    panning.pan = (value / 128.0f - 0.5f) * (panning.depth / 64.0f);
}

void Controller::setfiltercutoff(int value)
{
    filtercutoff.data = value;
    filtercutoff.relfreq =
        (value - 64.0f) * filtercutoff.depth / 4096.0f * 3.321928f; //3.3219f..=ln2(10)
}

void Controller::setfilterq(int value)
{
    filterq.data = value;
    filterq.relq = powf(30.0f, (value - 64.0f) / 64.0f * (filterq.depth / 64.0f));
}

void Controller::setbandwidth(int value)
{
    bandwidth.data = value;
    if (bandwidth.exponential == 0)
    {
        float tmp = powf(25.0f, powf(bandwidth.depth / 127.0f, 1.5f)) - 1.0f;
        if ((value < 64) && (bandwidth.depth >= 64))
            tmp = 1.0f;
        bandwidth.relbw = (value / 64.0f - 1.0f) * tmp + 1.0f;
        if (bandwidth.relbw < 0.01f)
            bandwidth.relbw = 0.01f;
    }
    else
        bandwidth.relbw =
            powf(25.0f, (value - 64.0f) / 64.0f * (bandwidth.depth / 64.0f));
    ;
}

void Controller::setmodwheel(int value)
{
    modwheel.data = value;
    if (modwheel.exponential == 0)
    {
        float tmp =
            powf(25.0f, powf(modwheel.depth / 127.0f, 1.5f) * 2.0f) / 25.0f;
        if ((value < 64) && (modwheel.depth >= 64))
            tmp = 1.0f;
        modwheel.relmod = (value / 64.0f - 1.0f) * tmp + 1.0f;
        if (modwheel.relmod < 0.0f)
            modwheel.relmod = 0.0f;
    }
    else
        modwheel.relmod =
            powf(25.0f, (value - 64.0f) / 64.0f * (modwheel.depth / 80.0f));
}

void Controller::setfmamp(int value)
{
    fmamp.data = value;
    fmamp.relamp = value / 127.0f;
    if (fmamp.receive != 0)
        fmamp.relamp = value / 127.0f;
    else
        fmamp.relamp = 1.0f;
}

void Controller::setvolume(int value)
{
    volume.data = value;
    if (volume.receive != 0)
        volume.volume = powf(0.1f, (127 - value) / 127.0f * 2.0f);
    else
        volume.volume = 1.0f;
}

void Controller::setsustain(int value)
{
    sustain.data = value;
    if (sustain.receive != 0)
        sustain.sustain = ((value < 64) ? 0 : 1);
    else
        sustain.sustain = 0;
}

void Controller::setportamento(int value)
{
    portamento.data = value;
    if (portamento.receive != 0)
        portamento.portamento = ((value < 64) ? 0 : 1);
}

int Controller::initportamento(float oldfreq,
                               float newfreq,
                               bool legatoflag)
{
    portamento.x = 0.0f;

    if (legatoflag)
    { // Legato in progress
        if (portamento.portamento == 0)
            return 0;
    }
    else // No legato, do the original if...return
        if ((portamento.used != 0) || (portamento.portamento == 0))
        return 0;
    ;

    float portamentotime = powf(100.0f, portamento.time / 127.0f) / 50.0f; //portamento time in seconds

    if (portamento.proportional)
    {
        //If there is a min(float,float) and a max(float,float) then they
        //could be used here
        //Linear functors could also make this nicer
        if (oldfreq > newfreq) //2 is the center of propRate
            portamentotime *=
                powf(oldfreq / newfreq / (portamento.propRate / 127.0f * 3 + .05),
                     (portamento.propDepth / 127.0f * 1.6f + .2));
        else //1 is the center of propDepth
            portamentotime *=
                powf(newfreq / oldfreq / (portamento.propRate / 127.0f * 3 + .05),
                     (portamento.propDepth / 127.0f * 1.6f + .2));
    }

    if ((portamento.updowntimestretch >= 64) && (newfreq < oldfreq))
    {
        if (portamento.updowntimestretch == 127)
            return 0;
        portamentotime *= powf(0.1f,
                               (portamento.updowntimestretch - 64) / 63.0f);
    }
    if ((portamento.updowntimestretch < 64) && (newfreq > oldfreq))
    {
        if (portamento.updowntimestretch == 0)
            return 0;
        portamentotime *= powf(0.1f,
                               (64.0f - portamento.updowntimestretch) / 64.0f);
    }

    //printf("%f->%f : Time %f\n",oldfreq,newfreq,portamentotime);

    portamento.dx = SystemSettings::Instance().buffersize_f / (portamentotime * SystemSettings::Instance().samplerate_f);
    portamento.origfreqrap = oldfreq / newfreq;

    float tmprap = ((portamento.origfreqrap > 1.0f) ? (portamento.origfreqrap) : (1.0f / portamento.origfreqrap));

    float thresholdrap = powf(2.0f, portamento.pitchthresh / 12.0f);
    if ((portamento.pitchthreshtype == 0) && (tmprap - 0.00001f > thresholdrap))
        return 0;
    if ((portamento.pitchthreshtype == 1) && (tmprap + 0.00001f < thresholdrap))
        return 0;

    portamento.used = 1;
    portamento.freqrap = portamento.origfreqrap;
    return 1;
}

void Controller::updateportamento()
{
    if (portamento.used == 0)
        return;

    portamento.x += portamento.dx;
    if (portamento.x > 1.0f)
    {
        portamento.x = 1.0f;
        portamento.used = 0;
    }
    portamento.freqrap =
        (1.0f - portamento.x) * portamento.origfreqrap + portamento.x;
}

void Controller::setresonancecenter(int value)
{
    resonancecenter.data = value;
    resonancecenter.relcenter =
        powf(3.0f, (value - 64.0f) / 64.0f * (resonancecenter.depth / 64.0f));
}
void Controller::setresonancebw(int value)
{
    resonancebandwidth.data = value;
    resonancebandwidth.relbw =
        powf(1.5f, (value - 64.0f) / 64.0f * (resonancebandwidth.depth / 127.0f));
}

//Returns 0 if there is NRPN or 1 if there is not
int Controller::getnrpn(int *parhi, int *parlo, int *valhi, int *vallo)
{
    if (NRPN.receive == 0)
        return 1;
    if ((NRPN.parhi < 0) || (NRPN.parlo < 0) || (NRPN.valhi < 0) || (NRPN.vallo < 0))
        return 1;

    *parhi = NRPN.parhi;
    *parlo = NRPN.parlo;
    *valhi = NRPN.valhi;
    *vallo = NRPN.vallo;
    return 0;
}

void Controller::setparameternumber(unsigned int type, int value)
{
    switch (type)
    {
        case C_nrpnhi:
            NRPN.parhi = value;
            NRPN.valhi = -1;
            NRPN.vallo = -1; //clear the values
            break;
        case C_nrpnlo:
            NRPN.parlo = value;
            NRPN.valhi = -1;
            NRPN.vallo = -1; //clear the values
            break;
        case C_dataentryhi:
            if ((NRPN.parhi >= 0) && (NRPN.parlo >= 0))
                NRPN.valhi = value;
            break;
        case C_dataentrylo:
            if ((NRPN.parhi >= 0) && (NRPN.parlo >= 0))
                NRPN.vallo = value;
            break;
    }
}

void Controller::InitPresets()
{
    AddPreset("pitchwheel_bendrange", &pitchwheel.bendrange, -6400, 6400);

    AddPresetAsBool("expression_receive", &expression.receive);
    AddPreset("panning_depth", &panning.depth);
    AddPreset("filter_cutoff_depth", &filtercutoff.depth);
    AddPreset("filter_q_depth", &filterq.depth);
    AddPreset("bandwidth_depth", &bandwidth.depth);
    AddPreset("mod_wheel_depth", &modwheel.depth);
    AddPresetAsBool("mod_wheel_exponential", &modwheel.exponential);
    AddPresetAsBool("fm_amp_receive", &fmamp.receive);
    AddPresetAsBool("volume_receive", &volume.receive);
    AddPresetAsBool("sustain_receive", &sustain.receive);

    AddPresetAsBool("portamento_receive", &portamento.receive);
    AddPreset("portamento_time", &portamento.time);
    AddPreset("portamento_pitchthresh", &portamento.pitchthresh);
    AddPreset("portamento_pitchthreshtype", &portamento.pitchthreshtype);
    AddPreset("portamento_portamento", &portamento.portamento);
    AddPreset("portamento_updowntimestretch", &portamento.updowntimestretch);
    AddPreset("portamento_proportional", &portamento.proportional);
    AddPreset("portamento_proprate", &portamento.propRate);
    AddPreset("portamento_propdepth", &portamento.propDepth);

    AddPreset("resonance_center_depth", &resonancecenter.depth);
    AddPreset("resonance_bandwidth_depth", &resonancebandwidth.depth);
}
