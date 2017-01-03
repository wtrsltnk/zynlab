#include <iostream>
#include <audioeffectx.h>
#include <math.h>
#include "zyn.common/Util.h"
#include "zyn.common/XMLwrapper.h"
#include "zyn.fx/EffectMgr.h"

SYNTH_T* synth;

#define NUM_PROGRAMS    2
#define NUM_PARAMS      0

class Zynstrument : public AudioEffectX
{
public:
    Zynstrument(audioMasterCallback audioMaster);
    ~Zynstrument();

    virtual void process (float **inputs, float **outputs, VstInt32 sampleframes);
    virtual void processReplacing (float **inputs, float **outputs, VstInt32 sampleFrames);

    VstInt32 processEvents(VstEvents* ev);

    virtual bool getEffectName (char* name) { strcpy(name, "zyffect"); return true; }	///< Fill \e text with a string identifying the effect
    virtual bool getVendorString (char* text) { strcpy(text, "zyn project"); return true; }	///< Fill \e text with a string identifying the vendor
    virtual bool getProductString (char* text) { strcpy(text, "zyffect"); return true; }///< Fill \e text with a string identifying the product name
    virtual VstInt32 getVendorVersion () { return 0; }			///< Return vendor-specific version

    virtual bool getProgramNameIndexed (VstInt32 category, VstInt32 index, char* text) { return false; } ///< Fill \e text with name of program \e index (\e category deprecated in VST 2.4)
    virtual void setProgram (VstInt32 program);	///< Set the current program to \e program

    float fGain;
    char programName[32];
    pthread_mutex_t mutex;
    EffectMgr* effect;
};

Zynstrument::Zynstrument(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
    fGain = 0.5f;
    setNumInputs (2);		// stereo in
    setNumOutputs (2);		// stereo out
//    setUniqueID ('ZynEffects');	// identify
    canProcessReplacing ();	// supports both accumulating and replacing output
    strcpy (programName, "Default");	// default program name

    synth = new SYNTH_T;
    synth->alias();

    //produce denormal buf
    denormalkillbuf = new float [synth->buffersize];
    for(int i = 0; i < synth->buffersize; ++i)
        denormalkillbuf[i] = (RND - 0.5f) * 1e-16;

    pthread_mutex_init(&mutex, NULL);
    effect = new EffectMgr(false, &mutex);
    effect->changeeffect(3);
}

Zynstrument::~Zynstrument()
{
    config.save();
    delete []denormalkillbuf;
    denormalkillbuf = 0;
    delete synth;
    synth = 0;
}

void Zynstrument::setProgram (VstInt32 program)
{
    curProgram = program;
//    effect->defaults();
//    if (curProgram >= 0 &&  curProgram < 8)
//    {
//        effect->defaults();
//        effect->changeeffect(curProgram + 1);
//    }
}

//-----------------------------------------------------------------------------------------
void Zynstrument::process (float **inputs, float **outputs, VstInt32 sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

    for (int i = 0; i < sampleFrames; i++)
    {
        out1[i] = in1[i];
        out2[i] = in2[i];
    }
    effect->out(out1, out2);

//    while (--sampleFrames >= 0)
//    {
//        (*out1++) += (*in1++) * fGain;    // accumulating
//        (*out2++) += (*in2++) * fGain;
//    }
}

//-----------------------------------------------------------------------------------------
void Zynstrument::processReplacing (float **inputs, float **outputs, VstInt32 sampleFrames)
{
    float *in1  =  inputs[0];
    float *in2  =  inputs[1];
    float *out1 = outputs[0];
    float *out2 = outputs[1];

    for (int i = 0; i < sampleFrames; i++)
    {
        out1[i] = in1[i];
        out2[i] = in2[i];
    }
    effect->out(out1, out2);

//    while (--sampleFrames >= 0)
//    {
//        (*out1++) = (*in1++) * fGain;    // replacing
//        (*out2++) = (*in2++) * fGain;
//    }
}

VstInt32 Zynstrument::processEvents (VstEvents* ev)
{
    return 0;
}

AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
    config.init();
    return new Zynstrument(audioMaster);
}

extern "C" {

#if defined (__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
    #define VST_EXPORT	__attribute__ ((visibility ("default")))
#else
    #define VST_EXPORT
#endif

//------------------------------------------------------------------------
/** Prototype of the export function main */
//------------------------------------------------------------------------
VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster)
{
    // Get VST Version of the Host
    if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
        return 0;  // old version

    // Create the AudioEffect
    AudioEffect* effect = createEffectInstance (audioMaster);
    if (!effect)
        return 0;

    // Return the VST AEffect structur
    return effect->getAeffect ();
}

// support for old hosts not looking for VSTPluginMain
#if (TARGET_API_MAC_CARBON && __ppc__)
VST_EXPORT AEffect* main_macho (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif WIN32
VST_EXPORT AEffect* MAIN (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif BEOS
VST_EXPORT AEffect* main_plugin (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#endif

} // extern "C"
