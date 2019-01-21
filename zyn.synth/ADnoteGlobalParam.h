#ifndef ADNOTEGLOBALPARAM_H
#define ADNOTEGLOBALPARAM_H

#include "EnvelopeParams.h"
#include "LFOParams.h"
#include "Resonance.h"
#include <zyn.dsp/FilterParams.h>

/*****************************************************************/
/*                    GLOBAL PARAMETERS                          */
/*****************************************************************/

struct ADnoteGlobalParam
{
    ADnoteGlobalParam();
    ~ADnoteGlobalParam();

    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);
    void Defaults();

    /* The instrument type  - MONO/STEREO
    If the mode is MONO, the panning of voices are not used
    Stereo=1, Mono=0. */
    unsigned char PStereo;

    /******************************************
    *     FREQUENCY GLOBAL PARAMETERS        *
    ******************************************/
    unsigned short int PDetune;       //fine detune
    unsigned short int PCoarseDetune; //coarse detune+octave
    unsigned char PDetuneType;        //detune type

    unsigned char PBandwidth; //how much the relative fine detunes of the voices are changed

    EnvelopeParams *FreqEnvelope; //Frequency Envelope

    LFOParams *FreqLfo; //Frequency LFO

    /********************************************
    *     AMPLITUDE GLOBAL PARAMETERS          *
    ********************************************/

    /* Panning -  0 - random
              1 - left
             64 - center
            127 - right */
    unsigned char PPanning;

    unsigned char PVolume;

    unsigned char PAmpVelocityScaleFunction;

    EnvelopeParams *AmpEnvelope;

    LFOParams *AmpLfo;

    unsigned char PPunchStrength, PPunchTime, PPunchStretch,
        PPunchVelocitySensing;

    /******************************************
    *        FILTER GLOBAL PARAMETERS        *
    ******************************************/
    FilterParams *GlobalFilter;

    // filter velocity sensing
    unsigned char PFilterVelocityScale;

    // filter velocity sensing
    unsigned char PFilterVelocityScaleFunction;

    EnvelopeParams *FilterEnvelope;

    LFOParams *FilterLfo;

    // RESONANCE
    Resonance *Reson;

    //how the randomness is applied to the harmonics on more voices using the same oscillator
    unsigned char Hrandgrouping;
};

#endif // ADNOTEGLOBALPARAM_H
