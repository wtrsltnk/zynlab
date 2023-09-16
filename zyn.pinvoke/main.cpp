#include "par.h"
#include <fstream>
#include <iostream>
#include <memory.h>
#include <regex>
#include <zyn.common/Config.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.serialization/BankManager.h>
#include <zyn.serialization/SaveToFileSerializer.h>
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

__declspec(dllexport) bool IsChannelEnabled(
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

__declspec(dllexport) unsigned char GetPar(
    Mixer *mixer,
    unsigned char chan,
    const char *id);

__declspec(dllexport) bool GetParBool(
    Mixer *mixer,
    unsigned char chan,
    const char *id);

__declspec(dllexport) float GetParReal(
    Mixer *mixer,
    unsigned char chan,
    const char *id);

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

__declspec(dllexport) void SetSample(
    Mixer *mixer,
    unsigned char chan,
    unsigned char instrumentIndex,
    unsigned char sampleIndex,
    const char *samplePath);

__declspec(dllexport) void SetSampleData(
    Mixer *mixer,
    unsigned char chan,
    unsigned char instrumentIndex,
    unsigned char sampleIndex,
    unsigned int channels,
    unsigned int samplesPerChannel,
    const char *sampleData);

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

bool IsChannelEnabled(
    Mixer *mixer,
    unsigned char chan)
{
    if (mixer == nullptr)
    {
        return false;
    }

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return false;
    }

    return track->Penabled;
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
    // logfile << "NoteOn[" << short(chan) << "]  : " << (int)note << std::endl;

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
    // logfile << "NoteOff[" << short(chan) << "] : " << (int)note << std::endl;

    if (mixer == nullptr)
    {
        return;
    }

    mixer->NoteOff(chan, note);
}

unsigned char GetPar(
    Mixer *mixer,
    unsigned char chan,
    const char *id)
{
    if (mixer == nullptr)
    {
        return 0;
    }

    auto par = GetParById(mixer, chan, id);

    unsigned char value = '\0';

    if (par.byteValue != nullptr)
    {
        return (*par.byteValue);
    }
    else
    {
        logfile << id << " not found as unsigned char" << std::endl;
    }

    return value;
}

bool GetParBool(
    Mixer *mixer,
    unsigned char chan,
    const char *id)
{
    if (mixer == nullptr)
    {
        return false;
    }

    auto par = GetParById(mixer, chan, id);

    bool value = false;

    if (par.boolValue != nullptr)
    {
        return (*par.boolValue);
    }
    else
    {
        logfile << id << " not found as bool" << std::endl;
    }

    return value;
}

float GetParReal(
    Mixer *mixer,
    unsigned char chan,
    const char *id)
{
    if (mixer == nullptr)
    {
        return 0.0f;
    }

    auto par = GetParById(mixer, chan, id);

    float value = 0.0f;

    if (par.floatValue != nullptr)
    {
        return (*par.floatValue);
    }
    else
    {
        logfile << id << " not found as float" << std::endl;
    }

    return value;
}

void SetPar(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    unsigned char value)
{
    if (mixer == nullptr)
    {
        return;
    }

    logfile << "SetPar[" << short(chan) << "] : " << id << ", " << value << std::endl;

    auto par = GetParById(mixer, chan, id);

    if (par.setByteIsSet)
    {
        logfile << "calling setByte function " << std::endl;

        par.setByte(value);
    }
    else if (par.byteValue != nullptr)
    {
        (*par.byteValue) = value;
    }
    else
    {
        logfile << id << " not found as unsigned char" << std::endl;
    }
}

void SetParBool(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    bool value)
{
    if (mixer == nullptr)
    {
        return;
    }

    logfile << "SetParBool[" << short(chan) << "] : " << id << ", " << value << std::endl;

    auto par = GetParById(mixer, chan, id);

    if (par.setBoolIsSet)
    {
        par.setBool(value);
    }
    else if (par.boolValue != nullptr)
    {
        (*par.boolValue) = value;
    }
    else
    {
        logfile << id << " not found as float" << std::endl;
    }
}

void SetParReal(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    float value)
{
    if (mixer == nullptr)
    {
        return;
    }

    logfile << "SetParReal[" << short(chan) << "] : " << id << ", " << value << std::endl;

    auto par = GetParById(mixer, chan, id);

    if (par.setFloatIsSet)
    {
        par.setFloat(value);
    }
    else if (par.floatValue != nullptr)
    {
        (*par.floatValue) = value;
    }
    else
    {
        logfile << id << " not found as float" << std::endl;
    }
}

