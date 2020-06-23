#include "pattern.h"
#include <iostream>

Pattern::Pattern(
    std::string const &name,
    unsigned int length)
    : _name(name),
    _length(length)
{
    for (unsigned int t = 0; t < NUM_MIXER_TRACKS; t++)
    {
        _notes[t].resize(_length);
    }
}

std::string const &Pattern::Name() const
{
    return _name;
}

void Pattern::Rename(
    std::string const &name)
{
    std::cout << _name << "" << name << std::endl;
    _name = name;
    std::cout << _name << "" << name << std::endl;
}

unsigned int Pattern::Length() const
{
    return _length;
}

void Pattern::Resize(
    unsigned int len)
{
    if (len < 4) return;
    if (len > 128) return;

    _length = len;

    for (unsigned int t = 0; t < NUM_MIXER_TRACKS; t++)
    {
        _notes[t].resize(_length);
    }
}

std::vector<Note> &Pattern::Notes(
    unsigned int trackIndex)
{
    return _notes[trackIndex];
}
