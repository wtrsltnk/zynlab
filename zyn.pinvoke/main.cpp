
#include <fstream>
#include <iostream>
#include <memory.h>
#include <regex>
#include <zyn.common/Config.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.serialization/BankManager.h>
#include <zyn.serialization/SaveToFileSerializer.h>
#include <zyn.synth/ADnoteParams.h>
#include <zyn.synth/PADnoteParams.h>
#include <zyn.synth/SUBnoteParams.h>
#include <zyn.synth/SampleNoteParams.h>

extern "C" {
__declspec(dllexport) Mixer *CreateMixer();

__declspec(dllexport) void DestroyMixer(
    Mixer *mixer);

__declspec(dllexport) void EnableChannel(
    Mixer *mixer,
    unsigned char chan);

__declspec(dllexport) void DisableChannel(
    Mixer *mixer,
    unsigned char chan);

__declspec(dllexport) void AudioOut(
    Mixer *mixer,
    float outl[],
    float outr[]);

__declspec(dllexport) void LoadPresets(
    Mixer *mixer,
    unsigned char chan,
    const char *xmldata);

__declspec(dllexport) void LoadPresetsFromFile(
    Mixer *mixer,
    unsigned char chan,
    const char *filePath);

__declspec(dllexport) void LoadPresetFromBank(
    Mixer *mixer,
    unsigned char chan,
    const char *library,
    int slot);

__declspec(dllexport) void NoteOn(
    Mixer *mixer,
    unsigned char chan,
    unsigned char note,
    unsigned char velocity);

__declspec(dllexport) void NoteOff(
    Mixer *mixer,
    unsigned char chan,
    unsigned char note);

__declspec(dllexport) void SetPar(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    unsigned char par);

__declspec(dllexport) void SetParBool(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    bool value);

__declspec(dllexport) void SetParReal(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    float value);

__declspec(dllexport) int GetBankCount();

__declspec(dllexport) int GetBankName(
    int bankIndex,
    char *buffer,
    int bufferSize);

__declspec(dllexport) int GetBankPath(
    int bankIndex,
    char *buffer,
    int bufferSize);

__declspec(dllexport) int GetInstrumentCount(
    int bankIndex);

__declspec(dllexport) int GetInstrumentName(
    int bankIndex,
    int instrumentIndex,
    char *buffer,
    int bufferSize);

__declspec(dllexport) int GetInstrumentPath(
    int bankIndex,
    int instrumentIndex,
    char *buffer,
    int bufferSize);
}

extern std::ofstream logfile;

// static BankManager banks;

Mixer *CreateMixer()
{
    Config::Current().init();

    if (!logfile.is_open())
    {
        logfile.open("c:\\temp\\log.txt", std::ios::out);
    }

    auto m = new Mixer();
    m->Init();

    logfile << "CreateMixer" << std::endl;
    logfile << "SampleRate:        " << m->SampleRate() << std::endl;
    logfile << "BufferSize:        " << m->BufferSize() << std::endl;
    logfile << "BufferSizeInBytes: " << m->BufferSizeInBytes() << std::endl;

    return m;
}

void DestroyMixer(
    Mixer *mixer)
{
    logfile << "DestroyMixer" << std::endl;
    if (logfile.is_open())
    {
        logfile.close();
    }

    if (mixer != nullptr)
    {
        delete mixer;
    }

    Config::Current().save();
}

void EnableChannel(
    Mixer *mixer,
    unsigned char chan)
{
    if (mixer == nullptr)
    {
        return;
    }

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }

    if (!track->Penabled)
    {
        mixer->EnableTrack(chan, true);
    }
}

void DisableChannel(
    Mixer *mixer,
    unsigned char chan)
{
    if (mixer == nullptr)
    {
        return;
    }

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }

    if (track->Penabled)
    {
        track->Lock();
        track->AllNotesOff();
        mixer->Unlock();

        mixer->EnableTrack(chan, false);
    }
}

void AudioOut(
    Mixer *mixer,
    float outl[],
    float outr[])
{
    if (mixer == nullptr)
    {
        return;
    }

    mixer->Lock();
    mixer->AudioOut(outl, outr);
    mixer->Unlock();
}

