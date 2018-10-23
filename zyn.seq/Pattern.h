#ifndef PATTERN_H
#define PATTERN_H

#include "SequencerStep.h"
#include <map>

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

    PatternEvent _events[16];
};

#endif // PATTERN_H
