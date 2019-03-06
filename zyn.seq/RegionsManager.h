#ifndef REGIONSMANAGER_H
#define REGIONSMANAGER_H

#include "TrackRegion.h"

class RegionsManager
{
    std::vector<TrackRegion> regionsByTrack[NUM_MIXER_TRACKS];

public:
    RegionsManager();

    bool DoesRegionExist(int trackIndex, int regionIndex) const;
    std::vector<TrackRegion> &GetRegionsByTrack(int trackIndex);
    TrackRegion &GetRegion(int trackIndex, int regionIndex);

    void AddRegion(int trackIndex, TrackRegion const &region);
    void AddRegion(int trackIndex, timestep start, timestep end);
    void RemoveRegion(int trackIndex, int regionIndex);
    void RemoveRegionEvent(int trackIndex, int regionIndex, struct TrackRegionEvent const &event);
    void ClearAllNotesInRegion(int trackIndex, int regionIndex);
    void ClearAllNotesInRegion(TrackRegion &region);
    void ClearAllButSelectedInRegion(TrackRegion &region, struct TrackRegionEvent event);
    void ClearSelectedInRegion(TrackRegion &region, struct TrackRegionEvent const &event);
};

#endif // REGIONSMANAGER_H
