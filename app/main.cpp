/*
  ZynAddSubFX - a software synthesizer

  main.cpp  -  Main file of the synthesizer
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Copyright (C) 2012-2014 Mark McCurry

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2 or later) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <FL/Fl.H>

#include "ui/common.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <csignal>
#include <ctime>
#include <cxxopts.hpp>
#include <iostream>
#include <thread>

#include <zyn.common/Util.h>
#include <zyn.mixer/Mixer.h>
#include <zyn.mixer/Track.h>
#include <zyn.serialization/BankManager.h>
#include <zyn.serialization/SaveToFileSerializer.h>
#include <zyn.serialization/TrackSerializer.h>
#include <zyn.synth/FFTwrapper.h>

//Nio System
#include "../zyn.nio/Nio.h"

#ifdef ENABLE_FLTKGUI
#include "MasterUI.h"

static MasterUI *ui;

#endif //ENABLE_FLTKGUI

static Mixer mixer;

static unsigned int swaplr = 0; //1 for left-right swapping

static int Pexitprogram = 0; //if the UI set this to 1, the program will exit

//cleanup on signaled exit
void sigterm_exit(int /*sig*/)
{
    Pexitprogram = 1;
}

/*
 * Program exit
 */
int exitprogram()
{
    //ensure that everything has stopped with the mutex wait
    mixer.Lock();
    mixer.Unlock();

    Nio::Stop();

#ifdef ENABLE_FLTKGUI
    delete ui;
#endif // ENABLE_FLTKGUI

    FFT_cleanup();

    return 0;
}

int exitwithversion()
{
    std::cout << "Version: " << VERSION << std::endl;

    return 0;
}

int exitwithhelp()
{
    std::cout << "Usage: zynaddsubfx [OPTION]\n\n"
              << "  -h , --help \t\t\t\t Display command-line help and exit\n"
              << "  -v , --version \t\t\t Display version and exit\n"
              << "  -l file, --load=FILE\t\t\t Loads a .xmz file\n"
              << "  -L file, --load-instrument=FILE\t Loads a .xiz file\n"
              << "  -r SR, --sample-rate=SR\t\t Set the sample rate SR\n"
              << "  -b BS, --buffer-size=SR\t\t Set the buffer size (granularity)\n"
              << "  -o OS, --oscil-size=OS\t\t Set the ADsynth oscil. size\n"
              << "  -S , --swap\t\t\t\t Swap Left <--> Right\n"
              << "  -D , --dump\t\t\t\t Dumps midi note ON/OFF commands\n"
              << "  -U , --no-gui\t\t\t\t Run ZynAddSubFX without user interface\n"
              << "  -O , --output\t\t\t\t Set Output Engine\n"
              << "  -I , --input\t\t\t\t Set Input Engine\n"
              << "  -e , --exec-after-init\t\t Run post-initialization script\n"
              << std::endl;

    return 0;
}