void LoadPresetsFromFile(
    Mixer *mixer,
    unsigned char chan,
    const char *filePath)
{
    if (mixer == nullptr)
    {
        return;
    }

    auto track = mixer->GetTrack(int(chan));

    if (track == nullptr)
    {
        return;
    }

    track->Lock();

    SaveToFileSerializer().LoadTrack(track, filePath);

    track->Unlock();

    track->ApplyParameters();
}

void LoadPresets(
    Mixer *mixer,
    unsigned char chan,
    const char *xmldata)
{
    logfile << "LoadPresets[" << short(chan) << "]  : " << std::endl;

    if (mixer == nullptr)
    {
        return;
    }

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }

    if (track->Penabled == false)
    {
        mixer->EnableTrack(chan, true);
    }

    track->Lock();

    SaveToFileSerializer().LoadTrackFromData(track, xmldata);

    track->Unlock();

    track->ApplyParameters();
}

IBankManager *GetBanks()
{
    static bool banksAreScanned = false;
    static BankManager banks;

    if (!banksAreScanned)
    {
        banks.RescanForBanks();
    }

    return &banks;
}

bool SwitchBankAndLoadSlot(
    const char *bankName,
    int slotIndex,
    Track *track)
{
    auto banks = GetBanks();

    const int bankCount = banks->GetBankCount();

    logfile << "bankCount = " << bankCount << std::endl;

    for (int i = 0; i < bankCount; i++)
    {
        const auto bank = banks->GetBank(i);
        logfile << "bank[" << i << "] = " << bank.name << std::endl;

        if (std::string(bankName) == bank.name)
        {
            track->Lock();

            banks->LoadBank(i);
            banks->LoadFromSlot(slotIndex, track);

            track->Unlock();

            track->ApplyParameters();

            return true;
        }
    }

    return false;
}

void LoadPresetFromBank(
    Mixer *mixer,
    unsigned char chan,
    const char *library,
    int slot)
{
    logfile << "LoadPresetFromBank[" << short(chan) << "]  : " << library << ", " << slot << std::endl;

    if (mixer == nullptr)
    {
        return;
    }

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }

    if (track->Penabled == false)
    {
        mixer->EnableTrack(chan, true);
    }

    if (!SwitchBankAndLoadSlot(library, slot, track))
    {
        logfile << "Failed to switch to " << library << " and load slot " << slot << std::endl;
    }
}

void NoteOn(
    Mixer *mixer,
    unsigned char chan,
    unsigned char note,
    unsigned char velocity)
{
    logfile << "NoteOn[" << short(chan) << "]  : " << (int)note << std::endl;
    if (mixer == nullptr)
    {
        return;
    }

    mixer->NoteOn(chan, note, velocity);
}

void NoteOff(
    Mixer *mixer,
    unsigned char chan,
    unsigned char note)
{
    logfile << "NoteOff[" << short(chan) << "] : " << (int)note << std::endl;
    if (mixer == nullptr)
    {
        return;
    }

    mixer->NoteOff(chan, note);
}

#define IS_PAR(id, name) std::string(id).substr(0, std::string(#name).length()) == #name

const char *TRACK_ID = "Track";
const char *INSTRUMENTS_ID = "Instruments";
const char *EFFECTS_ID = "Effects";

struct sPar
{
    unsigned char *byteValue = nullptr;
    char *charValue = nullptr;
    float *floatValue = nullptr;
    bool *boolValue = nullptr;
};

void SetLfoParametersPar(
    LFOParams *params,
    const char *id,
    unsigned char value)
{
    logfile << "SetLfoParametersPar " << id << std::endl;

    if (IS_PAR(id, Pfreq))
    {
        params->Pfreq = value;
    }
    else if (IS_PAR(id, Pintensity))
    {
        params->Pintensity = value;
    }
    else if (IS_PAR(id, Pstartphase))
    {
        params->Pstartphase = value;
    }
    else if (IS_PAR(id, PLFOtype))
    {
        params->PLFOtype = value;
    }
    else if (IS_PAR(id, Prandomness))
    {
        params->Prandomness = value;
    }
    else if (IS_PAR(id, Pfreqrand))
    {
        params->Pfreqrand = value;
    }
    else if (IS_PAR(id, Pdelay))
    {
        params->Pdelay = value;
    }
    else if (IS_PAR(id, Pcontinous))
    {
        params->Pcontinous = value;
    }
    else if (IS_PAR(id, Pstretch))
    {
        params->Pstretch = value;
    }
}

