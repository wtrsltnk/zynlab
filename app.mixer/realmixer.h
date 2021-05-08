#ifndef REALMIXER_H
#define REALMIXER_H

#include <memory>
#include <mutex>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/IMidiEventHandler.h>
#include <zyn.common/globals.h>
#include <zyn.synth/Controller.h>
#include <zyn.synth/SUBnoteParams.h>
#include <zyn.synth/SynthNote.h>

enum NoteStatus
{
    KEY_OFF,
    KEY_PLAYING,
    KEY_RELASED_AND_SUSTAINED,
    KEY_RELASED
};

struct TrackNote
{
    NoteStatus status;
    int note; //if there is no note playing, the "note"=-1
    int itemsplaying;
    SynthNote *synthNote;
    int time;
};

class RealMixer :
    public IAudioGenerator,
    public IMidiEventHandler
{
    std::unique_ptr<float> _tmpoutl;
    std::unique_ptr<float> _tmpoutr;

    std::mutex _mutex;
    SUBnoteParameters *_params;
    TrackNote _trackNotes[POLIPHONY];
    Controller ctl;

public:
    RealMixer();
    virtual ~RealMixer();

    void Init();

public: // IAudioGenerator implementation
    // Synth settings
    virtual unsigned int SampleRate() const;

    virtual unsigned int BufferSize() const;

    virtual unsigned int BufferSizeInBytes() const;

    // Mutex
    virtual void Lock();

    virtual void Unlock();

    // Audio generation
    virtual void AudioOut(
        float *outl,
        float *outr);

public: // IMidiEventHandler implementation
    // Midi IN
    virtual void NoteOn(
        unsigned char chan,
        unsigned char note,
        unsigned char velocity);

    virtual void NoteOff(
        unsigned char chan,
        unsigned char note);

    virtual void SetController(
        unsigned char chan,
        int type,
        int par);

    virtual void SetProgram(
        unsigned char chan,
        unsigned int pgm);

    virtual void PolyphonicAftertouch(
        unsigned char chan,
        unsigned char note,
        unsigned char velocity);
};

#endif // REALMIXER_H
