#ifndef ABSTRACTNOTEPARAMETERS_H
#define ABSTRACTNOTEPARAMETERS_H

#include "EnvelopeParams.h"
#include "LFOParams.h"
#include <zyn.common/Presets.h>
#include <zyn.dsp/FilterParams.h>

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

    /******************************************
    *     FREQUENCY GLOBAL PARAMETERS        *
    ******************************************/

    /** If the base frequency is fixed to 440 Hz */
    unsigned char Pfixedfreq;

    /** Equal temperate (this is used only if the Pfixedfreq is enabled)
           If this parameter is 0, the frequency is fixed (to 440 Hz);
           if this parameter is 64, 1 MIDI halftone -> 1 frequency halftone */
    unsigned char PfixedfreqET;

    /** Fine detune */
    unsigned short int PDetune;

    /** Coarse detune + octave */
    unsigned short int PCoarseDetune;

    /** Detune type */
    unsigned char PDetuneType;

    /** Frequency Envelope */
    unsigned char PFreqEnvelopeEnabled;
    EnvelopeParams *FreqEnvelope = nullptr;

    /** Frequency LFO */
    unsigned char PFreqLfoEnabled;
    LFOParams *FreqLfo = nullptr;

    /** how much the relative fine detunes of the voices are changed */
    unsigned char PBandwidth;

    /***************************
    *   AMPLITUDE PARAMETERS   *
    ***************************/

    /** Amplitude Envelope */
    unsigned char PAmpEnvelopeEnabled;
    EnvelopeParams *AmpEnvelope = nullptr;

    /******************************************
    *        FILTER GLOBAL PARAMETERS        *
    ******************************************/

    /** Global Filter */
    FilterParams *GlobalFilter = nullptr;

    /** filter velocity sensing */
    unsigned char PFilterVelocityScale;
    unsigned char PFilterVelocityScaleFunction;

    /** Filter Envelope */
    EnvelopeParams *FilterEnvelope = nullptr;
};

#endif // ABSTRACTNOTEPARAMETERS_H