void SetEnvelopePar(
    EnvelopeParams *params,
    const char *id,
    unsigned char value)
{
    logfile << "SetEnvelopePar " << id << std::endl;

    if (IS_PAR(id, PAttackTime))
    {
        params->PA_dt = value;
    }
    else if (IS_PAR(id, PAttackValue))
    {
        params->PA_val = value;
    }
    else if (IS_PAR(id, PDecayTime))
    {
        params->PD_dt = value;
    }
    else if (IS_PAR(id, PDecayValue))
    {
        params->PD_val = value;
    }
    else if (IS_PAR(id, PSustainValue))
    {
        params->PS_val = value;
    }
    else if (IS_PAR(id, PReleaseTime))
    {
        params->PR_dt = value;
    }
    else if (IS_PAR(id, PReleaseValue))
    {
        params->PR_val = value;
    }
}

void SetFilterPar(
    FilterParams *params,
    const char *id,
    unsigned char value)
{
    logfile << "SetFilterPar " << id << std::endl;

    if (IS_PAR(id, Pcategory))
    {
        params->Pcategory = value;
    }
    else if (IS_PAR(id, Pcenterfreq))
    {
        params->Pcenterfreq = value;
    }
    else if (IS_PAR(id, Pformantslowness))
    {
        params->Pformantslowness = value;
    }
    else if (IS_PAR(id, Pfreq))
    {
        params->Pfreq = value;
    }
    else if (IS_PAR(id, Pfreqtrack))
    {
        params->Pfreqtrack = value;
    }
    else if (IS_PAR(id, Pgain))
    {
        params->Pgain = value;
    }
    else if (IS_PAR(id, Pnumformants))
    {
        params->Pnumformants = value;
    }
    else if (IS_PAR(id, Poctavesfreq))
    {
        params->Poctavesfreq = value;
    }
    else if (IS_PAR(id, Pq))
    {
        params->Pq = value;
    }
    else if (IS_PAR(id, Pstages))
    {
        params->Pstages = value;
    }
    else if (IS_PAR(id, Ptype))
    {
        params->Ptype = value;
    }
    else if (IS_PAR(id, Pvowelclearness))
    {
        params->Pvowelclearness = value;
    }
}

bool SetAbstractSynthPar(
    AbstractNoteParameters *params,
    const char *id,
    unsigned char value)
{
    if (IS_PAR(id, Pvolume))
    {
        params->PVolume = value;

        return true;
    }
    else if (IS_PAR(id, Ppanning))
    {
        params->PPanning = value;

        return true;
    }
    else if (IS_PAR(id, PAmpVelocityScaleFunction))
    {
        params->PAmpVelocityScaleFunction = value;

        return true;
    }
    else if (IS_PAR(id, Pfixedfreq))
    {
        params->Pfixedfreq = value;

        return true;
    }
    else if (IS_PAR(id, PfixedfreqET))
    {
        params->PfixedfreqET = value;

        return true;
    }
    else if (IS_PAR(id, PDetune))
    {
        params->PDetune = value;

        return true;
    }
    else if (IS_PAR(id, PCoarseDetune))
    {
        params->PCoarseDetune = value;

        return true;
    }
    else if (IS_PAR(id, PDetuneType))
    {
        params->PDetuneType = value;

        return true;
    }
    else if (IS_PAR(id, PFreqEnvelopeEnabled))
    {
        params->PFreqEnvelopeEnabled = value;

        return true;
    }
    else if (IS_PAR(id, FrequencyEnvelope))
    {
        SetEnvelopePar(params->FreqEnvelope, id, value);

        return true;
    }
    else if (IS_PAR(id, PFreqLfoEnabled))
    {
        params->PFreqLfoEnabled = value;

        return true;
    }
    else if (IS_PAR(id, FrequencyLfo))
    {
        SetLfoParametersPar(params->FreqLfo, id, value);

        return true;
    }
    else if (IS_PAR(id, PBandwidth))
    {
        params->PBandwidth = value;

        return true;
    }
    else if (IS_PAR(id, PAmpEnvelopeEnabled))
    {
        params->PAmpEnvelopeEnabled = value;

        return true;
    }
    else if (IS_PAR(id, AmpEnvelope))
    {
        SetEnvelopePar(params->AmpEnvelope, id, value);

        return true;
    }
    else if (IS_PAR(id, GlobalFilter))
    {
        SetFilterPar(params->GlobalFilter, id, value);

        return true;
    }
    else if (IS_PAR(id, PFilterVelocityScale))
    {
        params->PFilterVelocityScale = value;

        return true;
    }
    else if (IS_PAR(id, PFilterVelocityScaleFunction))
    {
        params->PFilterVelocityScaleFunction = value;

        return true;
    }
    else if (IS_PAR(id, FilterEnvelope))
    {
        SetEnvelopePar(params->FilterEnvelope, id, value);

        return true;
    }

    return false;
}

