#include "vst.instrument.h"

#define NUM_PROGRAMS 2
#define NUM_PARAMS 0

ZynInstrument::ZynInstrument(audioMasterCallback audioMaster)
    : VstPlugin(audioMaster, NUM_PROGRAMS, NUM_PARAMS), fft(nullptr), currentProgram(0),
      adpars(nullptr)
{
    ctl.Init(&settings);
    ctl.defaults();
    fft = new FFTwrapper(settings.oscilsize);

    adpars = new ADnoteParameters(&settings, fft);
    adpars->Defaults();

    setNumInputs(0);
    setNumOutputs(2);
    isSynth(true);
    setEditor(new ZynEditor(this));
}

ZynInstrument::~ZynInstrument()
{
    delete adpars;
    delete fft;
    delete editor;
}

bool ZynInstrument::getEffectName(char *name)
{
    return strcpy(name, "zyn.vstadd") != nullptr;
}

bool ZynInstrument::getVendorString(char *text)
{
    return strcpy(text, "zyn lab") != nullptr;
}

bool ZynInstrument::getProductString(char *text)
{
    return strcpy(text, "zyn.vstadd") != nullptr;
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
    return new ADnote(adpars,
                      &ctl,
                      &settings,
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
