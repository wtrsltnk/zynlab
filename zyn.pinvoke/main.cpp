
#include <fstream>
#include <iostream>
#include <memory.h>
#include <zyn.common/Config.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.serialization/SaveToFileSerializer.h>
#include <zyn.serialization/BankManager.h>

extern "C" {
__declspec(dllexport) Mixer *CreateMixer();

__declspec(dllexport)  void DestroyMixer(
    Mixer *mixer);

__declspec(dllexport)  void EnableChannel(
    Mixer *mixer,
    unsigned char chan);

__declspec(dllexport)  void DisableChannel(
    Mixer *mixer,
    unsigned char chan);

__declspec(dllexport)  void AudioOut(
    Mixer *mixer,
    float outl[],
    float outr[]);

__declspec(dllexport) void LoadPresets(
    Mixer *mixer,
    unsigned char chan,
    const char *xmldata);

__declspec(dllexport)  void LoadPresetsFromFile(
    Mixer *mixer,
    unsigned char chan,
    const char *filePath);

__declspec(dllexport)  void LoadPresetFromBank(
    Mixer *mixer,
    unsigned char chan,
    const char *library,
    int slot);

__declspec(dllexport)  void NoteOn(
    Mixer *mixer,
    unsigned char chan,
    unsigned char note,
    unsigned char velocity);

__declspec(dllexport)  void NoteOff(
    Mixer *mixer,
    unsigned char chan,
    unsigned char note);

__declspec(dllexport)  void SetPar(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    unsigned char par);

__declspec(dllexport)  void SetParBool(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    bool value);

__declspec(dllexport)  void SetParReal(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    float value);
}

extern std::ofstream logfile;

//static BankManager banks;

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

bool SwitchBankAndLoadSlot(
    const char *bankName,
    int slotIndex,
    Track *track)
{
    static BankManager banks;

    banks.RescanForBanks();

    const int bankCount = banks.GetBankCount();

    logfile << "bankCount = " << bankCount << std::endl;

    for (int i = 0; i < bankCount; i++)
    {
        const auto bank = banks.GetBank(i);
        logfile << "bank[" << i << "] = " << bank.name << std::endl;

        if (std::string(bankName) == bank.name)
        {
            track->Lock();

            banks.LoadBank(i);
            banks.LoadFromSlot(slotIndex, track);

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

void SetPar(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    unsigned char value)
{
    logfile << "SetPar[" << short(chan) << "] : " << id  << ", " << value << std::endl;
    (void)id;
    (void)value;

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }
}

void SetParBool(
    Mixer *mixer,
    unsigned char chan,
    const char *id,
    bool value)
{
    logfile << "SetParBool[" << short(chan) << "] : " << id  << ", " << value << std::endl;
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
    logfile << "SetParReal[" << short(chan) << "] : " << id  << ", " << value << std::endl;
    (void)id;
    (void)value;

    auto track = mixer->GetTrack(chan);

    if (track == nullptr)
    {
        return;
    }
}
