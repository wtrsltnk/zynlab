#include <chrono>
#include <memory>
#include <thread>
#include <unistd.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.nio/EngineManager.h>
#include <zyn.nio/MidiInputManager.h>
#include <zyn.nio/Nio.h>
#include <zyn.serialization/LibraryManager.h>
#include <zyn.synth/FFTwrapper.h>
#include <zyn.synth/OscilGen.h>

#include "app.tiny.h"

static int Pexitprogram = 0;

static LibraryManager libraryManager;
static Mixer *mixer;

//cleanup on signaled exit
void sigterm_exit(int /*sig*/)
{
    Pexitprogram = 1;
}

/*
 * Program initialisation
 */
void initprogram()
{
    Config::Current().init();

    /* Get the settings from the Config*/
    SystemSettings::Instance().samplerate = Config::Current().cfg.SampleRate;
    SystemSettings::Instance().buffersize = Config::Current().cfg.SoundBufferSize;
    SystemSettings::Instance().oscilsize = Config::Current().cfg.OscilSize;
    SystemSettings::Instance().alias();

    std::cerr.precision(1);
    std::cerr << std::fixed;
    std::cerr << "\nSample Rate = \t\t" << SystemSettings::Instance().samplerate << std::endl;
    std::cerr << "Sound Buffer Size = \t" << SystemSettings::Instance().buffersize << " samples" << std::endl;
    std::cerr << "Internal latency = \t\t" << SystemSettings::Instance().buffersize_f * 1000.0f / SystemSettings::Instance().samplerate_f << " ms" << std::endl;
    std::cerr << "ADsynth Oscil.Size = \t" << SystemSettings::Instance().oscilsize << " samples" << std::endl;

    mixer = new Mixer();
    mixer->Init();
    mixer->swaplr = Config::Current().cfg.SwapStereo;

    Nio::preferedSampleRate(SystemSettings::Instance().samplerate);

    signal(SIGINT, sigterm_exit);
    signal(SIGTERM, sigterm_exit);

    //Run the Nio system
    if (!Nio::Start(mixer, mixer))
    {
        exit(-1);
    }

    Nio::SelectSink("PA");
    Nio::SelectSource("RT");

    Config::Current()
        .init();
}

/*
 * Program exit
 */
int exitprogram()
{
    //ensure that everything has stopped with the mutex wait
    mixer->Lock();
    mixer->Unlock();

    Nio::Stop();

    delete mixer;
    FFT_cleanup();

    return 0;
}

int main(int /*argc*/, char * /*argv*/ [])
{
    initprogram();

    AppTiny app;

    auto result = app.Run();

    exitprogram();

    return result;
}
