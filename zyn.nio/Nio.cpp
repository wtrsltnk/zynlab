#include "Nio.h"
#include "AudioOutput.h"
#include "AudioOutputManager.h"
#include "EngineManager.h"
#include "MidiInput.h"
#include "MidiInputManager.h"
#include "WavEngine.h"

#include <algorithm>
#include <iostream>

MidiInputManager *in = NULL;
AudioOutputManager *out = NULL;
EngineManager *eng = NULL;

std::string Nio::defaultSource = IN_DEFAULT;
std::string Nio::defaultSink = OUT_DEFAULT;

bool Nio::start(IMixer *mixer)
{
    in = &MidiInputManager::createInstance(mixer);    //Enable input wrapper
    out = &AudioOutputManager::createInstance(mixer); //Initialize the Output Systems
    eng = &EngineManager::createInstance(mixer);      //Initialize The Engines

    return eng->start();
}

void Nio::stop()
{
    eng->stop();
}

void Nio::setDefaultSource(const std::string &name)
{
    defaultSource = name;
    std::transform(defaultSource.begin(), defaultSource.end(), defaultSource.begin(), ::toupper);
}

void Nio::setDefaultSink(const std::string &name)
{
    defaultSink = name;
    std::transform(defaultSink.begin(), defaultSink.end(), defaultSink.begin(), ::toupper);
}

bool Nio::setSource(const std::string &name)
{
    return in->setSource(name);
}

bool Nio::setSink(const std::string &name)
{
    return out->setSink(name);
}

std::set<std::string> Nio::getSources(void)
{
    std::set<std::string> sources;

    for (Engine *e : eng->engines)
        if (dynamic_cast<MidiInput *>(e))
            sources.insert(e->name);

    return sources;
}

std::set<std::string> Nio::getSinks(void)
{
    std::set<std::string> sinks;

    for (Engine *e : eng->engines)
        if (dynamic_cast<AudioOutput *>(e))
            sinks.insert(e->name);

    return sinks;
}

std::string Nio::getSource()
{
    return in->getSource();
}

std::string Nio::getSink()
{
    return out->getSink();
}

#if JACK
#include <jack/jack.h>
void Nio::preferedSampleRate(unsigned &rate)
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
void Nio::preferedSampleRate(unsigned &)
{}
#endif
