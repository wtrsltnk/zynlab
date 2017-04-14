#ifndef SEQUENCERSTRIP_H
#define SEQUENCERSTRIP_H

#include <vector>
#include "SequencerStep.h"

class SequencerStrip
{
public:
    SequencerStrip();

    int _targetMixerChannel;
    std::vector<SequencerStep> _steps;
};

#endif // SEQUENCERSTRIP_H
