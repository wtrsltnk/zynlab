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

#include "SaveToFileSerializer.h"
#include <system.io/system.io.directoryinfo.h>
#include <system.io/system.io.fileinfo.h>
#include <system.io/system.io.path.h>
#include <zyn.mixer/Track.h>

#define INSTRUMENT_EXTENSION ".xiz"
#define SAMPLE_EXTENSION ".wav"

LibraryManager::LibraryManager() {}

LibraryManager::LibraryManager(std::set<std::string> const &libraryLocations)
    : _libraryLocations(libraryLocations)
{}

LibraryManager::~LibraryManager() = default;

ILibrary *LibraryManager::scanLocation(std::string const &location, ILibrary *parent)
{
    auto dir = System::IO::DirectoryInfo(location);

    if (!dir.Exists())
    {
        return nullptr;
    }

    auto lib = new Library(dir.Name(), location, parent);

    auto items = dir.GetFiles();

    for (auto item : items)
    {
        if (item[0] == '.')
        {
            continue;
        }

        auto itemFile = System::IO::FileInfo(System::IO::Path::Combine(dir.FullName(), item));
        if (itemFile.Extension() == INSTRUMENT_EXTENSION)
        {
            auto instrument = new LibraryItem(itemFile.Name(), itemFile.FullName(), lib);
            _instruments.insert(instrument);
            lib->_items.insert(instrument);
        }

        if (itemFile.Extension() == SAMPLE_EXTENSION)
        {
            auto instrument = new LibraryItem(itemFile.Name(), itemFile.FullName(), lib);
            _samples.insert(instrument);
            lib->_items.insert(instrument);
        }
    }

    auto banks = dir.GetDirectories();

    for (auto bank : banks)
    {
        auto bankDir = System::IO::DirectoryInfo(System::IO::Path::Combine(dir.FullName(), bank));
        auto res = scanLocation(System::IO::Path::Combine(location, bank), lib);

        if (res != nullptr)
        {
            lib->GetChildren().insert(res);
        }
    }

    if (lib->GetChildren().empty() && lib->_items.empty())
    {
        delete lib;
        return nullptr;
    }

    return lib;
}

void LibraryManager::Cleanup()
{
    for (auto item : _instruments)
    {
        delete item;
    }
    _instruments.clear();
    for (auto item : _samples)
    {
        delete item;
    }
    _samples.clear();
    for (auto item : _topLevelLibraries)
    {
        delete item;
    }
    _topLevelLibraries.clear();
}

void LibraryManager::RefreshLibraries()
{
    Cleanup();

    for (auto location : _libraryLocations)
    {
        auto lib = scanLocation(location, nullptr);

        if (lib != nullptr)
        {
            _topLevelLibraries.insert(lib);
        }
    }
}

bool LibraryManager::LoadAsInstrument(ILibraryItem *item, Track *track)
{
    if (_instruments.find(item) == _instruments.end())
    {
        return false;
    }

    track->AllNotesOff();
    track->InstrumentDefaults();

    return SaveToFileSerializer().LoadTrack(track, item->GetPath()) == 0;
}

void LibraryManager::AddLibraryLocation(std::string const &location)
{
    _libraryLocations.insert(location);
}

void LibraryManager::RemoveLibraryLocation(std::string const &location)
{
    _libraryLocations.erase(location);
}

std::set<std::string> const &LibraryManager::GetLibraryLocations() const
{
    return _libraryLocations;
}

std::set<ILibrary *> const &LibraryManager::GetTopLevelLibraries() const
{
    return _topLevelLibraries;
}

std::set<ILibraryItem *> const &LibraryManager::GetInstruments() const
{
    return _instruments;
}

std::set<ILibraryItem *> const &LibraryManager::GetSamples() const
{
    return _samples;
}
