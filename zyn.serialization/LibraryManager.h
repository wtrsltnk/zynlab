#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <set>
#include <string>
#include <zyn.common/ILibraryManager.h>

class Track;

class LibraryItem : public ILibraryItem
{
    std::string _name;
    std::string _path;
    std::set<std::string> _tags;

public:
    LibraryItem(std::string const &name, std::string const &path, std::set<std::string> const &tags);
    virtual ~LibraryItem();

    virtual std::string const &GetName();
    virtual std::string const &GetPath();
    virtual std::set<std::string> const &GetTags();
};

class LibraryManager : public ILibraryManager
{
    std::set<std::string> _libraryLocations;
    std::set<std::string> _instrumentTags;
    std::set<ILibraryItem *> _instruments;
    std::set<std::string> _sampleTags;
    std::set<ILibraryItem *> _samples;

    void scanLocation(std::string const &location, std::set<std::string> const &baseTags);

public:
    LibraryManager();
    LibraryManager(std::set<std::string> const &libraryLocations);
    virtual ~LibraryManager();

    virtual void RefreshLibrary();

    virtual void AddLibraryLocation(std::string const &location);
    virtual void RemoveLibraryLocation(std::string const &location);
    virtual std::set<std::string> const &GetLibraryLocations() const;

    virtual std::set<std::string> const &GetInstrumentTags() const;
    virtual std::set<ILibraryItem *> const &GetInstruments() const;
    virtual std::set<std::string> const &GetSampleTags() const;
    virtual std::set<ILibraryItem *> const &GetSamples() const;

    virtual bool LoadAsInstrument(ILibraryItem *item, Track *track);
};

#endif // ILIBRARYMANAGER_H
