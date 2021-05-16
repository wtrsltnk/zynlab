#ifndef ADNOTEVOICEPARAM_H
#define ADNOTEVOICEPARAM_H

#include "AbstractNoteParameters.h"
#include "EnvelopeParams.h"
#include "LFOParams.h"
#include "OscilGen.h"
#include "Resonance.h"
#include <zyn.dsp/FilterParams.h>

/***********************************************************/
/*                    VOICE PARAMETERS                     */
/***********************************************************/
class ADnoteVoiceParam :
    public AbstractNoteParameters
{
public:
    void InitPresets();

    void Defaults();

    void Enable(
        Resonance *Reson);

    void Disable();

    /** If the voice is enabled */
    unsigned char Enabled;

    /** How many subvoices are used in this voice */
    unsigned char Unison_size;

    /** How subvoices are spread */
    unsigned char Unison_frequency_spread;

    /** How much phase randomization */
    unsigned char Unison_phase_randomness;

    /** Stereo spread of the subvoices*/
    unsigned char Unison_stereo_spread;

    /** Vibratto of the subvoices (which makes the unison more "natural")*/
    unsigned char Unison_vibratto;

    /** Medium speed of the vibratto of the subvoices*/
    unsigned char Unison_vibratto_speed;

    /** Unison invert phase */
    unsigned char Unison_invert_phase; // 0=none,1=random,2=50%,3=33%,4=25%

    /** Type of the voice (0=Sound,1=Noise)*/
    unsigned char Type;

    /** Voice Delay */
    unsigned char PDelay;

    /** If the resonance is enabled for this voice */
    unsigned char Presonance;

    /** What external oscil should I use, -1 for internal OscilSmp&FMSmp */
    short int Pextoscil, PextFMoscil;
    /** it is not allowed that the externoscil,externFMoscil => current voice */

    /** oscillator phases */
    unsigned char Poscilphase, PFMoscilphase;

    /** filter bypass */
    unsigned char Pfilterbypass;

    /** Voice oscillator */
    OscilGen *OscilSmp = nullptr;

    /***************************
    *   AMPLITUDE PARAMETERS   *
    ***************************/

    /** If the Volume negative */
    unsigned char PVolumeminus;

    /** Amplitude LFO */
    unsigned char PAmpLfoEnabled;
    LFOParams *AmpLfo = nullptr;

    /*************************
    *   FILTER PARAMETERS    *
    *************************/

    /** Voice Filter */
    unsigned char PFilterEnabled;
    FilterParams *VoiceFilter = nullptr;

    /** Filter Envelope */
    unsigned char PFilterEnvelopeEnabled;

    /** LFO Envelope */
    unsigned char PFilterLfoEnabled;
    LFOParams *FilterLfo = nullptr;

    /****************************
    *   MODULLATOR PARAMETERS   *
    ****************************/

    /** Modullator Parameters (0=off,1=Morph,2=RM,3=PM,4=FM.. */
    unsigned char PFMEnabled;

    /** Voice that I use as modullator instead of FMSmp.
       It is -1 if I use FMSmp(default).
       It maynot be equal or bigger than current voice */
    short int PFMVoice;

    /** Modullator oscillator */
    OscilGen *FMSmp = nullptr;

    /** Modullator Volume */
    unsigned char PFMVolume;

    /** Modullator damping at higher frequencies */
    unsigned char PFMVolumeDamp;

    /** Modullator Velocity Sensing */
    unsigned char PFMVelocityScaleFunction;

    /** Fine Detune of the Modullator*/
    unsigned short int PFMDetune;

    /** Coarse Detune of the Modullator */
    unsigned short int PFMCoarseDetune;

    /** The detune type */
    unsigned char PFMDetuneType;

    /* Frequency Envelope of the Modullator */
    unsigned char PFMFreqEnvelopeEnabled;
    EnvelopeParams *FMFreqEnvelope = nullptr;

    /** Frequency Envelope of the Modullator */
    unsigned char PFMAmpEnvelopeEnabled;
    EnvelopeParams *FMAmpEnvelope = nullptr;
};

#endif // ADNOTEVOICEPARAM_H
