/*
  ZynAddSubFX - a software synthesizer

  globals.h - it contains program settings and the program capabilities
              like number of parts, of effects
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef GLOBALS_H
#define GLOBALS_H

#include <math.h>
#include <mutex>
#include <stdint.h>
#include <string>
#include <vector>

/**
 * The number of harmonics of additive synth
 * This must be smaller than OSCIL_SIZE/2
 */
#define MAX_AD_HARMONICS 128

/**
 * The number of harmonics of substractive
 */
#define MAX_SUB_HARMONICS 64

/*
 * The maximum number of samples that are used for 1 PADsynth instrument(or item)
 */
#define PAD_MAX_SAMPLES 64

/*
 * Number of tracks
 */
#define NUM_MIXER_TRACKS 16

/*
 * Number of tracks
 */
#define DISABLED_MIXER_SOLO -1

/*
 * Number of Midi channes
 */
#define NUM_MIDI_CHANNELS 16

/*
 * Number of keys/notes on a Piano
 */
#define NUM_MIDI_NOTES 88

/*
 * The number of voices of additive synth for a single note
 */
#define NUM_VOICES 8

/*
 * The poliphony (notes)
 */
#define POLIPHONY 60

/*
 * Number of system effects
 */
#define NUM_SYS_EFX 4

/*
 * Number of insertion effects
 */
#define NUM_INS_EFX 8

/*
 * Number of channel's insertion effects
 */
#define NUM_TRACK_EFX 3

/*
 * Maximum number of the instrument on a channel
 */
#define NUM_TRACK_INSTRUMENTS 16

/*
 * How is applied the velocity sensing
 */
#define VELOCITY_MAX_SCALE 8.0f

/*
 * The maximum length of channel's name
 */
#define TRACK_MAX_NAME_LEN 30

/*
 * The maximum number of bands of the equaliser
 */
#define MAX_EQ_BANDS 8
#if (MAX_EQ_BANDS >= 20)
#error "Too many EQ bands in globals.h"
#endif

/*
 * Missing PI definitions when on c++11
 */
#ifndef M_PI
#define M_PI (3.14159265358979323846)
#endif // M_PI

#ifndef M_PI_2
#define M_PI_2 (M_PI * 2.0)
#endif // M_PI_2
/*
 * Maximum filter stages
 */
#define MAX_FILTER_STAGES 5

/*
 * Formant filter (FF) limits
 */
#define FF_MAX_VOWELS 6
#define FF_MAX_FORMANTS 12
#define FF_MAX_SEQUENCE 8

#define LOG_2 0.693147181f
#define PI 3.1415926536f
#define LOG_10 2.302585093f

/*
 * The threshold for the amplitude interpolation used if the amplitude
 * is changed (by LFO's or Envelope's). If the change of the amplitude
 * is below this, the amplitude is not interpolated
 */
#define AMPLITUDE_INTERPOLATION_THRESHOLD 0.0001f

/*
 * How the amplitude threshold is computed
 */
#define ABOVE_AMPLITUDE_THRESHOLD(a, b) ((2.0f * fabs((b) - (a)) / (fabs((b) + (a) + 0.0000000001f))) > \
                                         AMPLITUDE_INTERPOLATION_THRESHOLD)

/*
 * Interpolate Amplitude
 */
#define INTERPOLATE_AMPLITUDE(a, b, x, size) ((a) + ((b) - (a)) * static_cast<float>(x) / static_cast<float>(size))

/*
 * dB
 */
#define dB2rap(dB) ((expf((dB)*LOG_10 / 20.0f)))
#define rap2dB(rap) ((20 * logf(rap) / LOG_10))

#define ZERO(data, size)                         \
    {                                            \
        char *data_ = static_cast<char *>(data); \
        for (int i = 0;                          \
             i < size;                           \
             i++)                                \
            data_[i] = 0;                        \
    }
#define ZEROUNSIGNED(data, size)                                   \
    {                                                              \
        unsigned char *data_ = static_cast<unsigned char *>(data); \
        for (int i = 0;                                            \
             i < size;                                             \
             i++)                                                  \
            data_[i] = 0;                                          \
    }
#define ZERO_float(data, size)                     \
    {                                              \
        float *data_ = static_cast<float *>(data); \
        for (int i = 0;                            \
             i < size;                             \
             i++)                                  \
            data_[i] = 0.0f;                       \
    }

