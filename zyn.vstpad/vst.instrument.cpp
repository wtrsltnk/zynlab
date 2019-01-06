#include <audioeffectx.h>
#include <iostream>
#include <math.h>
#include <zyn.common/globals.h>

static SystemSettings &settings = SystemSettings::Instance();

#define NUM_PROGRAMS 2
#define NUM_PARAMS 0

class ZynInstrument : public AudioEffectX
{
public:
    ZynInstrument(audioMasterCallback audioMaster);
    ~ZynInstrument();

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
    VstInt32 processEvents(VstEvents *ev);

    virtual bool getEffectName(char *name)
    {
        strcpy(name, "zynstrument");
        return true;
    } ///< Fill \e text with a string identifying the effect
    virtual bool getVendorString(char *text)
    {
        strcpy(text, "zyn project");
        return true;
    } ///< Fill \e text with a string identifying the vendor
    virtual bool getProductString(char *text)
    {
        strcpy(text, "zynstrument");
        return true;
    }                                                 ///< Fill \e text with a string identifying the product name
    virtual VstInt32 getVendorVersion() { return 0; } ///< Return vendor-specific version

    virtual bool getProgramNameIndexed(VstInt32 /*category*/, VstInt32 /*index*/, char * /*text*/) { return false; } ///< Fill \e text with name of program \e index (\e category deprecated in VST 2.4)
    virtual void setProgram(VstInt32 program);                                                                       ///< Set the current program to \e program
};

ZynInstrument::ZynInstrument(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS)
{
    this->isSynth(true);

    settings.alias();
}

ZynInstrument::~ZynInstrument()
{}

void ZynInstrument::setProgram(VstInt32 /*program*/)
{}

void ZynInstrument::processReplacing(float ** /*inputs*/, float ** /*outputs*/, VstInt32 /*sampleFrames*/)
{}

VstInt32 ZynInstrument::processEvents(VstEvents *ev)
{
    for (VstInt32 i = 0; i < ev->numEvents; i++)
    {
        if ((ev->events[i])->type != kVstMidiType)
            continue;

        VstMidiEvent *event = reinterpret_cast<VstMidiEvent *>(ev->events[i]);
        char *midiData = event->midiData;
        VstInt32 status = midiData[0] & 0xf0; // ignoring channel
        if (status == 0x90 || status == 0x80) // we only look at notes
        {
            VstInt32 note = midiData[1] & 0x7f;
            VstInt32 velocity = midiData[2] & 0x7f;
            if (status == 0x90)
            {
                // this->instrument->NoteOn(note, velocity, 0);
            }
            else
            {
                velocity = 0; // note off by velocity 0
                // this->instrument->NoteOff(note);
            }
        }
        event++;
    }
    return 1;
}

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
    return new ZynInstrument(audioMaster);
}

extern "C" {

#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
#define VST_EXPORT __attribute__((visibility("default")))
#else
#define VST_EXPORT
#endif

//------------------------------------------------------------------------
/** Prototype of the export function main */
//------------------------------------------------------------------------
VST_EXPORT AEffect *VSTPluginMain(audioMasterCallback audioMaster)
{
    // Get VST Version of the Host
    if (!audioMaster(0, audioMasterVersion, 0, 0, 0, 0))
        return 0; // old version

    // Create the AudioEffect
    AudioEffect *effect = createEffectInstance(audioMaster);
    if (!effect)
        return 0;

    // Return the VST AEffect structur
    return effect->getAeffect();
}

// support for old hosts not looking for VSTPluginMain
#if (TARGET_API_MAC_CARBON && __ppc__)
VST_EXPORT AEffect *main_macho(audioMasterCallback audioMaster)
{
    return VSTPluginMain(audioMaster);
}
#elif WIN32
VST_EXPORT AEffect *MAIN(audioMasterCallback audioMaster)
{
    return VSTPluginMain(audioMaster);
}
#elif BEOS
VST_EXPORT AEffect *main_plugin(audioMasterCallback audioMaster)
{
    return VSTPluginMain(audioMaster);
}
#endif

} // extern "C"