int main(int argc, char *argv[])
{
    int noui = 0;

    Config::Current().init();

    std::cout << "\n"
              << "ZynAddSubFX - Copyright (c) 2002-2011 Nasca Octavian Paul and others\n"
              << "              Copyright (c) 2009-2014 Mark McCurry [active maintainer]\n"
              << "Compiled: " << __DATE__ << " " << __TIME__ << "\n"
              << "This program is free software (GNU GPL v2 or later) and \n"
              << "it comes with ABSOLUTELY NO WARRANTY.\n"
              << std::endl;

    if (argc == 1)
    {
        std::cout << "Try 'zynaddsubfx --help' for command-line options." << std::endl;
    }

    auto const &config = Config::Current();
    auto &settings = SystemSettings::Instance();

    /* Get the settings from the Config*/
    settings.samplerate = config.cfg.SampleRate;
    settings.buffersize = config.cfg.SoundBufferSize;
    settings.oscilsize = config.cfg.OscilSize;
    swaplr = config.cfg.SwapStereo;

    settings.alias();

    Nio::preferedSampleRate(settings.samplerate);

    sprng(static_cast<unsigned int>(time(nullptr)));

    cxxopts::Options options("ZynAddSubFX", "Copyright (c) 2002-2011 Nasca Octavian Paul and others\n              Copyright (c) 2009-2014 Mark McCurry [active maintainer]\nThis program is free software (GNU GPL v2 or later) and \nit comes with ABSOLUTELY NO WARRANTY.\n");

    /* Parse command-line options */
    options.add_options()                                                                         //
        ("l,load", "Load session", cxxopts::value<std::string>())                                 // Load a session from file
        ("L,load-instrument", "Load instument", cxxopts::value<std::string>())                    //
        ("r,sample-rate", "Set sampel rate", cxxopts::value<unsigned int>())                      //
        ("b,buffer-size", "Set buffersize", cxxopts::value<unsigned int>())                       //
        ("o,oscil-size", "Set oscil size", cxxopts::value<unsigned int>())                        //
        ("D,dump", "Dump", cxxopts::value<bool>())                                                //
        ("S,swap", "Swap left and right", cxxopts::value<bool>())                                 //
        ("U,no-gui", "Open without ui", cxxopts::value<bool>())                                   //
        ("Y,dummy", "Dummy", cxxopts::value<bool>())                                              //
        ("h,help", "Show help", cxxopts::value<bool>())                                           //
        ("v,version", "Show version", cxxopts::value<bool>())                                     //
        ("N,named", "", cxxopts::value<bool>())                                                   //
        ("a,auto-connect", "Auto-connect", cxxopts::value<bool>())                                //
        ("O,output", "Auto-select output driver", cxxopts::value<std::string>())                  //
        ("I,input", "Auto-select input driver", cxxopts::value<std::string>())                    //
        ("e,exec-after-init", "Command-line to excute ater init", cxxopts::value<std::string>()); //

    auto result = options.parse(argc, argv);

    std::string loadfile, loadinstrument, execAfterInit;

    if (result["h"].as<bool>())
    {
        return exitwithhelp();
    }

    if (result["v"].as<bool>())
    {
        return exitwithversion();
    }

    if (result["no-gui"].as<bool>())
    {
        noui = 1;
    }

    if (result["swap"].as<bool>())
    {
        swaplr = 1;
    }

    if (result["I"].count() == 1)
    {
        Nio::SetDefaultSource(result["I"].as<std::string>());
    }

    if (result["O"].count() == 1)
    {
        Nio::SetDefaultSink(result["O"].as<std::string>());
    }

    if (result["l"].count() == 1)
    {
        loadfile = result["l"].as<std::string>();
    }

    if (result["L"].count() == 1)
    {
        loadinstrument = result["L"].as<std::string>();
    }

    if (result["e"].count() == 1)
    {
        execAfterInit = result["e"].as<std::string>();
    }

    if (result["sample-rate"].count() == 1)
    {
        settings.samplerate = result["sample-rate"].as<unsigned int>();
        if (settings.samplerate < 4000)
        {
            std::cerr << "ERROR:Incorrect sample rate: " << settings.samplerate
                      << std::endl;
            return 1;
        }
    }

    if (result["buffer-size"].count() == 1)
    {
        settings.buffersize = result["buffer-size"].as<unsigned int>();
        if (settings.buffersize < 2)
        {
            std::cerr << "ERROR:Incorrect buffer size: " << settings.buffersize
                      << std::endl;
            return 1;
        }
    }

    if (result["oscil-size"].count())
    {
        unsigned int tmp = 0;
        settings.oscilsize = tmp = result["oscil-size"].as<unsigned int>();
        if (settings.oscilsize < MAX_AD_HARMONICS * 2)
        {
            settings.oscilsize = MAX_AD_HARMONICS * 2;
        }

        settings.oscilsize = static_cast<unsigned int>(powf(2, ceil(logf(settings.oscilsize - 1.0f) / logf(2.0f))));

        if (tmp != settings.oscilsize)
        {
            std::cerr << "synth.oscilsize is wrong (must be 2^n) or too small. Adjusting to "
                      << settings.oscilsize << "." << std::endl;
        }
    }

    settings.alias();

    std::cout.precision(1);
    std::cout << std::fixed << "\n"
              << "Sample Rate        = " << settings.samplerate << "\n"
              << "Sound Buffer Size  = " << settings.buffersize << " samples\n"
              << "Internal latency   = " << settings.buffersize_f * 1000.0f / settings.samplerate_f << " ms\n"
              << "ADsynth Oscil.Size = " << settings.oscilsize << " samples" << std::endl;

    signal(SIGINT, sigterm_exit);
    signal(SIGTERM, sigterm_exit);

    mixer.Init();
    mixer.swaplr = swaplr;

    SaveToFileSerializer serializer;
    if (!loadfile.empty())
    {
        int tmp = serializer.LoadMixer(&mixer, loadfile);
        if (tmp < 0)
        {
            std::cerr << "ERROR: Could not load master file " << loadfile << "." << std::endl;
            exit(1);
        }
        else
        {
            mixer.ApplyParameters();
            std::cout << "Master file loaded." << std::endl;
        }
    }

    if (!loadinstrument.empty())
    {
        int tmp = serializer.LoadTrack(mixer.GetTrack(0), loadinstrument);
        if (tmp < 0)
        {
            std::cerr << "ERROR: Could not load instrument file "
                      << loadinstrument << '.' << std::endl;
            exit(1);
        }
        else
        {
            mixer.GetTrack(0)->ApplyParameters();
            std::cout << "Instrument file loaded." << std::endl;
        }
    }

    //Run the Nio system
    bool expectedEngineStarted = Nio::Start(&mixer, &mixer);

    if (!expectedEngineStarted)
    {
        std::cerr << "Failed to start default engine, Defaulting to NULL engine" << std::endl;
    }

    // Run a system command after starting zynaddsubfx
    if (!execAfterInit.empty())
    {
        std::cout << "Executing user supplied command: " << execAfterInit << std::endl;
        if (system(execAfterInit.c_str()) == -1)
        {
            std::cerr << "Command Failed..." << std::endl;
        }
    }

    BankManager banks;

#ifdef ENABLE_FLTKGUI

    ui = new MasterUI(&mixer, &banks, &serializer, &Pexitprogram);

    if (!noui)
    {
        ui->showUI();

        if (!expectedEngineStarted)
        {
            fl_alert("Default IO did not initialize.\nDefaulting to NULL backend.");
        }
    }

#endif

    while (Pexitprogram == 0)
    {
#ifdef ENABLE_FLTKGUI
        Fl::wait(0.02);
#else  // ENABLE_FLTKGUI
        usleep(100000);
#endif // ENABLE_FLTKGUI
    }

    return exitprogram();
}
