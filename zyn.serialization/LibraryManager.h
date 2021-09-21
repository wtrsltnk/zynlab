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

#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <set>
#include <string>
#include <zyn.common/ILibraryManager.h>

class Track;

class Library : public ILibrary
{
    std::string _name;
    std::string _path;
    ILibrary *_parent;
    std::set<ILibrary *> _children;

public:
    Library(std::string const &name, std::string const &path, ILibrary *parent);
    virtual ~Library();

    std::set<ILibraryItem *> _items;
    virtual std::string const &GetName() const;
    virtual std::string const &GetPath() const;
    virtual ILibrary *GetParent();
    virtual std::set<ILibrary *> &GetChildren();
    virtual bool IsParent(ILibrary *library);
    virtual void AddLibraryItem(class ILibraryItem *item);
    virtual std::set<class ILibraryItem *> &GetItems();
};

class LibraryItem : public ILibraryItem
{
    std::string _name;
    std::string _path;
    std::set<std::string> _tags;
    ILibrary *_library;

public:
    LibraryItem(std::string const &name, std::string const &path, ILibrary *parent);
    virtual ~LibraryItem();

    virtual std::string const &GetName() const;
    virtual std::string const &GetPath() const;
    virtual ILibrary *GetLibrary();
};

class LibraryManager : public ILibraryManager
{
    std::set<std::string> _libraryLocations;
    std::set<ILibrary *> _topLevelLibraries;
    std::set<ILibraryItem *> _instruments;
    std::set<ILibraryItem *> _samples;

    ILibrary *scanLocation(std::string const &location, ILibrary *library);

public:
    LibraryManager();
    LibraryManager(std::set<std::string> const &libraryLocations);
    virtual ~LibraryManager();

    void Cleanup();
    virtual void RefreshLibraries();

    virtual void AddLibraryLocation(std::string const &location);
    virtual void RemoveLibraryLocation(std::string const &location);
    virtual std::set<std::string> const &GetLibraryLocations() const;

    virtual std::set<ILibrary *> const &GetTopLevelLibraries() const;
    virtual std::set<ILibraryItem *> const &GetInstruments() const;
    virtual std::set<ILibraryItem *> const &GetSamples() const;

    virtual bool LoadAsInstrument(ILibraryItem *item, Track *track);
};

#endif // ILIBRARYMANAGER_H