enum ONOFFTYPE
{
    OFF = 0,
    ON = 1
};

enum MidiControllers
{
    C_bankselectmsb = 0,
    C_pitchwheel = 1000,
    C_NULL = 1001,
    C_expression = 11,
    C_panning = 10,
    C_bankselectlsb = 32,
    C_filtercutoff = 74,
    C_filterq = 71,
    C_bandwidth = 75,
    C_modwheel = 1,
    C_fmamp = 76,
    C_volume = 7,
    C_sustain = 64,
    C_allnotesoff = 123,
    C_allsoundsoff = 120,
    C_resetallcontrollers = 121,
    C_portamento = 65,
    C_resonance_center = 77,
    C_resonance_bandwidth = 78,

    C_dataentryhi = 0x06,
    C_dataentrylo = 0x26,
    C_nrpnhi = 99,
    C_nrpnlo = 98
};

enum LegatoMsg
{
    LM_Norm,
    LM_FadeIn,
    LM_FadeOut,
    LM_CatchUp,
    LM_ToNorm
};

//is like i=(int)(floor(f))
#ifdef ASM_F2I_YES
#define F2I(f,                                         \
            i) __asm__ __volatile__("fistpl %0"        \
                                    : "=m"(i)          \
                                    : "t"(f -          \
                                          0.49999999f) \
                                    : "st");
#else
#define F2I(f, i) (i) = ((f > 0) ? (static_cast<int>(f)) : (static_cast<int>(f - 1.0f)));
#endif

#ifndef O_BINARY
#define O_BINARY 0
#endif

//temporary include for synth->{samplerate/buffersize} members
class SystemSettings
{
    SystemSettings();

public:
    static SystemSettings &Instance();
    virtual ~SystemSettings();

    bool SetSampleRate(
        unsigned int rate);

    bool SetBufferSize(
        unsigned int size);

    void SetOscilSize(
        unsigned int size);

    float *denormalkillbuf; /**<the buffer to add noise in order to avoid denormalisation*/

    /**Sampling rate*/
    unsigned int samplerate;

    /**
     * The size of a sound buffer (or the granularity)
     * All internal transfer of sound data use buffer of this size
     * All parameters are constant during this period of time, exception
     * some parameters(like amplitudes) which are linear interpolated.
     * If you increase this you'll ecounter big latencies, but if you
     * decrease this the CPU requirements gets high.
     */
    unsigned int buffersize;

    /**
     * The size of ADnote Oscillator
     * Decrease this => poor quality
     * Increase this => CPU requirements gets high (only at start of the note)
     */
    unsigned int oscilsize;

    /**
     * 1 for left-right swapping
     */
    bool swaplr = false;

    //Alias for above terms
    float samplerate_f;
    float halfsamplerate_f;
    float buffersize_f;
    unsigned int bufferbytes;
    float oscilsize_f;

    inline void alias()
    {
        halfsamplerate_f = (samplerate_f = float(samplerate)) / 2.0f;
        buffersize_f = float(buffersize);
        bufferbytes = buffersize * sizeof(float);
        oscilsize_f = float(oscilsize);
    }
    static float numRandom(); //defined in Util.cpp for now
};

//entries in a bank
#define BANK_SIZE 160

class Track;

class IBankManager
{
public:
    virtual ~IBankManager();

    struct InstrumentBank
    {
        bool operator<(const InstrumentBank &b) const;
        std::string dir;
        std::string name;
        std::vector<std::string> instrumentNames;
    };

    virtual int NewBank(std::string const &newbankdirname) = 0;
    virtual int LoadBank(int index) = 0;
    virtual int GetBankCount() = 0;
    virtual std::vector<char const *> const &GetBankNames() = 0;
    virtual InstrumentBank &GetBank(int index) = 0;

    virtual std::string GetName(unsigned int ninstrument) = 0;
    virtual std::string GetNameNumbered(unsigned int ninstrument) = 0;
    virtual std::string GetInstrumentPath(unsigned int ninstrument) = 0;
    virtual void SetName(unsigned int ninstrument, const std::string &newname, int newslot) = 0;
    virtual bool isPADsynth_used(unsigned int ninstrument) = 0;

