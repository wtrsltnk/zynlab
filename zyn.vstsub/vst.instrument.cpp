#include "vst.instrument.h"
#include <aeffeditor.h>
#include <audioeffectx.h>
#include <iostream>
#include <math.h>
#include <zyn.common/Config.h>
#include <zyn.common/globals.h>
#include <zyn.mixer/Channel.h>
#include <zyn.synth/ADnote.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/Controller.h>
#include <zyn.synth/FFTwrapper.h>
#include <zyn.vst/vstcontrol.h>
#include <zyn.vst/vstknob.h>

#define NUM_PROGRAMS 2
#define NUM_PARAMS 0

bool Zynstrument::getEffectName(char *name)
{
    strcpy(name, "zyn.vstsub");
    return true;
}

bool Zynstrument::getVendorString(char *text)
{
    strcpy(text, "zyn lab");
    return true;
}

bool Zynstrument::getProductString(char *text)
{
    strcpy(text, "zyn.vstsub");
    return true;
}

VstInt32 Zynstrument::getVendorVersion()
{
    return 0;
}

Zynstrument::Zynstrument(audioMasterCallback audioMaster)
    : AudioEffectX(audioMaster, NUM_PROGRAMS, NUM_PARAMS), fft(nullptr), currentProgram(0),
      parameters(nullptr), playingNote(nullptr),
      _lastGeneratedBufferSize(0), _lastSampleFrames(0)
{
    Config::Current().init();

    /* Get the settings from the Config*/
    settings.samplerate = Config::Current().cfg.SampleRate;
    settings.buffersize = Config::Current().cfg.SoundBufferSize;
    settings.oscilsize = Config::Current().cfg.OscilSize;
    settings.alias();

    ctl.Init(&settings);
    ctl.defaults();
    fft = new FFTwrapper(settings.oscilsize);

    parameters = new SUBnoteParameters(&settings, fft);
    parameters->Defaults();

    _tmpoutr = new float[settings.buffersize * 4];
    _tmpoutl = new float[settings.buffersize * 4];

    setNumInputs(0);
    setNumOutputs(2);
    isSynth(true);
    setEditor(new ZynEditor(this));
}

Zynstrument::~Zynstrument()
{
    delete parameters;
    delete fft;
    delete editor;
}

bool Zynstrument::getProgramNameIndexed(VstInt32 /*category*/, VstInt32 index, char *text)
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

void Zynstrument::processReplacing(float ** /*inputs*/, float **outputs, VstInt32 sampleFrames)
{
    if (playingNote == nullptr)
    {
        for (unsigned int i = 0; i < settings.buffersize * 4; i++)
        {
            _tmpoutl[i] = _tmpoutr[i] = 0;
        }
        outputs[0] = &_tmpoutl[0];
        outputs[1] = &_tmpoutr[0];

        return;
    }

    if (_lastGeneratedBufferSize != 0 && _lastSampleFrames != 0)
    {
        auto offset = _lastGeneratedBufferSize - _lastSampleFrames;
        for (unsigned int i = 0; i < offset; i++)
        {
            _tmpoutl[i] = _tmpoutl[static_cast<unsigned int>(sampleFrames) + i];
            _tmpoutr[i] = _tmpoutr[static_cast<unsigned int>(sampleFrames) + i];
        }

        for (unsigned int i = _lastSampleFrames; i < settings.buffersize * 4; i++)
        {
            _tmpoutl[i] = _tmpoutr[i] = 0;
        }
        _lastGeneratedBufferSize = offset;
    }

    while (_lastGeneratedBufferSize < static_cast<unsigned int>(sampleFrames))
    {
        playingNote->noteout(&_tmpoutl[_lastGeneratedBufferSize], &_tmpoutr[_lastGeneratedBufferSize]);
        _lastGeneratedBufferSize += settings.buffersize;
    }

    outputs[0] = &_tmpoutl[0];
    outputs[1] = &_tmpoutr[0];

    _lastSampleFrames = static_cast<unsigned int>(sampleFrames);

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

        auto event = reinterpret_cast<VstMidiEvent *>(ev->events[i]);
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
                    playingNote = new SUBnote(parameters,
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
    if (!audioMaster(nullptr, audioMasterVersion, 0, 0, nullptr, 0))
    {
        return nullptr; // old version
    }

    // Create the AudioEffect
    AudioEffect *effect = createEffectInstance(audioMaster);
    if (!effect)
    {
        return nullptr;
    }

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
