#ifndef SEQUENCERSTRIP_H
#define SEQUENCERSTRIP_H

#include <map>
#include "SequencerStep.h"

class SequencerStrip
{
public:
    SequencerStrip();

    int _targetMixerChannel;
    std::map<int, SequencerStep> _steps;
};

#endif // SEQUENCERSTRIP_H