    virtual bool EmptySlot(unsigned int ninstrument) = 0;
    virtual void ClearSlot(unsigned int ninstrument) = 0;
    virtual void LoadFromSlot(unsigned int ninstrument, Track *track) = 0;
    virtual void SaveToSlot(unsigned int ninstrument, Track *track) = 0;
    virtual void SwapSlot(unsigned int n1, unsigned int n2) = 0;

    virtual std::string const &GetBankFileTitle() = 0;
    virtual int Locked() = 0;
    virtual void RescanForBanks() = 0;
};

struct vuData
{
    vuData();
    float outpeakl, outpeakr, maxoutpeakl, maxoutpeakr, rmspeakl, rmspeakr;
    int clipped;
};

class IMeter
{
public:
    virtual ~IMeter();

    virtual void SetFakePeak(int instrument, unsigned char velocity) = 0;
    virtual unsigned char GetFakePeak(int instrument) = 0;
    virtual float GetOutPeak(int instrument) = 0;
    virtual void ResetPeaks() = 0;
    virtual vuData GetVuData() = 0;
};

class SimpleNote
{
public:
    SimpleNote();
    SimpleNote(unsigned int n,
               unsigned int v,
               float l,
               unsigned int c);
    virtual ~SimpleNote();

    unsigned int note;
    unsigned int velocity;
    float lengthInSec;
    unsigned int channel;
};

class INoteSource
{
public:
    virtual ~INoteSource();

    virtual std::vector<SimpleNote> GetNotes(unsigned int frameCount, unsigned int sampleRate) = 0;
};

enum class ActiveSynths
{
    Add,
    Sub,
    Pad,
    Smpl,
};

class IMixer
{
public:
    virtual ~IMixer();

    virtual IMeter *GetMeter() = 0;

    // Instruments
    virtual unsigned int GetTrackCount() const = 0;

    virtual Track *GetTrack(
        int index) = 0;

    virtual void EnableTrack(
        int index,
        bool enabled) = 0;

    virtual unsigned char GetVolume() const = 0;

    virtual void SetVolume(
        unsigned char Pvolume_) = 0;

    // Effects
    virtual unsigned char GetSystemEffectVolume(
        int Ppart,
        int Pefx) = 0;

    virtual void SetSystemEffectVolume(
        int Ppart,
        int Pefx,
        unsigned char Pvol) = 0;

    virtual unsigned char GetSystemSendEffectVolume(
        int from,
        int to) = 0;

    virtual void SetSystemSendEffectVolume(
        int from,
        int to,
        unsigned char volume) = 0;

    virtual short int GetTrackIndexForInsertEffect(
        int fx) = 0;

    virtual void SetTrackIndexForInsertEffect(
        int fx,
        short int trackIndex) = 0;

    virtual int GetSystemEffectType(
        int fx) = 0;

    virtual void SetSystemEffectType(
        int fx,
        int type) = 0;

    virtual const char *GetSystemEffectName(
        int fx) = 0;

    virtual int GetInsertEffectType(
        int fx) = 0;

    virtual void SetInsertEffectType(
        int fx,
        int type) = 0;

    virtual const char *GetInsertEffectName(
        int fx) = 0;

    // Mutex
    virtual void Lock() = 0;
    virtual void Unlock() = 0;
    virtual std::mutex &Mutex() = 0;

    // Midi IN
    virtual void NoteOn(unsigned char chan, unsigned char note, unsigned char velocity) = 0;
    virtual void NoteOff(unsigned char chan, unsigned char note) = 0;
    virtual void SetController(unsigned char chan, int type, int par) = 0;
    virtual void SetProgram(unsigned char chan, unsigned int pgm) = 0;
    virtual void PolyphonicAftertouch(unsigned char chan, unsigned char note, unsigned char velocity) = 0;

    virtual void PreviewNote(unsigned int channel, unsigned int note, unsigned int length = 400, unsigned int velocity = 100) = 0;

    virtual INoteSource *GetNoteSource() const = 0;
    virtual void SetNoteSource(INoteSource *source) = 0;

    virtual void PreviewSample(std::string const &filename) = 0;

    // Synth state
    struct
    {
        short int currentTrack = 0;
        unsigned int currentTrackInstrument = 0;
        unsigned int currentVoice = NUM_VOICES;
        ActiveSynths currentSynth = ActiveSynths::Add;
    } State;
};

#endif
