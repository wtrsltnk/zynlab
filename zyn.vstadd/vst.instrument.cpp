#include <audioeffectx.h>
#include <iostream>
#include <math.h>
#include <zyn.common/Config.h>
#include <zyn.common/globals.h>
#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/ADnote.h>
#include <zyn.synth/Controller.h>
#include <zyn.synth/FFTwrapper.h>

#define NUM_PROGRAMS 2
#define NUM_PARAMS 0

class Zynstrument : public AudioEffectX
{
    SystemSettings settings;
    Controller ctl;
    FFTwrapper *fft;
    VstInt32 currentProgram;
    ADnoteParameters *adpars;
    SynthNote *playingNote;
    float *_tmpoutr;
    float *_tmpoutl;

public:
    Zynstrument(audioMasterCallback audioMaster);
    ~Zynstrument();

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
    VstInt32 processEvents(VstEvents *ev);

    virtual bool getEffectName(char *name)
    {
        strcpy(name, "zyn.vstadd");
        return true;
    } ///< Fill \e text with a string identifying the effect
    virtual bool getVendorString(char *text)
    {
        strcpy(text, "zyn lab");
        return true;
    } ///< Fill \e text with a string identifying the vendor
    virtual bool getProductString(char *text)
    {
        strcpy(text, "zyn.vstadd");
        return true;
    }                                                 ///< Fill \e text with a string identifying the product name
    virtual VstInt32 getVendorVersion() { return 0; } ///< Return vendor-specific version

    virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text); ///< Fill \e text with name of program \e index (\e category deprecated in VST 2.4)
    virtual void setProgram(VstInt32 program);                                         ///< Set the current program to \e program
    virtual void getProgramName(char *name);
};

Zynstrument::Zynstrument(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS), fft(nullptr), currentProgram(0), adpars(nullptr), playingNote(nullptr)
{
    this->isSynth(true);

    Config::Current().init();

    /* Get the settings from the Config*/
    settings.samplerate = Config::Current().cfg.SampleRate;
    settings.buffersize = Config::Current().cfg.SoundBufferSize;
    settings.oscilsize = Config::Current().cfg.OscilSize;
    settings.alias();

    ctl.Init(&settings);
    ctl.defaults();
    fft = new FFTwrapper(settings.oscilsize);

    adpars = new ADnoteParameters(&settings, fft);
    adpars->Defaults();

    _tmpoutr = new float[settings.buffersize];
    _tmpoutl = new float[settings.buffersize];
}

Zynstrument::~Zynstrument()
{
    delete adpars;
    delete fft;
}

bool Zynstrument::getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text)
{
    if (index == 0)
    {
        strcpy_s(text, 9, "preset01");
        return true;
    }

    if (index == 1)
    {
        strcpy_s(text, 9, "preset02");
        return true;
    }

    return false;
}

void Zynstrument::setProgram(VstInt32 program)
{
    currentProgram = program;
}

void Zynstrument::getProgramName(char *name)
{
    if (currentProgram == 0)
    {
        strcpy_s(name, 9, "preset01");
    }

    if (currentProgram == 1)
    {
        strcpy_s(name, 9, "preset02");
    }
}

void Zynstrument::processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames)
{
    if (playingNote == nullptr)
    {
        return;
    }

    playingNote->noteout(&_tmpoutl[0], &_tmpoutr[0]);

    outputs[0] = &_tmpoutl[0];
    outputs[1] = &_tmpoutr[0];

    if (playingNote->finished())
    {
        delete playingNote;
        playingNote = nullptr;
    }
}

VstInt32 Zynstrument::processEvents(VstEvents *ev)
{
    for (VstInt32 i = 0; i < ev->numEvents; i++)
    {
        if ((ev->events[i])->type != kVstMidiType)
            continue;

        VstMidiEvent *event = (VstMidiEvent *)ev->events[i];
        char *midiData = event->midiData;
        VstInt32 status = midiData[0] & 0xf0; // ignoring channel
        if (status == 0x90 || status == 0x80) // we only look at notes
        {
            VstInt32 note = midiData[1] & 0x7f;
            VstInt32 velocity = midiData[2] & 0x7f;
            if (status == 0x90)
            {
                // this->instrument->NoteOn(note, velocity, 0);
                if (playingNote == nullptr)
                {
                    float notebasefreq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
                    playingNote = new ADnote(adpars,
                                             &ctl,
                                             &settings,
                                             notebasefreq,
                                             velocity,
                                             0,
                                             note,
                                             false);
                }
            }
            else
            {
                if (playingNote != nullptr)
                {
                    playingNote->relasekey();
                }
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
    return new Zynstrument(audioMaster);
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