void SetAddSynthPar(
    ADnoteParameters *params,
    const char *id,
    unsigned char value)
{
    if (SetAbstractSynthPar(params, id, value))
    {
        return;
    }

    logfile << "SetAddSynthPar " << id << std::endl;

    if (IS_PAR(id, PPunchStrength))
    {
        params->PPunchStrength = value;
    }
    else if (IS_PAR(id, PPunchTime))
    {
        params->PPunchTime = value;
    }
    else if (IS_PAR(id, PPunchStretch))
    {
        params->PPunchStretch = value;
    }
    else if (IS_PAR(id, PPunchVelocitySensing))
    {
        params->PPunchVelocitySensing = value;
    }
    else if (IS_PAR(id, AmpLfo))
    {
        SetLfoParametersPar(params->AmpLfo, id, value);
    }
    else if (IS_PAR(id, AmpEnvelope))
    {
        SetEnvelopePar(params->AmpEnvelope, id, value);
    }
    else if (IS_PAR(id, FilterLfo))
    {
        SetLfoParametersPar(params->FilterLfo, id, value);
    }
    else if (IS_PAR(id, FilterEnvelope))
    {
        SetEnvelopePar(params->FilterEnvelope, id, value);
    }
    else if (IS_PAR(id, FreqLfo))
    {
        SetLfoParametersPar(params->FreqLfo, id, value);
    }
    else if (IS_PAR(id, FreqEnvelope))
    {
        SetEnvelopePar(params->FreqEnvelope, id, value);
    }
    else if (IS_PAR(id, Voices))
    {
    }
}

void SetSubSynthPar(
    SUBnoteParameters *params,
    const char *id,
    unsigned char value)
{
    if (SetAbstractSynthPar(params, id, value))
    {
        return;
    }

    if (IS_PAR(id, PBandWidthEnvelopeEnabled))
    {
        params->PBandWidthEnvelopeEnabled = value;
    }
    else if (IS_PAR(id, BandWidthEnvelope))
    {
        SetEnvelopePar(params->BandWidthEnvelope, id, value);
    }
    else if (IS_PAR(id, PGlobalFilterEnabled))
    {
        params->PGlobalFilterEnabled = value;
    }
    else if (IS_PAR(id, POvertoneSpread.type))
    {
        params->POvertoneSpread.type = value;
    }
    else if (IS_PAR(id, POvertoneSpread.par1))
    {
        params->POvertoneSpread.par1 = value;
    }
    else if (IS_PAR(id, POvertoneSpread.par2))
    {
        params->POvertoneSpread.par2 = value;
    }
    else if (IS_PAR(id, POvertoneSpread.par3))
    {
        params->POvertoneSpread.par3 = value;
    }
    else if (IS_PAR(id, Pnumstages))
    {
        params->Pnumstages = value;
    }
    else if (IS_PAR(id, Phmagtype))
    {
        params->Phmagtype = value;
    }
    else if (IS_PAR(id, Pbwscale))
    {
        params->Pbwscale = value;
    }
    else if (IS_PAR(id, Pstart))
    {
        params->Pstart = value;
    }
    else if (IS_PAR(id, POvertoneFreqMult))
    {
        std::cmatch m;
        std::regex_search(id, m, std::regex("POvertoneFreqMult(\\[([0-9]+)\\]\\)"));

        if (m.empty())
        {
            return;
        }

        auto index = std::atoi(m[2].str().c_str());

        if (index < 0 || index >= MAX_SUB_HARMONICS)
        {
            return;
        }

        params->POvertoneFreqMult[index] = value / 127.0f;
    }
    else if (IS_PAR(id, Phmag))
    {
        std::cmatch m;
        std::regex_search(id, m, std::regex("Phmag(\\[([0-9]+)\\]\\)"));

        if (m.empty())
        {
            return;
        }

        auto index = std::atoi(m[2].str().c_str());

        if (index < 0 || index >= MAX_SUB_HARMONICS)
        {
            return;
        }

        params->Phmag[index] = value;
    }
    else if (IS_PAR(id, Phrelbw))
    {
        std::cmatch m;
        std::regex_search(id, m, std::regex("Phrelbw(\\[([0-9]+)\\]\\)"));

        if (m.empty())
        {
            return;
        }

        auto index = std::atoi(m[2].str().c_str());

        if (index < 0 || index >= MAX_SUB_HARMONICS)
        {
            return;
        }

        params->Phrelbw[index] = value;
    }
}

