#include "NotesGenerator.h"
#include <algorithm>

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

    return {};
}

void NotesGenerator::Generate(
    RegionsManager *regionsManager,
    int trackIndex,
    int regionIndex,
    TrackRegionEvent const baseEvent)
{
    TrackRegion &region = regionsManager->GetRegion(trackIndex, regionIndex);

    std::vector<unsigned char> notes = notesFromChord(_options.Chord, baseEvent.note);
    std::vector<unsigned char> reverseNotes(notes);
    std::reverse(std::begin(reverseNotes), std::end(reverseNotes));

    timestep start = baseEvent.values[0];
    timestep length = baseEvent.values[1] - baseEvent.values[0];
    timestep regionLength = region.startAndEnd[1] - region.startAndEnd[0];
    unsigned char velocity = baseEvent.velocity;

    regionsManager->RemoveRegionEvent(trackIndex, regionIndex, baseEvent);

    TrackRegionEvent lastNote;
    while (start + length < regionLength)
    {
        switch (_options.ArpMode)
        {
            case ArpModes::Enum::OneNote:
            {
                TrackRegionEvent a = {
                    {start, start + length},
                    baseEvent.note,
                    velocity,
                };
                region.eventsByNote[baseEvent.note].push_back(a);

                start += (length + _options.Space * length);
                break;
            }
            case ArpModes::Enum::OnlyChords:
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

                start += (length + _options.Space * length);
                break;
            }
            case ArpModes::Enum::Up:
            {
                for (auto n : notes)
                {
                    TrackRegionEvent a = {
                        {start, start + length},
                        n,
                        velocity,
                    };
                    region.eventsByNote[n].push_back(a);

                    start += (length + _options.Space * length);
                }
                break;
            }
            case ArpModes::Enum::Down:
            {
                for (auto n : reverseNotes)
                {
                    TrackRegionEvent a = {
                        {start, start + length},
                        n,
                        velocity,
                    };
                    region.eventsByNote[n].push_back(a);

                    start += (length + _options.Space * length);
                }
                break;
            }
            case ArpModes::Enum::UpAndDownInclusive:
            {
                std::reverse(std::begin(reverseNotes), std::end(reverseNotes));
                for (auto n : reverseNotes)
                {
                    TrackRegionEvent a = {
                        {start, start + length},
                        n,
                        velocity,
                    };
                    region.eventsByNote[n].push_back(a);

                    start += (length + _options.Space * length);
                }
                break;
            }
            case ArpModes::Enum::DownAndUpIclusive:
            {
                std::reverse(std::begin(notes), std::end(notes));
                for (auto n : notes)
                {
                    TrackRegionEvent a = {
                        {start, start + length},
                        n,
                        velocity,
                    };
                    region.eventsByNote[n].push_back(a);

                    start += (length + _options.Space * length);
                }
                break;
            }
            case ArpModes::Enum::UpAndDownExclusive:
            {
                std::reverse(std::begin(reverseNotes), std::end(reverseNotes));
                for (auto n : reverseNotes)
                {
                    if (n == lastNote.note) continue;
                    TrackRegionEvent a = {
                        {start, start + length},
                        n,
                        velocity,
                    };
                    region.eventsByNote[n].push_back(a);

                    start += (length + _options.Space * length);
                    lastNote = a;
                }
                break;
            }
            case ArpModes::Enum::DownAndUpExclusive:
            {
                std::reverse(std::begin(notes), std::end(notes));
                for (auto n : notes)
                {
                    if (n == lastNote.note) continue;
                    TrackRegionEvent a = {
                        {start, start + length},
                        n,
                        velocity,
                    };
                    region.eventsByNote[n].push_back(a);

                    start += (length + _options.Space * length);
                    lastNote = a;
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }
}
