#include "EngineManager.h"
#include "AudioOutput.h"
#include "AudioOutputManager.h"
#include "MidiInput.h"
#include "MidiInputManager.h"
#include "Nio.h"
#include "NulEngine.h"
#include "RtEngine.h"
#include "NetMidiEngine.h"
#if OSS
#include "OssEngine.h"
#endif
#if ALSA
#include "AlsaEngine.h"
#endif
#if JACK
#include "JackEngine.h"
#endif
#if PORTAUDIO
#include "PaEngine.h"
#endif
#if SDL2
#include "SdlEngine.h"
#endif

#include <algorithm>
#include <iostream>

using namespace std;

EngineManager *EngineManager::_instance = nullptr;

EngineManager &EngineManager::createInstance(IMixer *mixer)
{
    if (EngineManager::_instance == 0) EngineManager::_instance = new EngineManager(mixer);
    return *EngineManager::_instance;
}
EngineManager &EngineManager::getInstance()
{
    return *EngineManager::_instance;
}

void EngineManager::destroyInstance()
{
    if (EngineManager::_instance != 0) delete EngineManager::_instance;
    EngineManager::_instance = 0;
}

EngineManager::EngineManager(class IMixer *mixer)
{
    Engine *defaultEng = new NulEngine(mixer->_synth);

    //conditional compiling mess (but contained)
    engines.push_back(defaultEng);
    engines.push_back(new RtEngine());
    engines.push_back(new NetMidiEngine());
#if OSS
    engines.push_back(new OssEngine(mixer->_synth));
#endif
#if ALSA
    engines.push_back(new AlsaEngine(mixer->_synth));
#endif
#if JACK
    engines.push_back(new JackEngine(mixer->_synth));
#endif
#if PORTAUDIO
    engines.push_back(new PaEngine(mixer->_synth));
#endif
#if SDL2
    // TODO Not working yet!
    //engines.push_back(new SdlEngine());
#endif

    defaultOut = dynamic_cast<AudioOutput *>(defaultEng);

    defaultIn = dynamic_cast<MidiInput *>(defaultEng);

    //Accept command line/compile time options
    if (!Nio::defaultSink.empty())
        setOutDefault(Nio::defaultSink);

    if (!Nio::defaultSource.empty())
        setInDefault(Nio::defaultSource);
}

EngineManager::~EngineManager()
{
    for (list<Engine *>::iterator itr = engines.begin();
         itr != engines.end(); ++itr)
        delete *itr;
}

Engine *EngineManager::getEng(string name)
{
    transform(name.begin(), name.end(), name.begin(), ::toupper);
    for (list<Engine *>::iterator itr = engines.begin();
         itr != engines.end(); ++itr)
        if ((*itr)->name == name)
            return *itr;
    return NULL;
}

bool EngineManager::start()
{
    bool expected = true;
    if (!(defaultOut && defaultIn))
    {
        cerr << "ERROR: It looks like someone broke the Nio Output\n"
             << "       Attempting to recover by defaulting to the\n"
             << "       Null Engine." << endl;
        defaultOut = dynamic_cast<AudioOutput *>(getEng("NULL"));
        defaultIn = dynamic_cast<MidiInput *>(getEng("NULL"));
    }

    AudioOutputManager::getInstance().currentOut = defaultOut;
    MidiInputManager::getInstance().current = defaultIn;

    //open up the default output(s)
    cout << "Starting Audio: " << defaultOut->name << endl;
    defaultOut->setAudioEn(true);
    if (defaultOut->getAudioEn())
        cout << "Audio Started" << endl;
    else
    {
        expected = false;
        cerr << "ERROR: The default audio output failed to open!" << endl;
        AudioOutputManager::getInstance().currentOut = dynamic_cast<AudioOutput *>(getEng("NULL"));
        AudioOutputManager::getInstance().currentOut->setAudioEn(true);
    }

    cout << "Starting MIDI: " << defaultIn->name << endl;
    defaultIn->setMidiEn(true);
    if (defaultIn->getMidiEn())
        cout << "MIDI Started" << endl;
    else
    { //recover
        expected = false;
        cerr << "ERROR: The default MIDI input failed to open!" << endl;
        MidiInputManager::getInstance().current = dynamic_cast<MidiInput *>(getEng("NULL"));
        MidiInputManager::getInstance().current->setMidiEn(true);
    }

    //Show if expected drivers were booted
    return expected;
}

void EngineManager::stop()
{
    for (list<Engine *>::iterator itr = engines.begin();
         itr != engines.end(); ++itr)
        (*itr)->Stop();
}

bool EngineManager::setInDefault(string name)
{
    MidiInput *chosen;
    if ((chosen = dynamic_cast<MidiInput *>(getEng(name))))
    { //got the input
        defaultIn = chosen;
        return true;
    }

    //Warn user
    cerr << "Error: " << name << " is not a recognized MIDI input source"
         << endl;
    cerr << "       Defaulting to the NULL input source" << endl;

    return false;
}

bool EngineManager::setOutDefault(string name)
{
    AudioOutput *chosen;
    if ((chosen = dynamic_cast<AudioOutput *>(getEng(name))))
    { //got the output
        defaultOut = chosen;
        return true;
    }

    //Warn user
    cerr << "Error: " << name << " is not a recognized audio backend" << endl;
    cerr << "       Defaulting to the NULL audio backend" << endl;
    return false;
}