void SetPadSynthPar(
    PADnoteParameters *params,
    const char *id,
    unsigned char value)
{
    if (SetAbstractSynthPar(params, id, value))
    {
        return;
    }
}

void SetSampleSynthPar(
    SampleNoteParameters *params,
    const char *id,
    unsigned char value)
{
    if (SetAbstractSynthPar(params, id, value))
    {
        return;
    }
}

void SetInstrumentPar(
    Track *track,
    const char *id,
    unsigned char value)
{
    std::cmatch m;
    std::regex_search(id, m, std::regex("^(\\[([0-9]+)\\]\\.)"));

    if (m.empty())
    {
        return;
    }

    auto index = std::atoi(m[2].str().c_str());

    if (index < 0 || index >= NUM_TRACK_INSTRUMENTS)
    {
        return;
    }

    auto relativeId = id + m[1].str().size();

    if (IS_PAR(relativeId, AddSynth))
    {
        SetAddSynthPar(track->Instruments[index].adpars, relativeId + std::string("AddSynth.").length(), value);
    }
    else if (IS_PAR(relativeId, SubSynth))
    {
        SetSubSynthPar(track->Instruments[index].subpars, relativeId + std::string("SubSynth.").length(), value);
    }
    else if (IS_PAR(relativeId, PadSynth))
    {
        SetPadSynthPar(track->Instruments[index].padpars, relativeId + std::string("PadSynth.").length(), value);
    }
    else if (IS_PAR(relativeId, SampleSynth))
    {
        SetSampleSynthPar(track->Instruments[index].smplpars, relativeId + std::string("SmplSynth.").length(), value);
    }
}

void SetEffectPar(
    Track *track,
    const char *id,
    unsigned char value)
{
    std::cmatch m;
    std::regex_search(id, m, std::regex("^(\\[([0-9]+)\\]\\.)"));

    if (m.empty())
    {
        return;
    }

    auto effectIndex = std::atoi(m[2].str().c_str());

    if (effectIndex < 0 || effectIndex >= NUM_TRACK_EFX)
    {
        return;
    }

    auto effect = track->partefx[effectIndex];

    auto relativeId = id + m[1].str().size();

    std::regex_search(relativeId, m, std::regex("^Parameters(\\[([0-9]+)\\])$"));

    if (m.empty())
    {
        return;
    }

    auto parameterIndex = std::atoi(m[2].str().c_str());

    if (parameterIndex < 0)
    {
        return;
    }

    effect->seteffectpar(parameterIndex, value);
}

void SetTrackPar(
    Track *track,
    const char *id,
    unsigned char value)
{
    logfile << "setting " << id << " to  : " << int(value) << std::endl;

    if (IS_PAR(id, Pvolume))
    {
        track->SetVolume(value);
    }
    else if (IS_PAR(id, Ppanning))
    {
        track->setPpanning(value);
    }
    else if (IS_PAR(id, Pvelsns))
    {
        track->Pvelsns = value;
    }
    else if (IS_PAR(id, Pveloffs))
    {
        track->Pveloffs = value;
    }
    else if (IS_PAR(id, Instruments))
    {
        SetInstrumentPar(track, id + std::string(INSTRUMENTS_ID).length(), value);
    }
    else if (IS_PAR(id, Effects))
    {
        SetEffectPar(track, id + std::string(EFFECTS_ID).length(), value);
    }
}

