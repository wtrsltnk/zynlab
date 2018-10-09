#ifndef IAUDIOGENERATOR_H
#define IAUDIOGENERATOR_H

class IAudioGenerator
{
public:
    virtual ~IAudioGenerator();

    // Synth settings
    virtual unsigned int SampleRate() const = 0;
    virtual unsigned int BufferSize() const = 0;
    virtual unsigned int BufferSizeInBytes() const = 0;

    // Mutex
    virtual void Lock() = 0;
    virtual void Unlock() = 0;

    // Midi IN
    virtual void NoteOn(unsigned char chan, unsigned char note, unsigned char velocity) = 0;
    virtual void NoteOff(unsigned char chan, unsigned char note) = 0;
    virtual void SetController(unsigned char chan, int type, int par) = 0;
    virtual void SetProgram(unsigned char chan, unsigned int pgm) = 0;
    virtual void PolyphonicAftertouch(unsigned char chan, unsigned char note, unsigned char velocity) = 0;

    // Audio generation
    virtual void AudioOut(float *outl, float *outr) = 0;
};

#endif  // IAUDIOGENERATOR_H
