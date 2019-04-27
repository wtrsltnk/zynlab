/*
  ZynAddSubFX - a software synthesizer

  LFOParams.h - Parameters for LFO
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

#include "LibraryManager.h"

LibraryItem::LibraryItem(std::string const &name, std::string const &path, ILibrary *library)
    : _name(name), _path(path), _library(library)
{}

LibraryItem::~LibraryItem() = default;

std::string const &LibraryItem::GetName()
{
    return _name;
}

std::string const &LibraryItem::GetPath()
{
    return _path;
}

ILibrary *LibraryItem::GetLibrary()
{
    return _library;
}