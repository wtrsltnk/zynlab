#include "Nio.h"

#include "AudioOutput.h"
#include "AudioOutputManager.h"
#include "EngineManager.h"
#include "MidiInput.h"
#include "MidiInputManager.h"

#include <algorithm>
#include <iostream>

static AudioOutputManager *_audioOutpuManager = nullptr;
static EngineManager *_engineManager = nullptr;
static MidiInputManager *_midiInputManager = nullptr;

std::string Nio::defaultSink;
std::string Nio::defaultSource;

bool Nio::Start(IAudioGenerator *audioGenerator)
{
    _midiInputManager = &MidiInputManager::CreateInstance(audioGenerator);    //Enable input wrapper
    _audioOutpuManager = &AudioOutputManager::createInstance(audioGenerator); //Initialize the Output Systems
    _engineManager = &EngineManager::CreateInstance(audioGenerator);          //Initialize The Engines

    return _engineManager->Start();
}

void Nio::Stop()
{
    _engineManager->Stop();

    _midiInputManager = nullptr;
    MidiInputManager::DestroyInstance(); // Disable input wrapper
    _audioOutpuManager = nullptr;
    AudioOutputManager::destroyInstance(); //Destroy the Output Systems
    _engineManager = nullptr;
    EngineManager::DestroyInstance(); //DestroyThe Engines
}

void Nio::SetDefaultSource(const std::string &name)
{
    defaultSource = name;
    std::transform(defaultSource.begin(), defaultSource.end(), defaultSource.begin(), ::toupper);
}

void Nio::SetDefaultSink(const std::string &name)
{
    defaultSink = name;
    std::transform(defaultSink.begin(), defaultSink.end(), defaultSink.begin(), ::toupper);
}

bool Nio::SelectSource(const std::string &name)
{
    return _midiInputManager->SetSource(name);
}

bool Nio::SelectSink(const std::string &name)
{
    return _audioOutpuManager->SetSink(name);
}

std::set<std::string> Nio::GetSources()
{
    std::set<std::string> sources;

    for (Engine *e : _engineManager->engines)
    {
        if (dynamic_cast<MidiInput *>(e) != nullptr)
        {
            sources.insert(e->_name);
        }
    }

    return sources;
}

std::set<std::string> Nio::GetSinks()
{
    std::set<std::string> sinks;

    for (Engine *e : _engineManager->engines)
    {
        if (dynamic_cast<AudioOutput *>(e) != nullptr)
        {
            sinks.insert(e->_name);
        }
    }

    return sinks;
}

std::string Nio::GetSelectedSource()
{
    return _midiInputManager->GetSource();
}

std::string Nio::GetSelectedSink()
{
    return _audioOutpuManager->GetSink();
}

#ifdef JACK
#include <jack/jack.h>
void Nio::preferedSampleRate(unsigned int &rate)
{
    jack_client_t *client = jack_client_open("temp-client",
                                             JackNoStartServer, 0);
    if (client)
    {
        rate = jack_get_sample_rate(client);
        jack_client_close(client);
    }
}
#else
void Nio::preferedSampleRate(unsigned int &)
{}
#endif
