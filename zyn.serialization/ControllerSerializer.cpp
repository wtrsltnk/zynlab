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

#include "ControllerSerializer.h"
#include <cmath>

ControllerSerializer::ControllerSerializer(
    Controller *parameters)
    : _parameters(parameters)
{}

ControllerSerializer::~ControllerSerializer() = default;

void ControllerSerializer::Serialize(
    IPresetsSerializer *xml)
{
    xml->addpar("pitchwheel_bendrange", _parameters->pitchwheel.bendrange);

    xml->addparbool("expression_receive", _parameters->expression.receive);
    xml->addpar("panning_depth", _parameters->panning.depth);
    xml->addpar("filter_cutoff_depth", _parameters->filtercutoff.depth);
    xml->addpar("filter_q_depth", _parameters->filterq.depth);
    xml->addpar("bandwidth_depth", _parameters->bandwidth.depth);
    xml->addpar("mod_wheel_depth", _parameters->modwheel.depth);
    xml->addparbool("mod_wheel_exponential", _parameters->modwheel.exponential);
    xml->addparbool("fm_amp_receive", _parameters->fmamp.receive);
    xml->addparbool("volume_receive", _parameters->volume.receive);
    xml->addparbool("sustain_receive", _parameters->sustain.receive);

    xml->addparbool("portamento_receive", _parameters->portamento.receive);
    xml->addpar("portamento_time", _parameters->portamento.time);
    xml->addpar("portamento_pitchthresh", _parameters->portamento.pitchthresh);
    xml->addpar("portamento_pitchthreshtype", _parameters->portamento.pitchthreshtype);
    xml->addpar("portamento_portamento", _parameters->portamento.portamento);
    xml->addpar("portamento_updowntimestretch", _parameters->portamento.updowntimestretch);
    xml->addpar("portamento_proportional", _parameters->portamento.proportional);
    xml->addpar("portamento_proprate", _parameters->portamento.propRate);
    xml->addpar("portamento_propdepth", _parameters->portamento.propDepth);

    xml->addpar("resonance_center_depth", _parameters->resonancecenter.depth);
    xml->addpar("resonance_bandwidth_depth", _parameters->resonancebandwidth.depth);
}

void ControllerSerializer::Deserialize(
    IPresetsSerializer *xml)
{
    _parameters->pitchwheel.bendrange = xml->getpar("pitchwheel_bendrange", _parameters->pitchwheel.bendrange, -6400, 6400);

    _parameters->expression.receive = xml->getparbool("expression_receive", _parameters->expression.receive);
    _parameters->panning.depth = xml->getpar127("panning_depth", _parameters->panning.depth);
    _parameters->filtercutoff.depth = xml->getpar127("filter_cutoff_depth", _parameters->filtercutoff.depth);
    _parameters->filterq.depth = xml->getpar127("filter_q_depth", _parameters->filterq.depth);
    _parameters->bandwidth.depth = xml->getpar127("bandwidth_depth", _parameters->bandwidth.depth);
    _parameters->modwheel.depth = xml->getpar127("mod_wheel_depth", _parameters->modwheel.depth);
    _parameters->modwheel.exponential = xml->getparbool("mod_wheel_exponential", _parameters->modwheel.exponential);
    _parameters->fmamp.receive = xml->getparbool("fm_amp_receive", _parameters->fmamp.receive);
    _parameters->volume.receive = xml->getparbool("volume_receive", _parameters->volume.receive);
    _parameters->sustain.receive = xml->getparbool("sustain_receive", _parameters->sustain.receive);

    _parameters->portamento.receive = xml->getparbool("portamento_receive", _parameters->portamento.receive);
    _parameters->portamento.time = xml->getpar127("portamento_time", _parameters->portamento.time);
    _parameters->portamento.pitchthresh = xml->getpar127("portamento_pitchthresh", _parameters->portamento.pitchthresh);
    _parameters->portamento.pitchthreshtype = xml->getpar127("portamento_pitchthreshtype", _parameters->portamento.pitchthreshtype);
    _parameters->portamento.portamento = xml->getpar127("portamento_portamento", _parameters->portamento.portamento);
    _parameters->portamento.updowntimestretch = xml->getpar127("portamento_updowntimestretch", _parameters->portamento.updowntimestretch);
    _parameters->portamento.proportional = xml->getpar127("portamento_proportional", _parameters->portamento.proportional);
    _parameters->portamento.propRate = xml->getpar127("portamento_proprate", _parameters->portamento.propRate);
    _parameters->portamento.propDepth = xml->getpar127("portamento_propdepth", _parameters->portamento.propDepth);

    _parameters->resonancecenter.depth = xml->getpar127("resonance_center_depth", _parameters->resonancecenter.depth);
    _parameters->resonancebandwidth.depth = xml->getpar127("resonance_bandwidth_depth", _parameters->resonancebandwidth.depth);
}
