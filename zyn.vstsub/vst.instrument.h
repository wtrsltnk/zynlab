#ifndef VST_INSTRUMENT_H
#define VST_INSTRUMENT_H

#include <aeffeditor.h>
#include <zyn.synth/Controller.h>
#include <zyn.synth/SUBnote.h>
#include <zyn.synth/SUBnoteParams.h>
#include <zyn.vst/vstcontrol.h>
#include <zyn.vst/vstknob.h>
#include <zyn.vst/vstplugin.h>

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

class ZynInstrument : public VstPlugin
{
    Controller ctl;
    VstInt32 currentProgram;
    SUBnoteParameters *parameters;

public:
    ZynInstrument(audioMasterCallback audioMaster);
    virtual ~ZynInstrument();

    virtual SynthNote *createNote(VstInt32 note, VstInt32 velocity);

    virtual bool getEffectName(char *name);
    virtual bool getVendorString(char *text);
    virtual bool getProductString(char *text);
    virtual VstInt32 getVendorVersion();

    virtual bool getProgramNameIndexed(VstInt32 category, VstInt32 index, char *text);
    virtual void setProgram(VstInt32 program);
    virtual void getProgramName(char *name);
};

#endif // VST_INSTRUMENT_H
