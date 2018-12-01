#ifndef VST_INSTRUMENT_H
#define VST_INSTRUMENT_H

#include <aeffeditor.h>
#include <audioeffectx.h>
#include <zyn.common/globals.h>
#include <zyn.synth/ADnote.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/Controller.h>
#include <zyn.synth/FFTwrapper.h>
#include <zyn.vst/vstcontrol.h>
#include <zyn.vst/vstknob.h>

class ZynEditor : public AEffEditor, public VstControl
{
    VstControl _main;
    VstKnob knob;
    ERect _rect;

public:
    ZynEditor(AudioEffect *effect);

    virtual bool getRect(ERect **rect);
    virtual bool open(void *ptr);
    virtual void close();
    virtual bool isOpen();
    virtual void idle();
};

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

    unsigned int _lastGeneratedBufferSize;
    unsigned int _lastSampleFrames;

public:
    Zynstrument(audioMasterCallback audioMaster);
    ~Zynstrument();

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
    VstInt32 processEvents(VstEvents *ev);

    virtual bool getEffectName(char *name);
    virtual bool getVendorString(char *text);
    virtual bool getProductString(char *text);
    virtual VstInt32 getVendorVersion();

    virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text);
    virtual void setProgram(VstInt32 program);
    virtual void getProgramName(char *name);
};

#endif // VST_INSTRUMENT_H
