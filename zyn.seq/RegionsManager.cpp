#include "RegionsManager.h"

#include <algorithm>
#include <vector>

RegionsManager::RegionsManager()
{}

bool RegionsManager::DoesRegionExist(int trackIndex, int regionIndex) const
{
    if (trackIndex < 0 || trackIndex >= NUM_MIXER_TRACKS)
    {
        return false;
    }

    if (regionIndex < 0 || static_cast<size_t>(regionIndex) >= regionsByTrack[trackIndex].size())
    {
        return false;
    }

    return true;
}

std::vector<TrackRegion> &RegionsManager::GetRegionsByTrack(int trackIndex)
{
    return regionsByTrack[trackIndex];
}

TrackRegion &RegionsManager::GetRegion(int trackIndex, int regionIndex)
{
    if (!DoesRegionExist(trackIndex, regionIndex))
    {
        static TrackRegion emptyRegion;

        return emptyRegion;
    }

    return regionsByTrack[trackIndex][static_cast<size_t>(regionIndex)];
}

void RegionsManager::AddRegion(int trackIndex, TrackRegion const &region)
{
    if (trackIndex < 0 || trackIndex >= NUM_MIXER_TRACKS)
    {
        return;
    }

    GetRegionsByTrack(trackIndex).push_back(region);
}

void RegionsManager::AddRegion(int trackIndex, timestep start, timestep end)
{
    TrackRegion region;

    region.startAndEnd[0] = start;
    region.startAndEnd[1] = end;

    AddRegion(trackIndex, region);
}

void RegionsManager::RemoveRegion(int trackIndex, int regionIndex)
{
    if (!DoesRegionExist(trackIndex, regionIndex))
    {
        return;
    }

    auto &regions = regionsByTrack[trackIndex];

    regions.erase(regions.begin() + regionIndex);
}

std::vector<struct TrackRegionEvent>::iterator indexOf(std::vector<struct TrackRegionEvent> &vec, struct TrackRegionEvent const &event)
{
    auto i = vec.begin();
    while (i != vec.end())
    {
        if (!(event < *i) && !(*i < event))
        {
            return i;
        }
        ++i;
    }

    return vec.end();
}

void RegionsManager::RemoveRegionEvent(int trackIndex, int regionIndex, struct TrackRegionEvent const &event)
{
    auto &region = GetRegion(trackIndex, regionIndex);

    auto &notes = region.eventsByNote[event.note];

    auto found = indexOf(notes, event);

    if (found != notes.end())
    {
        notes.erase(found);
    }
}

void RegionsManager::ClearAllNotesInRegion(int trackIndex, int regionIndex)
{
    if (!DoesRegionExist(trackIndex, regionIndex))
    {
        return;
    }

    auto &region = GetRegion(trackIndex, regionIndex);

    ClearAllNotesInRegion(region);
}

void RegionsManager::ClearAllNotesInRegion(TrackRegion &region)
{
    for (int i = 0; i < NUM_MIDI_NOTES; i++)
    {
        region.eventsByNote[i].clear();
    }
}
