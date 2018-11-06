#include "Track.h"

Track::Track()
    : _patternsStart(0)
{}

Track::~Track() = default;

PatternEvent *Track::GetPatternEventByStep(unsigned int step)
{
    if (_patterns.size() == 0)
    {
        return nullptr;
    }

    auto eventIndex = step % NUM_PATTERN_EVENTS;
    auto patternIndex = step - eventIndex;

    if (patternIndex < _patternsStart)
    {
        return nullptr;
    }

    if (patternIndex > (_patternsStart + _patterns.size()))
    {
        return nullptr;
    }

    auto &event = _patterns[patternIndex - _patternsStart]._events[eventIndex];

    return &event;
}
