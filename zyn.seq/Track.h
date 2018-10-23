#ifndef TRACK_H
#define TRACK_H

#include "Pattern.h"
#include <vector>

class Track
{
public:
    Track();
    virtual ~Track();

    unsigned int _patternsStart;
    std::vector<Pattern> _patterns;
};

#endif // TRACK_H
