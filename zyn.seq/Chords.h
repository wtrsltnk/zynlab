#ifndef CHORDS_H
#define CHORDS_H

struct Chords
{
    enum Enum
    {
        Major,
        Minor,
        Diminished,
        MajorSeventh,
        MinorSeventh,
        DominantSeventh,
        Suspended2,
        Suspended4,
        Augmented,
        Count,
    };

    static char const *ToString(Chords::Enum chord);
    static Chords::Enum ToEnum(unsigned char value);
    static Chords::Enum ToEnum(unsigned int value);
    static char const *const Names[];
};

#endif // CHORDS_H
