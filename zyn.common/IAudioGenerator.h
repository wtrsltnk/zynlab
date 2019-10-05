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

    // Audio generation
    virtual void AudioOut(float *outl, float *outr) = 0;
};

#endif  // IAUDIOGENERATOR_H
