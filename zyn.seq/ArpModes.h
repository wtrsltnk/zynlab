#ifndef ARPMODES_H
#define ARPMODES_H

#include <string>
#include <vector>

struct ArpModes
{
    enum Enum
    {
        OneNote,
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
    static std::vector<std::string> Names;
};

#endif // ARPMODES_H
