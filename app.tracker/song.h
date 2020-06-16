#ifndef SONG_H
#define SONG_H

#include "pattern.h"

#include <vector>

class Song
{
    std::vector<Pattern *> _patterns;

public:
    unsigned int currentPattern = 0;

    unsigned int GetPatternCount() const;
    Pattern *GetPattern(unsigned int index);
    void AddPattern();
    void RemovePattern(unsigned int index);
    void MovePattern(unsigned int index, int direction);
    void DuplicatePattern(unsigned int index);
};

#endif // SONG_H
