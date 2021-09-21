#ifndef AUDIOOUTPUTMANAGER_H
#define AUDIOOUTPUTMANAGER_H

#include <list>
#include <string>
#include <zyn.common/IAudioGenerator.h>
#include <zyn.common/Stereo.h>
#include <zyn.common/globals.h>

class AudioOutput;
class WavEngine;

class AudioOutputManager
{
    static AudioOutputManager *_instance;

public:
    static AudioOutputManager &createInstance(IAudioGenerator *audioGenerator);
    static AudioOutputManager &getInstance();
    static void destroyInstance();
    virtual ~AudioOutputManager();

    /**Execute a tick*/
    const Stereo<float *> NextSample(unsigned int frameSize);

    /**Gets requested driver
         * @param name case unsensitive name of driver
         * @return pointer to Audio Out or NULL
         */
    AudioOutput *GetAudioOutput(std::string const &name);

    bool SetSink(std::string const &name);

    std::string GetSink() const;

    friend class EngineManager;

private:
    AudioOutputManager(IAudioGenerator *audioGenerator);
    unsigned int storedSmps() const { return static_cast<unsigned int>(priBuffCurrent._left - priBuf._left); }
    void removeStaleSmps();

    AudioOutput *currentOut; /**<The current output driver*/

    /**Buffer*/
    Stereo<float *> priBuf;         //buffer for primary drivers
    Stereo<float *> priBuffCurrent; //current array accessor

    float *outl;
    float *outr;
    IAudioGenerator *_audioGenerator;

    unsigned int stales;
};

#endif // AUDIOOUTPUTMANAGER_H
