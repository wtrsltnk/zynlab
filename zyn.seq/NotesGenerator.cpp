#include "NotesGenerator.h"

NotesGenerator::NotesGenerator(NotesGeneratorOptions options)
    : _options(options)
{}

NotesGenerator::~NotesGenerator() = default;

std::vector<unsigned char> notesFromChord(Chords::Enum chord, unsigned char baseNote)
{
    switch (chord)
    {
        case Chords::Enum::Major:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 4),
                static_cast<unsigned char>(baseNote + 7),
            };
        case Chords::Enum::Minor:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 3),
                static_cast<unsigned char>(baseNote + 7),
            };
        case Chords::Enum::Diminished:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 3),
                static_cast<unsigned char>(baseNote + 6),
            };
        case Chords::Enum::MajorSeventh:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 4),
                static_cast<unsigned char>(baseNote + 7),
                static_cast<unsigned char>(baseNote + 11),
            };
        case Chords::Enum::MinorSeventh:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 3),
                static_cast<unsigned char>(baseNote + 7),
                static_cast<unsigned char>(baseNote + 10),
            };
        case Chords::Enum::DominantSeventh:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 4),
                static_cast<unsigned char>(baseNote + 7),
                static_cast<unsigned char>(baseNote + 10),
            };
        case Chords::Enum::Suspended2:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 2),
                static_cast<unsigned char>(baseNote + 7),
            };
        case Chords::Enum::Suspended4:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 5),
                static_cast<unsigned char>(baseNote + 7),
            };
        case Chords::Enum::Augmented:
            return {
                baseNote,
                static_cast<unsigned char>(baseNote + 4),
                static_cast<unsigned char>(baseNote + 8),
            };
        case Chords::Enum::Count:
        {
            return {};
        }
    }
}

void NotesGenerator::Generate(TrackRegion &region, TrackRegionEvent const baseEvent)
{
    if (_options.ArpMode == ArpModes::Enum::OnlyChords)
    {
        auto notes = notesFromChord(_options.Chord, baseEvent.note);
        auto start = baseEvent.values[0];
        auto length = baseEvent.values[1] - baseEvent.values[0];
        auto velocity = baseEvent.velocity;

        region.ClearAllNotes();

        auto regionLength = region.startAndEnd[1] - region.startAndEnd[0];
        while (start + length < regionLength)
        {
            for (auto n : notes)
            {
                TrackRegionEvent a = {
                    {start, start + length},
                    n,
                    velocity,
                };
                region.eventsByNote[n].push_back(a);
            }

            start += (length + _options.Skips * length);
        }
    }
}
