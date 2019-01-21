#include "vst.instrument.h"

#include <cmath>

#define NUM_PROGRAMS 2
#define NUM_PARAMS 0

ZynInstrument::ZynInstrument(audioMasterCallback audioMaster)
    : VstPlugin(audioMaster, NUM_PROGRAMS, NUM_PARAMS), currentProgram(0),
      parameters(nullptr)
{
    ctl.Init();
    ctl.Defaults();

    parameters = new SUBnoteParameters();
    parameters->Defaults();

    setNumInputs(0);
    setNumOutputs(2);
    isSynth(true);
    setEditor(new ZynEditor(this));
}

ZynInstrument::~ZynInstrument()
{
    delete parameters;
    delete editor;
}

bool ZynInstrument::getEffectName(char *name)
{
    return strcpy(name, "zyn.vstsub") != nullptr;
}

bool ZynInstrument::getVendorString(char *text)
{
    return strcpy(text, "zyn lab") != nullptr;
}

bool ZynInstrument::getProductString(char *text)
{
    return strcpy(text, "zyn.vstsub") != nullptr;
}

VstInt32 ZynInstrument::getVendorVersion()
{
    return 100;
}

bool ZynInstrument::getProgramNameIndexed(VstInt32 /*category*/, VstInt32 index, char *text)
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

void ZynInstrument::setProgram(VstInt32 program)
{
    currentProgram = program;
}

void ZynInstrument::getProgramName(char *name)
{
    getProgramNameIndexed(0, currentProgram, name);
}

SynthNote *ZynInstrument::createNote(VstInt32 note, VstInt32 velocity)
{
    float notebasefreq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);
    return new SUBnote(parameters,
                       &ctl,
                       notebasefreq,
                       velocity,
                       0,
                       note,
                       false);
}

AudioEffect *createEffectInstance(audioMasterCallback audioMaster)
{
    return new ZynInstrument(audioMaster);
}
