#ifndef OUTMGR_H
#define OUTMGR_H

#include "zyn.common/globals.h"
#include "zyn.common/Stereo.h"
#include <list>
#include <string>
#include <semaphore.h>

class AudioOut;
class OutMgr
{
    static OutMgr* _instance;
    public:
        static OutMgr &createInstance(IMixer* mixer);
        static OutMgr &getInstance();
        static void destroyInstance();
        virtual ~OutMgr();

        /**Execute a tick*/
        const Stereo<float *> tick(unsigned int frameSize);

        /**Request a new set of samples
         * @param n number of requested samples (defaults to 1)
         * @return -1 for locking issues 0 for valid request*/
        void requestSamples(unsigned int n = 1);

        /**Gets requested driver
         * @param name case unsensitive name of driver
         * @return pointer to Audio Out or NULL
         */
        AudioOut *getOut(std::string name);

        /**Gets the name of the first running driver
         * Deprecated
         * @return if no running output, "" is returned
         */
        std::string getDriver() const;

        bool setSink(std::string name);

        std::string getSink() const;

        class WavEngine * wave;     /**<The Wave Recorder*/
        friend class EngineMgr;
    private:
        OutMgr(IMixer* mixer);
        void addSmps(float *l, float *r);
        unsigned int  storedSmps() const {return priBuffCurrent.l - priBuf.l; }
        void removeStaleSmps();

        AudioOut *currentOut; /**<The current output driver*/

        sem_t requested;

        /**Buffer*/
        Stereo<float *> priBuf;          //buffer for primary drivers
        Stereo<float *> priBuffCurrent; //current array accessor

        float *outl;
        float *outr;
        IMixer* mixer;

        int stales;
};

#endif