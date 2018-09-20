#include "SequencerStep.h"

SequencerStep::SequencerStep(char note, char velocity)
    : _note(note), _velocity(velocity)
{}

SequencerStep::SequencerStep(const SequencerStep &clone) { (*this) = clone; }

SequencerStep &SequencerStep::operator=(const SequencerStep &clone) = default;
