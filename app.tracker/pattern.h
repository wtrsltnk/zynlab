#ifndef PATTERN_H
#define PATTERN_H

#include "note.h"

#include <map>
#include <string>
#include <vector>
#include <zyn.common/globals.h>

class AutomatedParameter
{
public:
    float GetValue(
        unsigned int step);

    //private:
    int _paramIndex;
    std::map<unsigned int, float> _keyFrames;
};

class Pattern
{
public:
    Pattern(
        std::string const &name,
        unsigned int length = 64);

    std::string const &Name() const;

    void Rename(
        std::string const &name);

    unsigned int Length() const;

    void Resize(
        unsigned int len);

    std::vector<Note> &Notes(
        unsigned int trackIndex);

    std::map<int, AutomatedParameter> &AutomatedTrackParameters(
        unsigned int trackIndex);

private:
    std::string _name;
    unsigned int _length;
    std::vector<Note> _notes[NUM_MIXER_TRACKS];
    std::map<int, AutomatedParameter> _automatedTrackParameters[NUM_MIXER_TRACKS];
};

#endif // PATTERN_H
