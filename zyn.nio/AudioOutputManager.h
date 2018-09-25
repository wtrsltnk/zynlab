#ifndef AUDIOOUTPUTMANAGER_H
#define AUDIOOUTPUTMANAGER_H

#include <zyn.common/Stereo.h>
#include <zyn.common/globals.h>

#include <list>
#include <semaphore.h>
#include <string>

class AudioOutput;
class AudioOutputManager
{
    static AudioOutputManager *_instance;

public:
    static AudioOutputManager &createInstance(IMixer *mixer);
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
    AudioOutputManager(IMixer *mixer);
    void addSmps(float *l, float *r);
    unsigned int storedSmps() const { return static_cast<unsigned int>(priBuffCurrent._left - priBuf._left); }
    void removeStaleSmps();

    AudioOutput *currentOut; /**<The current output driver*/

    sem_t requested;

    /**Buffer*/
    Stereo<float *> priBuf;         //buffer for primary drivers
    Stereo<float *> priBuffCurrent; //current array accessor

    float *outl;
    float *outr;
    IMixer *mixer;

    unsigned int stales;
};

#endif // AUDIOOUTPUTMANAGER_H
