#ifndef ILIBRARYMANAGER_H
#define ILIBRARYMANAGER_H

#include <set>
#include <string>

class Track;

class ILibraryItem
{
public:
    virtual ~ILibraryItem();

    virtual std::string const &GetName() = 0;
    virtual std::string const &GetPath() = 0;
    virtual std::set<std::string> const &GetTags() = 0;
};

class ILibraryManager
{
public:
    virtual ~ILibraryManager();

    virtual void RefreshLibrary() = 0;

    virtual void AddLibraryLocation(std::string const &location) = 0;
    virtual void RemoveLibraryLocation(std::string const &location) = 0;
    virtual std::set<std::string> const &GetLibraryLocations() const = 0;

    virtual std::set<std::string> const &GetInstrumentTags() const = 0;
    virtual std::set<ILibraryItem *> const &GetInstruments() const = 0;
    virtual std::set<std::string> const &GetSampleTags() const = 0;
    virtual std::set<ILibraryItem *> const &GetSamples() const = 0;

    virtual bool LoadAsInstrument(ILibraryItem *item, Track *track) = 0;
};

#endif // ILIBRARYMANAGER_H
