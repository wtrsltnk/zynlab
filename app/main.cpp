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

static MasterUI *ui = nullptr;

#endif //ENABLE_FLTKGUI

static Mixer mixer;

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
    if (ui != nullptr)
    {
        delete ui;
        ui = nullptr;
    }
#endif // ENABLE_FLTKGUI

    return 0;
}

int main(
    int argc,
    char *argv[])
{
    Config::Current().init();

    mixer.Init();

    std::cout << "\n"
              << "ZynAddSubFX - Copyright (c) 2002-2011 Nasca Octavian Paul and others\n"
              << "              Copyright (c) 2009-2014 Mark McCurry [active maintainer]\n"
              << "              Copyright (c) 2016-2021 Wouter Saaltink [zynlab maintainer]\n"
              << "Compiled: " << __DATE__ << " " << __TIME__ << "\n"
              << "This program is free software (GNU GPL v2 or later) and \n"
              << "it comes with ABSOLUTELY NO WARRANTY." << std::endl;

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
    settings.swaplr = config.cfg.SwapStereo;

    settings.alias();

    sprng(static_cast<unsigned int>(time(nullptr)));

    cxxopts::Options options("ZynAddSubFX");

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
        ("O,output", "Auto-select output driver", cxxopts::value<std::string>())                  //
        ("I,input", "Auto-select input driver", cxxopts::value<std::string>())                    //
        ("e,exec-after-init", "Command-line to excute ater init", cxxopts::value<std::string>()); //

    auto result = options.parse(argc, argv);

    if (result["h"].as<bool>())
    {
        std::cout << options.help() << std::endl;

        return 0;
    }

    if (result["v"].as<bool>())
    {
        std::cout << "Version: " << VERSION << std::endl;

        return 0;
    }

    if (result["swap"].as<bool>())
    {
        settings.swaplr = 1;
    }

    if (result["sample-rate"].count() == 1 && !settings.SetSampleRate(result["sample-rate"].as<unsigned int>()))
    {
        std::cerr << "ERROR:Incorrect sample rate: " << settings.samplerate
                  << std::endl;

        return 1;
    }

    if (result["buffer-size"].count() == 1 && !settings.SetBufferSize(result["buffer-size"].as<unsigned int>()))
    {
        std::cerr << "ERROR:Incorrect buffer size: " << result["buffer-size"].as<unsigned int>()
                  << std::endl;

        return 1;
    }

    if (result["oscil-size"].count())
    {
        settings.SetOscilSize(result["oscil-size"].as<unsigned int>());
    }

    settings.alias();

    Nio::preferedSampleRate(settings.samplerate);

    if (result["I"].count() == 1)
    {
        Nio::SetDefaultSource(result["I"].as<std::string>());
    }

    if (result["O"].count() == 1)
    {
        Nio::SetDefaultSink(result["O"].as<std::string>());
    }

    bool expectedEngineStarted = Nio::Start(&mixer, &mixer);

    if (!expectedEngineStarted)
    {
        std::cerr << "Failed to start default engine, Defaulting to NULL engine" << std::endl;
    }

    std::cout.precision(1);
    std::cout << std::fixed << "\n"
              << "Sample Rate        = " << settings.samplerate << "\n"
              << "Sound Buffer Size  = " << settings.buffersize << " samples\n"
              << "Internal latency   = " << settings.buffersize_f * 1000.0f / settings.samplerate_f << " ms\n"
              << "ADsynth Oscil Size = " << settings.oscilsize << " samples" << std::endl;

    signal(SIGINT, sigterm_exit);
    signal(SIGTERM, sigterm_exit);

    SaveToFileSerializer serializer;
    if (result["l"].count() == 1)
    {
        auto loadfile = result["l"].as<std::string>();

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

    if (result["L"].count() == 1)
    {
        auto loadinstrument = result["L"].as<std::string>();

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

    // Run a system command after starting zynaddsubfx
    if (result["e"].count() == 1)
    {
        auto execAfterInit = result["e"].as<std::string>();

        std::cout << "Executing user supplied command: " << execAfterInit << std::endl;
        if (system(execAfterInit.c_str()) == -1)
        {
            std::cerr << "Command Failed..." << std::endl;
        }
    }

    BankManager banks;

#ifdef ENABLE_FLTKGUI

    if (!result["no-gui"].as<bool>())
    {
        ui = new MasterUI(&mixer, &banks, &serializer, &Pexitprogram);
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
