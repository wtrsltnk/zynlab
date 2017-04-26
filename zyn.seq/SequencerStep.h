#ifndef SEQUENCERSTEP_H
#define SEQUENCERSTEP_H


class SequencerStep
{
public:
    SequencerStep(char note, char velocity);
    SequencerStep(const SequencerStep& clone);

    SequencerStep& operator = (const SequencerStep& clone);
    char _note;
    char _velocity;
};

#endif // SEQUENCERSTEP_H