void SetSample(
    Mixer *mixer,
    unsigned char chan,
    unsigned char instrumentIndex,
    unsigned char sampleIndex,
    const char *samplePath)
{
    logfile << "SetSample[" << int(chan) << "," << int(instrumentIndex) << "," << int(sampleIndex) << "] " << samplePath << std::endl;

    if (mixer == nullptr)
    {
        logfile << "mixer == nullptr" << std::endl;

        return;
    }

    if (instrumentIndex < 0 || instrumentIndex >= NUM_TRACK_INSTRUMENTS)
    {
        logfile << "instrumentIndex < 0 || instrumentIndex >= " << NUM_TRACK_INSTRUMENTS << std::endl;

        return;
    }

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        logfile << "track == nullptr" << std::endl;

        return;
    }

    if (!track->Instruments[instrumentIndex].Psmplenabled)
    {
        logfile << "!track->Instruments[instrumentIndex].Psmplenabled" << std::endl;

        return;
    }

    auto newWavData = WavData::Load(samplePath);

    if (newWavData == nullptr)
    {
        logfile << "newWavData == nullptr" << std::endl;

        return;
    }

    auto sample = track->Instruments[instrumentIndex].smplpars->PwavData.find(sampleIndex);
    if (sample != track->Instruments[instrumentIndex].smplpars->PwavData.end())
    {
        delete sample->second;
        track->Instruments[instrumentIndex].smplpars->PwavData.erase(sample);
    }

    track->Instruments[instrumentIndex].smplpars->PwavData.insert(std::make_pair(sampleIndex, newWavData));
}

void SetSampleData(
    Mixer *mixer,
    unsigned char chan,
    unsigned char instrumentIndex,
    unsigned char sampleIndex,
    unsigned int channels,
    unsigned int samplesPerChannel,
    const char *sampleData)
{
    logfile << "SetSample[" << int(chan) << "," << int(instrumentIndex) << "," << int(sampleIndex) << "] " << std::endl;

    if (mixer == nullptr)
    {
        logfile << "mixer == nullptr" << std::endl;

        return;
    }

    if (instrumentIndex < 0 || instrumentIndex >= NUM_TRACK_INSTRUMENTS)
    {
        logfile << "instrumentIndex < 0 || instrumentIndex >= " << NUM_TRACK_INSTRUMENTS << std::endl;

        return;
    }

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        logfile << "track == nullptr" << std::endl;

        return;
    }

    if (!track->Instruments[instrumentIndex].Psmplenabled)
    {
        logfile << "!track->Instruments[instrumentIndex].Psmplenabled" << std::endl;

        return;
    }

    auto newWavData = new WavData();
    newWavData->channels = channels;
    newWavData->samplesPerChannel = samplesPerChannel;

    if (!WavData::fromBase64(sampleData, newWavData))
    {
        logfile << "WavData::fromBase64 failed" << std::endl;

        return;
    }

    auto sample = track->Instruments[instrumentIndex].smplpars->PwavData.find(sampleIndex);
    if (sample != track->Instruments[instrumentIndex].smplpars->PwavData.end())
    {
        delete sample->second;
        track->Instruments[instrumentIndex].smplpars->PwavData.erase(sample);
    }

    track->Instruments[instrumentIndex].smplpars->PwavData.insert(std::make_pair(sampleIndex, newWavData));
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
        logfile << "bankIndex not valid " << bankIndex << std::endl;

        return 0;
    }

    if (bankIndex >= banks->GetBankCount())
    {
        logfile << "bankIndex not valid " << bankIndex << std::endl;

        return 0;
    }

    auto bank = banks->GetBank(bankIndex);

    if (instrumentIndex < 0)
    {
        logfile << "instrumentIndex not valid " << instrumentIndex << std::endl;

        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    if (instrumentIndex >= BANK_SIZE)
    {
        logfile << "instrumentIndex not valid " << instrumentIndex << std::endl;

        return strcpy_s(buffer, bufferSize, emptyBankName);
    }

    logfile << "getting instrumentPath[" << bankIndex << "][" << instrumentIndex << "]" << std::endl;

    banks->LoadBank(bankIndex);

    auto path = banks->GetInstrumentPath(instrumentIndex);

    return strcpy_s(buffer, bufferSize, path.c_str());
}
