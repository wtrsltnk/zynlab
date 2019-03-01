#ifndef ARPMODES_H
#define ARPMODES_H

struct ArpModes
{
    enum Enum
    {
        OnlyChords,
        Up,
        Down,
        UpAndDownInclusive,
        UpAndDownExclusive,
        DownAndUpIclusive,
        DownAndUpExclusive,
        Count,
    };

    static char const *ToString(ArpModes::Enum arpMode);
    static ArpModes::Enum ToEnum(unsigned char value);
    static ArpModes::Enum ToEnum(unsigned int value);
    static char const *const Names[];
};

#endif // ARPMODES_H
