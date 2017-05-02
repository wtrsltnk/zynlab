#ifndef MIDIINPUTMANAGER_H
#define MIDIINPUTMANAGER_H

#include "ZynSema.h"
#include "SafeQueue.h"
#include "../zyn.common/globals.h"

#include <string>

enum class MidiEventTypes {
    M_NOTE = 1,
    M_CONTROLLER = 2,
    M_PGMCHANGE  = 3,
    M_PRESSURE   = 4
};    //type=1 for note, type=2 for controller, type=3 for program change
//type=4 for polyphonic aftertouch

struct MidiEvent {
    MidiEvent();
    int channel; //the midi channel for the event
    MidiEventTypes type;    //type=1 for note, type=2 for controller
    int num;     //note, controller or program number
    int value;   //velocity or controller value
    int time;    //time offset of event (used only in jack->jack case at the moment)
};

//super simple class to manage the inputs
class MidiInputManager
{
    static MidiInputManager* _instance;
public:
    static MidiInputManager &createInstance(IMixer* mixer);
    static MidiInputManager &getInstance();
    static void destroyInstance();
    ~MidiInputManager();

    void putEvent(MidiEvent ev);

    /**Flush the Midi Queue*/
    void flush(unsigned frameStart, unsigned frameStop);

    bool empty() const;

    bool setSource(std::string name);

    std::string getSource() const;

    friend class EngineManager;
private:
    MidiInputManager(IMixer* mixer);
    class MidiInput *getIn(std::string name);
    SafeQueue<MidiEvent> queue;
    mutable ZynSema work;
    class MidiInput * current;

    /**the link to the rest of zyn*/
    IMixer* mixer;
};

#endif // MIDIINPUTMANAGER_H
