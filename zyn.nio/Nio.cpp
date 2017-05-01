#include "Nio.h"
#include "AudioOutput.h"
#include "AudioOutputManager.h"
#include "MidiInput.h"
#include "MidiInputManager.h"
#include "EngineManager.h"
#include "WavEngine.h"
#include <iostream>
#include <algorithm>
using std::string;
using std::set;
using std::cerr;
using std::endl;

MidiInputManager     *in  = NULL;
AudioOutputManager    *out = NULL;
EngineManager *eng = NULL;
string     postfix;

bool   Nio::autoConnect   = false;
string Nio::defaultSource = IN_DEFAULT;
string Nio::defaultSink   = OUT_DEFAULT;

bool Nio::start(IMixer* mixer)
{
    in  = &MidiInputManager::createInstance(mixer); //Enable input wrapper
    out = &AudioOutputManager::createInstance(mixer); //Initialize the Output Systems
    eng = &EngineManager::getInstance(); //Initialize The Engines

    return eng->start();
}

void Nio::stop()
{
    eng->stop();
}

void Nio::setDefaultSource(string name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    defaultSource = name;
}

void Nio::setDefaultSink(string name)
{
    std::transform(name.begin(), name.end(), name.begin(), ::toupper);
    defaultSink = name;
}

bool Nio::setSource(string name)
{
    return in->setSource(name);
}

bool Nio::setSink(string name)
{
    return out->setSink(name);
}

void Nio::setPostfix(std::string post)
{
    postfix = post;
}

std::string Nio::getPostfix(void)
{
    return postfix;
}

set<string> Nio::getSources(void)
{
    set<string> sources;
    for(std::list<Engine *>::iterator itr = eng->engines.begin();
        itr != eng->engines.end(); ++itr)
        if(dynamic_cast<MidiInput *>(*itr))
            sources.insert((*itr)->name);
    return sources;
}

set<string> Nio::getSinks(void)
{
    set<string> sinks;
    for(std::list<Engine *>::iterator itr = eng->engines.begin();
        itr != eng->engines.end(); ++itr)
        if(dynamic_cast<AudioOutput *>(*itr))
            sinks.insert((*itr)->name);
    return sinks;
}

string Nio::getSource()
{
    return in->getSource();
}

string Nio::getSink()
{
    return out->getSink();
}

#if JACK
#include <jack/jack.h>
void Nio::preferedSampleRate(unsigned &rate)
{
    jack_client_t *client = jack_client_open("temp-client",
                                             JackNoStartServer, 0);
    if(client) {
        rate = jack_get_sample_rate(client);
        jack_client_close(client);
    }
}
#else
void Nio::preferedSampleRate(unsigned &)
{}
#endif

void Nio::waveNew(class WavFile *wave)
{
    out->wave->newFile(wave);
}

void Nio::waveStart(void)
{
    out->wave->Start();
}

void Nio::waveStop(void)
{
    out->wave->Stop();
}

void Nio::waveEnd(void)
{
    out->wave->destroyFile();
}
