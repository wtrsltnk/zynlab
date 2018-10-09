#include <memory>
#include <zyn.nio/Nio.h>
#include <zyn.synth/FFTwrapper.h>
#include <zyn.synth/OscilGen.h>
#include <unistd.h>

#define OSCIL_SMP_EXTRA_SAMPLES 5

class Example : public IAudioGenerator
{
    SystemSettings synth;
    FFTwrapper fft;
    Resonance res;
    OscilGen oscil;
    float *samples;

public:
    Example();
    // Synth settings
    virtual unsigned int SampleRate() const { return 44100; }
    virtual unsigned int BufferSize() const { return 256; }
    virtual unsigned int BufferSizeInBytes() const { return 256 * sizeof(float); }

    // Mutex
    virtual void Lock() { }
    virtual void Unlock() { }

    // Midi IN
    virtual void NoteOn(unsigned char chan, unsigned char note, unsigned char velocity) { }
    virtual void NoteOff(unsigned char chan, unsigned char note) { }
    virtual void SetController(unsigned char chan, int type, int par) { }
    virtual void SetProgram(unsigned char chan, unsigned int pgm) { }
    virtual void PolyphonicAftertouch(unsigned char chan, unsigned char note, unsigned char velocity) { }

    // Audio generation
    virtual void AudioOut(float *outl, float *outr);
};

Example::Example()
    : fft(synth.oscilsize), oscil(&fft, &res, &synth), samples(new float[synth.oscilsize + OSCIL_SMP_EXTRA_SAMPLES])
{
    oscil.Defaults();
}

void Example::AudioOut(float *outl, float *outr)
{
    oscil.get(outl, 100);
    memcpy(outr, outr, synth.bufferbytes);
}

int main(int /*argc*/, char * /*argv*/ [])
{
    Example example;

    //Run the Nio system
    if (!Nio::Start(&example))
    {
        return -1;
    }

    Nio::SelectSink("PA");
    Nio::SelectSource("RT");

    sleep(2000);

    return 0;
}
