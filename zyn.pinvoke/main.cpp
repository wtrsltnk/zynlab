#include <iostream>
#include <zyn.mixer/Mixer.h>
#include <zyn.serialization/LibraryManager.h>

extern "C" Mixer* CreateMixer();

extern "C" void DestroyMixer(
    Mixer* mixer);

extern "C" void AudioOut(
    Mixer* mixer,
    float outl[],
    float outr[]);

extern "C" void NoteOn(
    Mixer* mixer,
    unsigned char chan,
    unsigned char note,
    unsigned char velocity);

extern "C" void NoteOff(
    Mixer* mixer,
    unsigned char chan,
    unsigned char note);

Mixer* CreateMixer()
{
    auto m = new Mixer();
    m->Init();

    std::cout << "SampleRate:        " << m->SampleRate() << std::endl;
    std::cout << "BufferSize:        " << m->BufferSize() << std::endl;
    std::cout << "BufferSizeInBytes: " << m->BufferSizeInBytes() << std::endl;

    return m;
}

void AudioOut(
    Mixer* mixer,
    float outl[],
    float outr[])
{
    if (mixer != nullptr)
    {
        mixer->AudioOut(outl, outr);
    }
}

void NoteOn(
    Mixer* mixer,
    unsigned char chan,
    unsigned char note,
    unsigned char velocity)
{
    if (mixer == nullptr)
    {
        return;
    }

    mixer->NoteOn(chan, note, velocity);
}

void NoteOff(
    Mixer* mixer,
    unsigned char chan,
    unsigned char note)
{
    if (mixer == nullptr)
    {
        return;
    }

    mixer->NoteOff(chan, note);
}

void DestroyMixer(
    Mixer* mixer)
{
    if (mixer != nullptr)
    {
        delete mixer;
        mixer = nullptr;
    }
}
