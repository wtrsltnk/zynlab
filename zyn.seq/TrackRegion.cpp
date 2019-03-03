#include "TrackRegion.h"

TrackRegion::TrackRegion()
    : previewImage(0), repeat(0)
{}

TrackRegion::~TrackRegion()
{}

bool operator<(struct TrackRegionEvent const &a, struct TrackRegionEvent const &b)
{
    if (a.note < b.note) return true;
    if (a.note == b.note)
    {
        if (a.values[0] < b.values[0]) return true;
        if (a.values[0] == b.values[0])
        {
            if (a.values[1] < b.values[1]) return true;
            if (a.values[1] == b.values[1])
            {
                return a.velocity < b.velocity;
            }
        }
    }

    return false;
}
