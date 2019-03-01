#ifndef NOTESGENERATOR_H
#define NOTESGENERATOR_H

#include "ArpModes.h"
#include "Chords.h"
#include "TrackRegion.h"

class NotesGeneratorOptions
{
public:
    ArpModes::Enum ArpMode;
    Chords::Enum Chord;
    int Skips;
};

class NotesGenerator
{
    NotesGeneratorOptions _options;

public:
    NotesGenerator(NotesGeneratorOptions options);
    virtual ~NotesGenerator();

    void Generate(TrackRegion &region, TrackRegionEvent const baseEvent);
};

#endif // NotesGenerator
