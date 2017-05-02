#ifndef ENGINEMANAGER_H
#define ENGINEMANAGER_H

#include "Engine.h"

#include <list>
#include <string>

class MidiInput;
class AudioOutput;
class AudioOutputManager;
/**Container/Owner of the long lived Engines*/
class EngineManager
{
    friend class Nio;
    static EngineManager* _instance;
    EngineManager(class IMixer* mixer);
public:
    static EngineManager &createInstance(class IMixer* mixer);
    static EngineManager &getInstance();
    static void destroyInstance();
    virtual ~EngineManager();

    /**Gets requested engine
         * @param name case unsensitive name of engine
         * @return pointer to Engine or NULL
         */
    Engine *getEng(std::string name);

    /**Start up defaults*/
    bool start();

    /**Stop all engines*/
    void stop();

    std::list<Engine *> engines;

    //return false on failure
    bool setInDefault(std::string name);
    bool setOutDefault(std::string name);

    //default I/O
    AudioOutput *defaultOut;
    MidiInput   *defaultIn;
};

#endif // ENGINEMANAGER_H
