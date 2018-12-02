#ifndef VSTPLUGIN_H
#define VSTPLUGIN_H

#include <aeffeditor.h>
#include <map>
#include <mutex>
#include <zyn.common/globals.h>
#include <zyn.synth/SynthNote.h>

class VstPlugin : public AudioEffectX
{
protected:
    SystemSettings settings;
    std::map<VstInt32, SynthNote *> playingNotes;
    std::mutex _notesMutex;
    float *_tmpoutr;
    float *_tmpoutl;
    float *_mixedOutR;
    float *_mixedOutL;
    unsigned int _lastGeneratedBufferSize;
    unsigned int _lastSampleFrames;

    virtual SynthNote *createNote(VstInt32 note, VstInt32 velocity) = 0;

public:
    VstPlugin(audioMasterCallback audioMaster, VstInt32 numPrograms, VstInt32 numParams);
    virtual ~VstPlugin();

    void processReplacing(float **inputs, float **outputs, VstInt32 sampleFrames);
    VstInt32 processEvents(VstEvents *ev);
};

extern AudioEffect *createEffectInstance(audioMasterCallback audioMaster);

#endif // VSTPLUGIN_H
