#include "EngineManager.h"
#include "AudioOutput.h"
#include "AudioOutputManager.h"
#include "MidiInput.h"
#include "MidiInputManager.h"
#include "NetMidiEngine.h"
#include "Nio.h"
#include "NulEngine.h"
#include "RtEngine.h"
#ifdef OSS
#include "OssEngine.h"
#endif
#ifdef ALSA
#include "AlsaEngine.h"
#endif
#ifdef JACK
#include "JackEngine.h"
#endif
#ifdef PORTAUDIO
#include "PaEngine.h"
#endif
#ifdef SDL2
#include "SdlEngine.h"
#endif

#include <algorithm>
#include <iostream>

EngineManager *EngineManager::_instance = nullptr;

EngineManager &EngineManager::CreateInstance(IMixer *mixer)
{
    if (EngineManager::_instance == nullptr)
    {
        EngineManager::_instance = new EngineManager(mixer);
    }

    return *EngineManager::_instance;
}
EngineManager &EngineManager::Instance()
{
    return *EngineManager::_instance;
}

void EngineManager::DestroyInstance()
{
    delete EngineManager::_instance;

    EngineManager::_instance = nullptr;
}

EngineManager::EngineManager(class IMixer *mixer)
{
    Engine *defaultEng = new NulEngine(mixer->_synth);

    //conditional compiling mess (but contained)
    engines.push_back(defaultEng);
    engines.push_back(new RtEngine());
    engines.push_back(new NetMidiEngine());
#ifdef OSS
    engines.push_back(new OssEngine(mixer->_synth));
#endif
#ifdef ALSA
    engines.push_back(new AlsaEngine(mixer->_synth));
#endif
#ifdef JACK
    engines.push_back(new JackEngine(mixer->_synth));
#endif
#ifdef PORTAUDIO
    engines.push_back(new PaEngine(mixer->_synth));
#endif
#ifdef SDL2
    // TODO Not working yet!
//    engines.push_back(new SdlEngine(mixer->_synth));
#endif

    defaultOut = dynamic_cast<AudioOutput *>(defaultEng);

    defaultIn = dynamic_cast<MidiInput *>(defaultEng);

    //Accept command line/compile time options
    if (!Nio::defaultSink.empty())
    {
        SetDefaultMidiInput(Nio::defaultSink);
    }
    if (!Nio::defaultSource.empty())
    {
        SetDefaultAudioOutput(Nio::defaultSource);
    }
}

EngineManager::~EngineManager()
{
    while (!engines.empty())
    {
        auto engine = engines.back();
        engines.pop_back();
        delete engine;
    }
}

Engine *EngineManager::GetEngine(std::string const &name)
{
    std::string upperName = name;
    transform(upperName.begin(), upperName.end(), upperName.begin(), ::toupper);

    for (auto engine : engines)
    {
        if (engine->_name == upperName)
        {
            return engine;
        }
    }

    return nullptr;
}

bool EngineManager::Start()
{
    bool expected = true;
    if (defaultOut == nullptr || defaultIn == nullptr)
    {
        std::cerr << "ERROR: It looks like someone broke the Nio Output\n"
                  << "       Attempting to recover by defaulting to the\n"
                  << "       Null Engine." << std::endl;
        defaultOut = dynamic_cast<AudioOutput *>(GetEngine("NULL"));
        defaultIn = dynamic_cast<MidiInput *>(GetEngine("NULL"));
    }

    AudioOutputManager::getInstance().currentOut = defaultOut;
    MidiInputManager::Instance()._current = defaultIn;

    //open up the default output(s)
    std::cout << "Starting Audio: " << defaultOut->_name << std::endl;
    defaultOut->SetAudioEnabled(true);
    if (defaultOut->IsAudioEnabled())
    {
        std::cout << "Audio Started" << std::endl;
    }
    else
    {
        expected = false;
        std::cerr << "ERROR: The default audio output failed to open!" << std::endl;
        AudioOutputManager::getInstance().currentOut = dynamic_cast<AudioOutput *>(GetEngine("NULL"));
        AudioOutputManager::getInstance().currentOut->SetAudioEnabled(true);
    }

    std::cout << "Starting MIDI: " << defaultIn->_name << std::endl;
    defaultIn->SetMidiEnabled(true);
    if (defaultIn->IsMidiEnabled())
    {
        std::cout << "MIDI Started" << std::endl;
    }
    else
    {
        expected = false;
        std::cerr << "ERROR: The default MIDI input failed to open!" << std::endl;
        MidiInputManager::Instance()._current = dynamic_cast<MidiInput *>(GetEngine("NULL"));
        MidiInputManager::Instance()._current->SetMidiEnabled(true);
    }

    return expected;
}

void EngineManager::Stop()
{
    for (auto &engine : engines)
    {
        engine->Stop();
    }
}

bool EngineManager::SetDefaultAudioOutput(std::string const &name)
{
    auto chosen = dynamic_cast<MidiInput *>(GetEngine(name));

    if (chosen != nullptr)
    {
        defaultIn = chosen;
        return true;
    }

    //Warn user
    std::cerr << "Error: " << name << " is not a recognized MIDI input source" << std::endl;
    std::cerr << "       Defaulting to the NULL input source" << std::endl;

    return false;
}

bool EngineManager::SetDefaultMidiInput(std::string const &name)
{
    auto chosen = dynamic_cast<AudioOutput *>(GetEngine(name));

    if (chosen != nullptr)
    {
        defaultOut = chosen;
        return true;
    }

    //Warn user
    std::cerr << "Error: " << name << " is not a recognized audio backend" << std::endl;
    std::cerr << "       Defaulting to the NULL audio backend" << std::endl;

    return false;
}
