#include "MidiInputManager.h"
#include "MidiInput.h"
#include "EngineManager.h"
#include "../zyn.common/globals.h"

#include <iostream>

using namespace std;

ostream &operator<<(ostream &out, const MidiEvent &ev)
{
    switch(ev.type) {
    case MidiEventTypes::M_NOTE:
        out << "MidiNote: note(" << ev.num << ")\n"
            << "          channel(" << ev.channel << ")\n"
            << "          velocity(" << ev.value << ")";
        break;

    case MidiEventTypes::M_CONTROLLER:
        out << "MidiCtl: controller(" << ev.num << ")\n"
            << "         channel(" << ev.channel << ")\n"
            << "         value(" << ev.value << ")";
        break;

    case MidiEventTypes::M_PGMCHANGE:
        out << "PgmChange: program(" << ev.num << ")\n"
            << "           channel(" << ev.channel << ")";
        break;
    }

    return out;
}

MidiEvent::MidiEvent()
    :channel(0), type(MidiEventTypes::M_NOTE), num(0), value(0), time(0)
{}

MidiInputManager *MidiInputManager::_instance = 0;

MidiInputManager &MidiInputManager::createInstance(IMixer* mixer)
{
    if (MidiInputManager::_instance == 0) MidiInputManager::_instance = new MidiInputManager(mixer);

    return *MidiInputManager::_instance;
}


MidiInputManager &MidiInputManager::getInstance()
{
    return *MidiInputManager::_instance;
}


void MidiInputManager::destroyInstance()
{
    if (MidiInputManager::_instance != 0) delete MidiInputManager::_instance;
    MidiInputManager::_instance = 0;
}

MidiInputManager::MidiInputManager(IMixer* mixer)
    :queue(100), mixer(mixer)
{
    current = NULL;
    work.init(PTHREAD_PROCESS_PRIVATE, 0);
}

MidiInputManager::~MidiInputManager()
{
    //lets stop the consumer thread
}

void MidiInputManager::putEvent(MidiEvent ev)
{
    if(queue.push(ev)) //check for error
        cerr << "ERROR: Midi Ringbuffer is FULL" << endl;
    else
        work.post();
}

void MidiInputManager::flush(unsigned frameStart, unsigned frameStop)
{
    MidiEvent ev;
    while(!work.trywait()) {
        queue.peak(ev);
        if(ev.time < (int)frameStart || ev.time > (int)frameStop) {
            //Back out of transaction
            work.post();
            //printf("%d vs [%d..%d]\n",ev.time, frameStart, frameStop);
            break;
        }
        queue.pop(ev);
        //cout << ev << endl;

        switch(ev.type) {
        case MidiEventTypes::M_NOTE:
            //                dump.dumpnote(ev.channel, ev.num, ev.value);

            if(ev.value)
                this->mixer->NoteOn(ev.channel, ev.num, ev.value);
            else
                this->mixer->NoteOff(ev.channel, ev.num);
            break;

        case MidiEventTypes::M_CONTROLLER:
            //                dump.dumpcontroller(ev.channel, ev.num, ev.value);
            this->mixer->SetController(ev.channel, ev.num, ev.value);
            break;

        case MidiEventTypes::M_PGMCHANGE:
            this->mixer->SetProgram(ev.channel, ev.num);
            break;
        case MidiEventTypes::M_PRESSURE:
            this->mixer->PolyphonicAftertouch(ev.channel, ev.num, ev.value);
            break;
        }
    }
}

bool MidiInputManager::empty(void) const
{
    int semvalue = work.getvalue();
    return semvalue <= 0;
}

bool MidiInputManager::setSource(string name)
{
    MidiInput *src = getIn(name);

    if(!src)
        return false;

    if(current)
        current->setMidiEn(false);
    current = src;
    current->setMidiEn(true);

    bool success = current->getMidiEn();

    //Keep system in a valid state (aka with a running driver)
    if(!success)
        (current = getIn("NULL"))->setMidiEn(true);

    return success;
}

string MidiInputManager::getSource() const
{
    if(current)
        return current->name;
    else
        return "ERROR";
}

MidiInput *MidiInputManager::getIn(string name)
{
    EngineManager &eng = EngineManager::getInstance();
    return dynamic_cast<MidiInput *>(eng.getEng(name));
}
