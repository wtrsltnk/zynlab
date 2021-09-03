/*
  ZynAddSubFX - a software synthesizer

  globals.h - it contains program settings and the program capabilities
              like number of parts, of effects
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

#include "globals.h"

SimpleNote::SimpleNote() : note(0), velocity(0), lengthInSec(0), channel(0) {}

SimpleNote::SimpleNote(
    unsigned int n,
    unsigned int v,
    float l,
    unsigned int c)
    : note(n),
      velocity(v),
      lengthInSec(l),
      channel(c)
{}

SimpleNote::~SimpleNote() = default;

INoteSource::~INoteSource() = default;

IMixer::~IMixer() = default;

IBankManager::~IBankManager() = default;

bool IBankManager::InstrumentBank::operator<(const InstrumentBank &b) const
{
    return name < b.name;
}
