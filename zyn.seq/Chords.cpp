#include "Chords.h"

char const *const CHORD_MAJOR = "Major";
char const *const CHORD_MINOR = "Minor";
char const *const CHORD_DIMINISHED = "Diminished";
char const *const CHORD_MAJOR_SEVENTH = "Major Seventh";
char const *const CHORD_MINOR_SEVENTH = "Minor Seventh";
char const *const CHORD_DOMINANT_SEVENTH = "Dominant Seventh";
char const *const CHORD_SUSPENDED2 = "Suspended2";
char const *const CHORD_SUSPENDED4 = "Suspended4";
char const *const CHORD_AUGMENTED = "Augmented";

char const *Chords::ToString(Chords::Enum chord)
{
    switch (chord)
    {
        case Enum::Major:
            return CHORD_MAJOR;
        case Enum::Minor:
            return CHORD_MINOR;
        case Enum::Diminished:
            return CHORD_DIMINISHED;
        case Enum::MajorSeventh:
            return CHORD_MAJOR_SEVENTH;
        case Enum::MinorSeventh:
            return CHORD_MINOR_SEVENTH;
        case Enum::DominantSeventh:
            return CHORD_DOMINANT_SEVENTH;
        case Enum::Suspended2:
            return CHORD_SUSPENDED2;
        case Enum::Suspended4:
            return CHORD_SUSPENDED4;
        case Enum::Augmented:
            return CHORD_AUGMENTED;
        case Enum::Count:
            return "";
    }
    return "";
}

Chords::Enum Chords::ToEnum(
    unsigned char value)
{
    return static_cast<Chords::Enum>(value);
}

Chords::Enum Chords::ToEnum(
    unsigned int value)
{
    return static_cast<Chords::Enum>(value);
}

std::vector<std::string> Chords::Names = {
    CHORD_MAJOR,
    CHORD_MINOR,
    CHORD_DIMINISHED,
    CHORD_MAJOR_SEVENTH,
    CHORD_MINOR_SEVENTH,
    CHORD_DOMINANT_SEVENTH,
    CHORD_SUSPENDED2,
    CHORD_SUSPENDED4,
    CHORD_AUGMENTED,
};
