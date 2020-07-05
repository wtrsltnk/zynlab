#ifndef ABSTRACTNOTEPARAMETERS_H
#define ABSTRACTNOTEPARAMETERS_H

#include <zyn.common/Presets.h>

class AbstractNoteParameters :
    public WrappedPresets
{
public:
    AbstractNoteParameters();

    /* The instrument type  - MONO/STEREO
    If the mode is MONO, the panning of voices are not used
    Stereo=1, Mono=0. */
    unsigned char PStereo;

    /* Panning -  0 - random
                  1 - left
                 64 - center
                127 - right */
    unsigned char PPanning;

    unsigned char PVolume;

    unsigned char PAmpVelocityScaleFunction;
};

#endif // ABSTRACTNOTEPARAMETERS_H
