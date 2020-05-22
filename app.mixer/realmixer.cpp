#include "realmixer.h"

#include <cmath>
#include <memory.h>
#include <zyn.common/Util.h>
#include <zyn.synth/SUBnote.h>

RealMixer::RealMixer()
    : _tmpoutl(std::make_unique<float>(SystemSettings::Instance().buffersize)),
      _tmpoutr(std::make_unique<float>(SystemSettings::Instance().buffersize)),
      _params(nullptr)
{
    for (auto &i : _trackNotes)
    {
        i.status = KEY_OFF;
        i.note = -1;
        i.itemsplaying = 0;
        i.synthNote = nullptr;
        i.time = 0;
    }
    pthread_mutex_init(&_mutex, 0);
}

RealMixer::~RealMixer()
{
    pthread_mutex_destroy(&_mutex);
    if (_params != nullptr)
    {
        delete _params;
    }
}

void RealMixer::Init()
{
    ctl.Init();
    ctl.Defaults();

    _params = new SUBnoteParameters();
    _params->Defaults();
}

unsigned int RealMixer::SampleRate() const
{
    return SystemSettings::Instance().samplerate;
}

unsigned int RealMixer::BufferSize() const
{
    return SystemSettings::Instance().buffersize;
}

unsigned int RealMixer::BufferSizeInBytes() const
{
    return SystemSettings::Instance().bufferbytes;
}

void RealMixer::Lock()
{
    pthread_mutex_lock(&_mutex);
}

void RealMixer::Unlock()
{
    pthread_mutex_unlock(&_mutex);
}

void RealMixer::AudioOut(
    float *outl,
    float *outr)
{
    memset(outl, 0, this->BufferSizeInBytes());
    memset(outr, 0, this->BufferSizeInBytes());

    const float outvol = 0.8f;

    for (unsigned k = 0; k < POLIPHONY; ++k)
    {
        if (_trackNotes[k].status == KEY_OFF)
        {
            continue;
        }

        if (_trackNotes[k].synthNote == nullptr)
        {
            continue;
        }

        auto tmpoutl = _tmpoutl.get();
        auto tmpoutr = _tmpoutr.get();

        _trackNotes[k].time++;
        _trackNotes[k].synthNote->noteout(&tmpoutl[0], &tmpoutr[0]);

        if (_trackNotes[k].synthNote->finished())
        {
            delete _trackNotes[k].synthNote;
            _trackNotes[k].synthNote = nullptr;
            _trackNotes[k].status = KEY_OFF;
            _trackNotes[k].note = -1;
            _trackNotes[k].time = 0;
            _trackNotes[k].itemsplaying = 0;
        }

        for (unsigned int i = 0; i < this->BufferSize(); ++i)
        {
            outl[i] += _tmpoutl.get()[i] * outvol;
            outr[i] += _tmpoutr.get()[i] * outvol;
        }
    }
}

void RealMixer::NoteOn(
    unsigned char chan,
    unsigned char note,
    unsigned char velocity)
{
    int pos = -1;
    for (int i = 0; i < POLIPHONY; ++i)
    {
        if (_trackNotes[i].status == KEY_OFF)
        {
            pos = static_cast<int>(i);
            break;
        }
    }

    if (pos < 0 || pos >= POLIPHONY)
    {
        return;
    }

    unsigned char Pvelsns = 64;

    auto vel = VelF(velocity / 127.0f, Pvelsns);
    auto notebasefreq = 440.0f * powf(2.0f, (note - 69.0f) / 12.0f);

    _trackNotes[pos].status = KEY_PLAYING;
    _trackNotes[pos].note = note;
    _trackNotes[pos].synthNote = new SUBnote(
        _params,
        &ctl,
        notebasefreq,
        vel,
        0,
        note,
        false);
}

void RealMixer::NoteOff(
    unsigned char chan,
    unsigned char note)
{
    for (int i = POLIPHONY - 1; i >= 0; i--) //first note in, is first out if there are same note multiple times
    {
        if ((_trackNotes[i].status == KEY_PLAYING) && (_trackNotes[i].note == note))
        {
            if (ctl.sustain.sustain == 0)
            { //the sustain pedal is not pushed
                _trackNotes[i].status = KEY_RELASED;
            }
            else //the sustain pedal is pushed
            {
                _trackNotes[i].status = KEY_RELASED_AND_SUSTAINED;
            }
        }
    }
}

void RealMixer::SetController(
    unsigned char chan,
    int type,
    int par) {}

void RealMixer::SetProgram(
    unsigned char chan,
    unsigned int pgm) {}

void RealMixer::PolyphonicAftertouch(
    unsigned char chan,
    unsigned char note,
    unsigned char velocity) {}
