#include "Pattern.h"

PatternEvent::PatternEvent()
    : _note(0), _velocity(0), _gate(0), _swing(0)
{}

PatternEvent::~PatternEvent() = default;

Pattern::~Pattern() = default;
