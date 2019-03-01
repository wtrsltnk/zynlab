#include "TrackRegion.h"

TrackRegion::TrackRegion()
    : previewImage(0), repeat(0)
{}

TrackRegion::~TrackRegion()
{}

void TrackRegion::ClearAllNotes()
{
    for (int i = 0; i < NUM_MIDI_NOTES; i++)
    {
        eventsByNote[i].clear();
    }
}
