#ifndef AUDIOOUTPUTMANAGER_H
#define AUDIOOUTPUTMANAGER_H

#include "../zyn.common/globals.h"
#include "../zyn.common/Stereo.h"

#include <list>
#include <string>
#include <semaphore.h>

class AudioOutput;
class AudioOutputManager
{
    static AudioOutputManager* _instance;
public:
    static AudioOutputManager &createInstance(IMixer* mixer);
    static AudioOutputManager &getInstance();
    static void destroyInstance();
    virtual ~AudioOutputManager();

    /**Execute a tick*/
    const Stereo<float *> nextSample(unsigned int frameSize);

    /**Request a new set of samples
         * @param n number of requested samples (defaults to 1)
         * @return -1 for locking issues 0 for valid request*/
    void requestSamples(unsigned int n = 1);

    /**Gets requested driver
         * @param name case unsensitive name of driver
         * @return pointer to Audio Out or NULL
         */
    AudioOutput *getOut(std::string name);

    bool setSink(std::string name);

    std::string getSink() const;

    friend class EngineManager;
private:
    AudioOutputManager(IMixer* mixer);
    void addSmps(float *l, float *r);
    unsigned int  storedSmps() const { return priBuffCurrent.l - priBuf.l; }
    void removeStaleSmps();

    AudioOutput *currentOut; /**<The current output driver*/

    sem_t requested;

    /**Buffer*/
    Stereo<float *> priBuf;          //buffer for primary drivers
    Stereo<float *> priBuffCurrent; //current array accessor

    float *outl;
    float *outr;
    IMixer* mixer;

    int stales;
};

#endif // AUDIOOUTPUTMANAGER_H
