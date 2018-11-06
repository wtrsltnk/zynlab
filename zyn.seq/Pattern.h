#ifndef PATTERN_H
#define PATTERN_H

#include <map>

#define NUM_PATTERN_EVENTS 16

class PatternEvent
{
public:
    PatternEvent();
    virtual ~PatternEvent();

    unsigned char _note;
    unsigned char _velocity;
    unsigned char _gate;
    unsigned char _swing;
};

class Pattern
{
public:
    virtual ~Pattern();

    PatternEvent _events[NUM_PATTERN_EVENTS];
};

#endif // PATTERN_H
