#ifndef PATTERN_H
#define PATTERN_H

#include <set>
#include <string>

class TrackPatternNote
{
public:
    TrackPatternNote() {}
    TrackPatternNote(unsigned char note, int step) : _note(note), _step(step) {}

    bool operator<(TrackPatternNote const &other) const { return (_note < other._note) || (_note == other._note && _step < other._step); }
    unsigned char _note;
    int _step;
};

class TrackPattern
{
public:
    TrackPattern() {}
    TrackPattern(std::string const &name, float hue) : _name(name), _hue(hue) {}

    std::string _name;
    float _hue;
    std::set<TrackPatternNote> _notes;
};

#endif // PATTERN_H
