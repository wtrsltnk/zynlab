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
    static EngineManager *_instance;
    EngineManager(class IMixer *mixer);

public:
    static EngineManager &CreateInstance(class IMixer *mixer);
    static EngineManager &Instance();
    static void DestroyInstance();
    virtual ~EngineManager();

    /**Gets requested engine
         * @param name case unsensitive name of engine
         * @return pointer to Engine or NULL
         */
    Engine *GetEngine(std::string const &name);

    /**Start up defaults*/
    bool Start();

    /**Stop all engines*/
    void Stop();

    std::list<Engine *> engines;

    //return false on failure
    bool SetDefaultAudioOutput(std::string const &name);
    bool SetDefaultMidiInput(std::string const &name);

    //default I/O
    AudioOutput *defaultOut;
    MidiInput *defaultIn;
};

#endif // ENGINEMANAGER_H
