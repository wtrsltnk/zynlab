#include "MidiInputManager.h"
#include "EngineManager.h"
#include "MidiInput.h"
#include <iostream>
#include <utility>

std::ostream &operator<<(std::ostream &out, const MidiEvent &ev)
{
    switch (ev.type)
    {
        case MidiEventTypes::M_NOTE:
        {
            out << "MidiNote: note(" << ev.num << ")\n"
                << "          channel(" << ev.channel << ")\n"
                << "          velocity(" << ev.value << ")";
            break;
        }
        case MidiEventTypes::M_CONTROLLER:
        {
            out << "MidiCtl: controller(" << ev.num << ")\n"
                << "         channel(" << ev.channel << ")\n"
                << "         value(" << ev.value << ")";
            break;
        }
        case MidiEventTypes::M_PGMCHANGE:
        {
            out << "PgmChange: program(" << ev.num << ")\n"
                << "           channel(" << ev.channel << ")";
            break;
        }
        case MidiEventTypes::M_PRESSURE:
        {
            out << "Pressure: program(" << ev.num << ")\n"
                << "          channel(" << ev.channel << ")";
            break;
        }
    }

    return out;
}

MidiEvent::MidiEvent()
    : channel(0), type(MidiEventTypes::M_NOTE), num(0), value(0), time(0)
{}

MidiInputManager *MidiInputManager::_instance = nullptr;

MidiInputManager &MidiInputManager::CreateInstance(IMidiEventHandler *midiEventHandler)
{
    if (MidiInputManager::_instance == nullptr)
    {
        MidiInputManager::_instance = new MidiInputManager(midiEventHandler);
    }

    return *MidiInputManager::_instance;
}

MidiInputManager &MidiInputManager::Instance()
{
    return *MidiInputManager::_instance;
}

void MidiInputManager::DestroyInstance()
{
    delete MidiInputManager::_instance;
    MidiInputManager::_instance = nullptr;
}

MidiInputManager::MidiInputManager(IMidiEventHandler *midiEventHandler)
    : _queue(100), _midiEventHandler(midiEventHandler)
{
    _current = nullptr;
    _work.init(PTHREAD_PROCESS_PRIVATE, 0);
}

MidiInputManager::~MidiInputManager() = default;

void MidiInputManager::PutEvent(MidiEvent ev)
{
    if (_queue.push(ev))
    {
        std::cerr << "ERROR: Midi Ringbuffer is FULL" << std::endl;
        return;
    }

    _work.post();
}

void MidiInputManager::Flush(unsigned int frameStart, unsigned int frameStop)
{
    if (Empty())
    {
        return;
    }

    MidiEvent ev;
    while (!_work.trywait())
    {
        _queue.peak(ev);
        if (ev.time < frameStart || ev.time > frameStop)
        {
            //Back out of transaction
            _work.post();
            break;
        }
        _queue.pop(ev);

        switch (ev.type)
        {
            case MidiEventTypes::M_NOTE:
            {
                if (ev.value)
                {
                    this->_midiEventHandler->NoteOn(
                        static_cast<unsigned char>(ev.channel),
                        static_cast<unsigned char>(ev.num),
                        static_cast<unsigned char>(ev.value));
                }
                else
                {
                    this->_midiEventHandler->NoteOff(
                        static_cast<unsigned char>(ev.channel),
                        static_cast<unsigned char>(ev.num));
                }
                break;
            }
            case MidiEventTypes::M_CONTROLLER:
            {
                this->_midiEventHandler->SetController(
                    static_cast<unsigned char>(ev.channel),
                    static_cast<char>(ev.num),
                    static_cast<char>(ev.value));
                break;
            }
            case MidiEventTypes::M_PGMCHANGE:
            {
                this->_midiEventHandler->SetProgram(
                    static_cast<unsigned char>(ev.channel), ev.num);
                break;
            }
            case MidiEventTypes::M_PRESSURE:
            {
                this->_midiEventHandler->PolyphonicAftertouch(
                    static_cast<unsigned char>(ev.channel),
                    static_cast<unsigned char>(ev.num),
                    static_cast<unsigned char>(ev.value));
                break;
            }
        }
    }
}

bool MidiInputManager::Empty() const
{
    int semvalue = _work.getvalue();
    return semvalue <= 0;
}

MidiInput *MidiInputManager::GetMidiInput(std::string const &name)
{
    return dynamic_cast<MidiInput *>(EngineManager::Instance().GetEngine(name));
}

bool MidiInputManager::SetSource(std::string const &name)
{
    MidiInput *src = GetMidiInput(name);

    if (!src)
    {
        return false;
    }

    if (_current)
    {
        _current->SetMidiEnabled(false);
    }

    _current = src;
    _current->SetMidiEnabled(true);

    //Keep system in a valid state (aka with a running driver)
    if (_current->IsMidiEnabled())
    {
        return true;
    }

    _current = GetMidiInput("NULL");
    _current->SetMidiEnabled(true);

    return false;
}

std::string MidiInputManager::GetSource() const
{
    if (_current)
    {
        return _current->_name;
    }

    std::cerr << "BUG: No current input in MidiInputManager " << __LINE__ << std::endl;

    return "ERROR";
}
