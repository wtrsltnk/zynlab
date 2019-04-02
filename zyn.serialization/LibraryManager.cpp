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

void LibraryManager::scanLocation(std::string const &location, std::set<std::string> const &baseTags)
{
    auto dir = System::IO::DirectoryInfo(location);

    if (!dir.Exists())
    {
        return;
    }

    auto banks = dir.GetDirectories();

    for (auto bank : banks)
    {
        auto bankDir = System::IO::DirectoryInfo(System::IO::Path::Combine(dir.FullName(), bank));
        auto items = bankDir.GetFiles();
        auto tags = std::set<std::string>({bankDir.Name()});
        tags.insert(baseTags.begin(), baseTags.end());

        for (auto item : items)
        {
            if (item[0] == '.')
            {
                continue;
            }

            auto itemFile = System::IO::FileInfo(System::IO::Path::Combine(bankDir.FullName(), item));
            if (itemFile.Extension() == INSTRUMENT_EXTENSION)
            {
                _instrumentTags.insert(bankDir.Name());

                auto instrument = new LibraryItem(itemFile.Name(), itemFile.FullName(), tags);
                _instruments.insert(instrument);
            }

            if (itemFile.Extension() == SAMPLE_EXTENSION)
            {
                _sampleTags.insert(bankDir.Name());

                auto instrument = new LibraryItem(itemFile.Name(), itemFile.FullName(), tags);
                _samples.insert(instrument);
            }
        }
        
        scanLocation(System::IO::Path::Combine(location, bank), tags);
    }
}

void LibraryManager::RefreshLibrary()
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
    _instrumentTags.clear();
    _sampleTags.clear();

  std::set<std::string> tags;
  
    for (auto location : _libraryLocations)
    {
        scanLocation(location, tags);
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

std::set<std::string> const &LibraryManager::GetInstrumentTags() const
{
    return _instrumentTags;
}

std::set<ILibraryItem *> const &LibraryManager::GetInstruments() const
{
    return _instruments;
}

std::set<std::string> const &LibraryManager::GetSampleTags() const
{
    return _sampleTags;
}

std::set<ILibraryItem *> const &LibraryManager::GetSamples() const
{
    return _samples;
}
