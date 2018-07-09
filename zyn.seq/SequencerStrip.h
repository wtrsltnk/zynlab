#ifndef SEQUENCERSTRIP_H
#define SEQUENCERSTRIP_H

#include "SequencerStep.h"
#include <map>

class SequencerStrip
{
public:
    SequencerStrip();

    int _targetMixerChannel;
    std::map<int, SequencerStep> _steps;
};

#endif // SEQUENCERSTRIP_H
