#include "pattern.h"
#include <iostream>

// TODO: make this work linear from the previous and the next keyframe
float AutomatedParameter::GetValue(
    unsigned int step)
{
    if (_keyFrames.empty())
    {
        return -1;
    }

    std::pair<unsigned int, float> prevValue;
    std::pair<unsigned int, float> nextValue;
    for (auto &pair : _keyFrames)
    {
        if (pair.first == step)
        {
            return pair.second;
        }
    }

    return -1;
}

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
    _name = name;
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

std::map<int, AutomatedParameter> &Pattern::AutomatedTrackParameters(
    unsigned int trackIndex)
{
    return _automatedTrackParameters[trackIndex];
}