void SetPar(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    unsigned char value)
{
    logfile << "SetPar[" << short(chan) << "] : " << id << ", " << value << std::endl;

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }

    auto const trackId = std::string(id).substr(0, std::string(TRACK_ID).length());

    if (trackId != TRACK_ID)
    {
        logfile << "id not valid : " << id << " (" << trackId << ")" << std::endl;

        return;
    }

    SetTrackPar(track, id + trackId.length() + 1, value); // the +1 is for the dot
}

void SetParBool(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    bool value)
{
    logfile << "SetParBool[" << short(chan) << "] : " << id << ", " << value << std::endl;
    (void)id;
    (void)value;

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }
}

void SetParReal(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    float value)
{
    logfile << "SetParReal[" << short(chan) << "] : " << id << ", " << value << std::endl;
    (void)id;
    (void)value;

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }

    if (std::string(id).substr(0, sizeof(TRACK_ID)) != TRACK_ID)
    {
        logfile << "id not valid : " << id << std::endl;

        return;
    }

    auto relativeId = id + sizeof(TRACK_ID) - 1;

    auto floatValue = (unsigned char)(127 * value);
    if (IS_PAR(relativeId, Pvolume))
    {
        track->SetVolume(floatValue);
    }
    else if (IS_PAR(relativeId, Ppanning))
    {
        track->setPpanning(floatValue);
    }
    else if (IS_PAR(relativeId, Pvelsns))
    {
        track->Pvelsns = floatValue;
    }
    else if (IS_PAR(relativeId, Pveloffs))
    {
        track->Pveloffs = floatValue;
    }
}

int GetBankCount()
{
    auto banks = GetBanks();

    return banks->GetBankCount();
}

const char *emptyBankName = "";

int GetBankName(
    int bankIndex,
    char *buffer,
    int bufferSize)
{
    auto banks = GetBanks();

    if (bankIndex < 0)
    {
        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    if (bankIndex >= banks->GetBankCount())
    {
        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    auto name = banks->GetBank(bankIndex).name;

    return strcpy_s(buffer, bufferSize, name.c_str());
}

int GetBankPath(
    int bankIndex,
    char *buffer,
    int bufferSize)
{
    auto banks = GetBanks();

    if (bankIndex < 0)
    {
        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    if (bankIndex >= banks->GetBankCount())
    {
        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    auto dir = banks->GetBank(bankIndex).dir;

    return strcpy_s(buffer, bufferSize, dir.c_str());
}

int GetInstrumentCount(
    int bankIndex)
{
    auto banks = GetBanks();

    if (bankIndex < 0)
    {
        return 0;
    }

    if (bankIndex >= banks->GetBankCount())
    {
        return 0;
    }

    return BANK_SIZE;
}

int GetInstrumentName(
    int bankIndex,
    int instrumentIndex,
    char *buffer,
    int bufferSize)
{
    auto banks = GetBanks();

    if (bankIndex < 0)
    {
        return 0;
    }

    if (bankIndex >= banks->GetBankCount())
    {
        return 0;
    }

    auto bank = banks->GetBank(bankIndex);

    if (instrumentIndex < 0)
    {
        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    if (instrumentIndex >= BANK_SIZE)
    {
        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    logfile << "getting instrumentName[" << bankIndex << "][" << instrumentIndex << "]" << std::endl;

    banks->LoadBank(bankIndex);

    auto name = banks->GetName(instrumentIndex);

    return strcpy_s(buffer, bufferSize, name.c_str());
}

int GetInstrumentPath(
    int bankIndex,
    int instrumentIndex,
    char *buffer,
    int bufferSize)
{
    auto banks = GetBanks();

    if (bankIndex < 0)
    {
        return 0;
    }

    if (bankIndex >= banks->GetBankCount())
    {
        return 0;
    }

    auto bank = banks->GetBank(bankIndex);

    if (instrumentIndex < 0)
    {
        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    if (instrumentIndex >= bank.instrumentNames.size())
    {
        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    logfile << "getting instrumentPath[" << bankIndex << "][" << instrumentIndex << "]" << std::endl;

    banks->LoadBank(bankIndex);

    auto path = banks->GetInstrumentPath(instrumentIndex);

    return strcpy_s(buffer, bufferSize, path.c_str());
}
