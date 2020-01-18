#include "ArpModes.h"

char const *const ARP_MODE_NAME_ONLY_ONE_NOTE = "One note";
char const *const ARP_MODE_NAME_ONLY_CHORDS = "Only chords";
char const *const ARP_MODE_NAME_UP = "Up";
char const *const ARP_MODE_NAME_DOWN = "Down";
char const *const ARP_MODE_NAME_UP_AND_DOWN_INC = "Up and Down inclusive";
char const *const ARP_MODE_NAME_UP_AND_DOWN_EXCL = "Up and Down exclusive";
char const *const ARP_MODE_NAME_DOWN_AND_UP_INC = "Down and Up inclusive";
char const *const ARP_MODE_NAME_DOWN_AND_UP_EXCL = "Down and Up exclusive";

char const *ArpModes::ToString(ArpModes::Enum arpMode)
{
    switch (arpMode)
    {
        case Enum::OneNote:
            return ARP_MODE_NAME_ONLY_ONE_NOTE;
        case Enum::OnlyChords:
            return ARP_MODE_NAME_ONLY_CHORDS;
        case Enum::Up:
            return ARP_MODE_NAME_UP;
        case Enum::Down:
            return ARP_MODE_NAME_DOWN;
        case Enum::UpAndDownInclusive:
            return ARP_MODE_NAME_UP_AND_DOWN_INC;
        case Enum::UpAndDownExclusive:
            return ARP_MODE_NAME_UP_AND_DOWN_EXCL;
        case Enum::DownAndUpIclusive:
            return ARP_MODE_NAME_DOWN_AND_UP_INC;
        case Enum::DownAndUpExclusive:
            return ARP_MODE_NAME_DOWN_AND_UP_EXCL;
        case Enum::Count:
            return "";
    }
    return "";
}

ArpModes::Enum ArpModes::ToEnum(unsigned char value)
{
    return static_cast<ArpModes::Enum>(value);
}

ArpModes::Enum ArpModes::ToEnum(unsigned int value)
{
    return static_cast<ArpModes::Enum>(value);
}

char const *const ArpModes::Names[] = {
    ARP_MODE_NAME_ONLY_ONE_NOTE,
    ARP_MODE_NAME_ONLY_CHORDS,
    ARP_MODE_NAME_UP,
    ARP_MODE_NAME_DOWN,
    ARP_MODE_NAME_UP_AND_DOWN_INC,
    ARP_MODE_NAME_UP_AND_DOWN_EXCL,
    ARP_MODE_NAME_DOWN_AND_UP_INC,
    ARP_MODE_NAME_DOWN_AND_UP_EXCL,
};
