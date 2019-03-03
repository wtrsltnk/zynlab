#ifndef NOTESGENERATOR_H
#define NOTESGENERATOR_H

#include "ArpModes.h"
#include "Chords.h"
#include "RegionsManager.h"
#include "TrackRegion.h"

class NotesGeneratorOptions
{
public:
    ArpModes::Enum ArpMode;
    Chords::Enum Chord;
    int Space;
};

class NotesGenerator
{
    NotesGeneratorOptions _options;

public:
    NotesGenerator(NotesGeneratorOptions options);
    virtual ~NotesGenerator();

    void Generate(RegionsManager *regionsManager, int trackIndex, int regionIndex, TrackRegionEvent const baseEvent);
};

#endif // NotesGenerator
