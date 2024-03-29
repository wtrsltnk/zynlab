#ifndef MIDIINPUTMANAGER_H
#define MIDIINPUTMANAGER_H

#include "SafeQueue.h"
#include "ZynSema.h"
#include <set>
#include <string>
#include <zyn.common/IMidiEventHandler.h>
#include <zyn.common/globals.h>

enum class MidiEventTypes
{
    M_NOTE = 1,       // note
    M_CONTROLLER = 2, // controller
    M_PGMCHANGE = 3,  // program change
    M_PRESSURE = 4    // polyphonic aftertouch
};

struct MidiEvent
{
    MidiEvent();
    unsigned int channel; // the midi channel for the event
    MidiEventTypes type;  // type=1 for note, type=2 for controller
    unsigned int num;     // note, controller or program number
    unsigned int value;   // velocity or controller value
    unsigned int time;    // time offset of event (used only in jack->jack case at the moment)
};

class MidiInputManager
{
public:
    static MidiInputManager &CreateInstance(
        IMidiEventHandler *midiEventHandler);

    static MidiInputManager &Instance();

    static void DestroyInstance();

    virtual ~MidiInputManager();

    void PutEvent(
        MidiEvent ev);

    void Flush(
        unsigned int frameStart,
        unsigned int frameStop);

    bool Empty() const;

    bool SetSource(
        std::string const &name);

    std::string GetSource() const;

    friend class EngineManager;

private:
    static MidiInputManager *_instance;

    MidiInputManager(
        IMidiEventHandler *midiEventHandler);

    class MidiInput *GetMidiInput(
        std::string const &name);

    SafeQueue<MidiEvent> _queue;
    mutable ZynSema _work;
    class MidiInput *_current;

    IMidiEventHandler *_midiEventHandler;
};

#endif // MIDIINPUTMANAGER_H
