#ifndef ADNOTEVOICEPARAM_H
#define ADNOTEVOICEPARAM_H

#include "EnvelopeParams.h"
#include "LFOParams.h"
#include "OscilGen.h"
#include "Resonance.h"
#include <zyn.dsp/FilterParams.h>

/***********************************************************/
/*                    VOICE PARAMETERS                     */
/***********************************************************/
class ADnoteVoiceParam : public WrappedPresets
{
public:
    void InitPresets();

    void Deserialize(IPresetsSerializer *xml, unsigned nvoice);
    void Serialize(IPresetsSerializer *xml, bool fmoscilused);
    void Deserialize(IPresetsSerializer *xml);
    void Serialize(IPresetsSerializer *xml);
    void Defaults();

    void Enable(IFFTwrapper *fft, Resonance *Reson);
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
    unsigned char Unison_invert_phase; //0=none,1=random,2=50%,3=33%,4=25%

    /** Type of the voice (0=Sound,1=Noise)*/
    unsigned char Type;

    /** Voice Delay */
    unsigned char PDelay;

    /** If the resonance is enabled for this voice */
    unsigned char Presonance;

    // What external oscil should I use, -1 for internal OscilSmp&FMSmp
    short int Pextoscil, PextFMoscil;
    // it is not allowed that the externoscil,externFMoscil => current voice

    // oscillator phases
    unsigned char Poscilphase, PFMoscilphase;

    // filter bypass
    unsigned char Pfilterbypass;

    /** Voice oscillator */
    OscilGen *OscilSmp;

    /**********************************
    *     FREQUENCY PARAMETERS        *
    **********************************/

    /** If the base frequency is fixed to 440 Hz*/
    unsigned char Pfixedfreq;

    /* Equal temperate (this is used only if the Pfixedfreq is enabled)
       If this parameter is 0, the frequency is fixed (to 440 Hz);
       if this parameter is 64, 1 MIDI halftone -> 1 frequency halftone */
    unsigned char PfixedfreqET;

    /** Fine detune */
    unsigned short int PDetune;

    /** Coarse detune + octave */
    unsigned short int PCoarseDetune;

    /** Detune type */
    unsigned char PDetuneType;

    /* Frequency Envelope */
    unsigned char PFreqEnvelopeEnabled;
    EnvelopeParams *FreqEnvelope;

    /* Frequency LFO */
    unsigned char PFreqLfoEnabled;
    LFOParams *FreqLfo;

    /***************************
    *   AMPLITUDE PARAMETERS   *
    ***************************/

    /* Panning
             0 - random
             1 - left
            64 - center
           127 - right
       The Panning is ignored if the instrument is mono */
    unsigned char PPanning;

    /* Voice Volume */
    unsigned char PVolume;

    /* If the Volume negative */
    unsigned char PVolumeminus;

    /* Velocity sensing */
    unsigned char PAmpVelocityScaleFunction;

    /* Amplitude Envelope */
    unsigned char PAmpEnvelopeEnabled;
    EnvelopeParams *AmpEnvelope;

    /* Amplitude LFO */
    unsigned char PAmpLfoEnabled;
    LFOParams *AmpLfo;

    /*************************
    *   FILTER PARAMETERS    *
    *************************/

    /* Voice Filter */
    unsigned char PFilterEnabled;
    FilterParams *VoiceFilter;

    /* Filter Envelope */
    unsigned char PFilterEnvelopeEnabled;
    EnvelopeParams *FilterEnvelope;

    /* LFO Envelope */
    unsigned char PFilterLfoEnabled;
    LFOParams *FilterLfo;

    /****************************
    *   MODULLATOR PARAMETERS   *
    ****************************/

    /* Modullator Parameters (0=off,1=Morph,2=RM,3=PM,4=FM.. */
    unsigned char PFMEnabled;

    /* Voice that I use as modullator instead of FMSmp.
       It is -1 if I use FMSmp(default).
       It maynot be equal or bigger than current voice */
    short int PFMVoice;

    /* Modullator oscillator */
    OscilGen *FMSmp;

    /* Modullator Volume */
    unsigned char PFMVolume;

    /* Modullator damping at higher frequencies */
    unsigned char PFMVolumeDamp;

    /* Modullator Velocity Sensing */
    unsigned char PFMVelocityScaleFunction;

    /* Fine Detune of the Modullator*/
    unsigned short int PFMDetune;

    /* Coarse Detune of the Modullator */
    unsigned short int PFMCoarseDetune;

    /* The detune type */
    unsigned char PFMDetuneType;

    /* Frequency Envelope of the Modullator */
    unsigned char PFMFreqEnvelopeEnabled;
    EnvelopeParams *FMFreqEnvelope;

    /* Frequency Envelope of the Modullator */
    unsigned char PFMAmpEnvelopeEnabled;
    EnvelopeParams *FMAmpEnvelope;
};

#endif // ADNOTEVOICEPARAM_H
