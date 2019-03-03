#ifndef TRACKREGION_H
#define TRACKREGION_H

#include <chrono>
#include <vector>
#include <zyn.common/globals.h>

typedef std::chrono::milliseconds::rep timestep;

struct TrackRegionEvent
{
    timestep values[2];
    unsigned char note;
    unsigned char velocity;
};

bool operator < (TrackRegionEvent const& a, TrackRegionEvent const &b);

class TrackRegion
{
public:
    TrackRegion();
    virtual ~TrackRegion();

    unsigned int previewImage;
    timestep startAndEnd[2];
    std::vector<struct TrackRegionEvent> eventsByNote[NUM_MIDI_NOTES];
    int repeat;
};

#endif // TRACKREGION_H
