/*
  ZynAddSubFX - a software synthesizer

  Config.cpp - Configuration file functions
  Copyright (C) 2003-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

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

#include "Config.h"
#include "PresetsSerializer.h"
#include <cmath>
#include <direct.h>

using namespace std;

Config *Config::_instance = nullptr;

Config::Config()
{
    cfg.SampleRate = 44100;
    cfg.SoundBufferSize = 256;
    cfg.OscilSize = 1024;
    cfg.SwapStereo = 0;
}

Config::~Config()
{
    delete[] cfg.LinuxOSSWaveOutDev;
    delete[] cfg.LinuxOSSSeqInDev;
}

Config &Config::Current()
{
    return *_instance;
}

void Config::init()
{
    if (_instance != nullptr)
    {
        return;
    }

    _instance = new Config();

    _instance->maxstringsize = MAX_STRING_SIZE; //for ui
    //defaults
    _instance->cfg.SampleRate = 44100;
    _instance->cfg.SoundBufferSize = 256;
    _instance->cfg.OscilSize = 1024;
    _instance->cfg.SwapStereo = 0;

    _instance->cfg.LinuxOSSWaveOutDev = new char[MAX_STRING_SIZE];
    snprintf(_instance->cfg.LinuxOSSWaveOutDev, MAX_STRING_SIZE, "/dev/dsp");
    _instance->cfg.LinuxOSSSeqInDev = new char[MAX_STRING_SIZE];
    snprintf(_instance->cfg.LinuxOSSSeqInDev, MAX_STRING_SIZE, "/dev/sequencer");

    _instance->cfg.DumpFile = "zynaddsubfx_dump.txt";

    _instance->cfg.WindowsWaveOutId = 0;
    _instance->cfg.WindowsMidiInId = 0;

    _instance->cfg.BankUIAutoClose = 0;
    _instance->cfg.DumpNotesToFile = 0;
    _instance->cfg.DumpAppend = 1;

    _instance->cfg.GzipCompression = 3;

    _instance->cfg.Interpolation = 0;
    _instance->cfg.CheckPADsynth = 1;
    _instance->cfg.IgnoreProgramChange = 0;

    _instance->cfg.UserInterfaceMode = 0;
    _instance->cfg.VirKeybLayout = 1;

    //get the midi input devices name
    _instance->cfg.currentBankDir = "./testbnk";

    char filename[MAX_STRING_SIZE];
    _instance->getConfigFileName(filename, MAX_STRING_SIZE);
    _instance->readConfig(filename);

    if (_instance->cfg.bankRootDirList[0].empty())
    {
        int b = 0;
        //banks
        _instance->cfg.bankRootDirList[b++] = "~/banks";
        _instance->cfg.bankRootDirList[b++] = "./";
        _instance->cfg.bankRootDirList[b++] = "/usr/share/zynaddsubfx/banks";
        _instance->cfg.bankRootDirList[b++] = "/usr/local/share/zynaddsubfx/banks";
#ifdef __APPLE__
        _instance->cfg.bankRootDirList[b++] = "../Resources/banks";
#else
        _instance->cfg.bankRootDirList[b++] = "../banks";
#endif
        _instance->cfg.bankRootDirList[b++] = "banks";
#ifdef _WIN32
        _instance->cfg.bankRootDirList[b++] = R"(C:\Code\synthdev\zynaddsubfx-instruments\banks)";
        _instance->cfg.bankRootDirList[b++] = R"(C:\Code\synthdev\samples)";
#endif // _WIN32
    }

    if (_instance->cfg.presetsDirList[0].empty())
    {
        //presets
        _instance->cfg.presetsDirList[0] = "./";
#ifdef __APPLE__
        _instance->cfg.presetsDirList[1] = "../Resources/presets";
#else
        _instance->cfg.presetsDirList[1] = "../presets";
#endif
        _instance->cfg.presetsDirList[2] = "presets";
        _instance->cfg.presetsDirList[3] = "/usr/share/zynaddsubfx/presets";
        _instance->cfg.presetsDirList[4] = "/usr/local/share/zynaddsubfx/presets";
    }
    _instance->cfg.LinuxALSAaudioDev = "default";
    _instance->cfg.nameTag = "";
}

void Config::save()
{
    char filename[MAX_STRING_SIZE];
    getConfigFileName(filename, MAX_STRING_SIZE);
    saveConfig(filename);
}

void Config::clearbankrootdirlist()
{
    for (auto &i : cfg.bankRootDirList)
    {
        i.clear();
    }
}

void Config::clearpresetsdirlist()
{
    for (auto &i : cfg.presetsDirList)
    {
        i.clear();
    }
}

void Config::readConfig(const char *filename)
{
    PresetsSerializer xmlcfg;
    if (xmlcfg.loadXMLfile(filename) < 0)
    {
        return;
    }

    if (xmlcfg.enterbranch("CONFIGURATION"))
    {
        cfg.SampleRate = xmlcfg.getparunsigned("sample_rate",
                                               cfg.SampleRate,
                                               4000,
                                               1024000);
        cfg.SoundBufferSize = xmlcfg.getparunsigned("sound_buffer_size",
                                                    cfg.SoundBufferSize,
                                                    16,
                                                    8192);
        cfg.OscilSize = xmlcfg.getparunsigned("oscil_size",
                                              cfg.OscilSize,
                                              MAX_AD_HARMONICS * 2,
                                              131072);
        cfg.SwapStereo = xmlcfg.getparunsigned("swap_stereo",
                                               cfg.SwapStereo,
                                               0,
                                               1);
        cfg.BankUIAutoClose = xmlcfg.getpar("bank_window_auto_close",
                                            cfg.BankUIAutoClose,
                                            0,
                                            1);

        cfg.DumpNotesToFile = xmlcfg.getpar("dump_notes_to_file",
                                            cfg.DumpNotesToFile,
                                            0,
                                            1);
        cfg.DumpAppend = xmlcfg.getpar("dump_append",
                                       cfg.DumpAppend,
                                       0,
                                       1);
        cfg.DumpFile = xmlcfg.getparstr("dump_file", "");

        cfg.GzipCompression = xmlcfg.getpar("gzip_compression",
                                            cfg.GzipCompression,
                                            0,
                                            9);

        cfg.currentBankDir = xmlcfg.getparstr("bank_current", "");
        cfg.Interpolation = xmlcfg.getpar("interpolation",
                                          cfg.Interpolation,
                                          0,
                                          1);

        cfg.CheckPADsynth = xmlcfg.getpar("check_pad_synth",
                                          cfg.CheckPADsynth,
                                          0,
                                          1);

        cfg.IgnoreProgramChange = xmlcfg.getpar("ignore_program_change",
                                                cfg.IgnoreProgramChange,
                                                0,
                                                1);

        cfg.UserInterfaceMode = xmlcfg.getpar("user_interface_mode",
                                              cfg.UserInterfaceMode,
                                              0,
                                              2);
        cfg.VirKeybLayout = xmlcfg.getpar("virtual_keyboard_layout",
                                          cfg.VirKeybLayout,
                                          0,
                                          10);

        //get bankroot dirs
        for (int i = 0; i < MAX_BANK_ROOT_DIRS; ++i)
        {
            if (xmlcfg.enterbranch("BANKROOT", i))
            {
                cfg.bankRootDirList[i] = xmlcfg.getparstr("bank_root", "");
                xmlcfg.exitbranch();
            }
        }

        //get preset root dirs
        for (int i = 0; i < MAX_BANK_ROOT_DIRS; ++i)
        {
            if (xmlcfg.enterbranch("PRESETSROOT", i))
            {
                cfg.presetsDirList[i] = xmlcfg.getparstr("presets_root", "");
                xmlcfg.exitbranch();
            }
        }

        //linux stuff
        xmlcfg.getparstr("linux_oss_wave_out_dev",
                         cfg.LinuxOSSWaveOutDev,
                         MAX_STRING_SIZE);
        xmlcfg.getparstr("linux_oss_seq_in_dev",
                         cfg.LinuxOSSSeqInDev,
                         MAX_STRING_SIZE);

        //windows stuff
        cfg.WindowsWaveOutId = xmlcfg.getpar("windows_wave_out_id",
                                             cfg.WindowsWaveOutId,
                                             0,
                                             1);
        cfg.WindowsMidiInId = xmlcfg.getpar("windows_midi_in_id",
                                            cfg.WindowsMidiInId,
                                            0,
                                            1);

        xmlcfg.exitbranch();
    }

    cfg.OscilSize = static_cast<unsigned int>(powf(2, ceilf(logf(cfg.OscilSize - 1.0f) / logf(2.0f))));
}

void Config::saveConfig(const char *filename)
{
    PresetsSerializer xmlcfg;

    xmlcfg.beginbranch("CONFIGURATION");

    xmlcfg.addparunsigned("sample_rate", cfg.SampleRate);
    xmlcfg.addparunsigned("sound_buffer_size", cfg.SoundBufferSize);
    xmlcfg.addparunsigned("oscil_size", cfg.OscilSize);
    xmlcfg.addparunsigned("swap_stereo", cfg.SwapStereo);
    xmlcfg.addpar("bank_window_auto_close", cfg.BankUIAutoClose);

    xmlcfg.addpar("dump_notes_to_file", cfg.DumpNotesToFile);
    xmlcfg.addpar("dump_append", cfg.DumpAppend);
    xmlcfg.addparstr("dump_file", cfg.DumpFile);

    xmlcfg.addpar("gzip_compression", cfg.GzipCompression);

    xmlcfg.addpar("check_pad_synth", cfg.CheckPADsynth);
    xmlcfg.addpar("ignore_program_change", cfg.IgnoreProgramChange);

    xmlcfg.addparstr("bank_current", cfg.currentBankDir);

    xmlcfg.addpar("user_interface_mode", cfg.UserInterfaceMode);
    xmlcfg.addpar("virtual_keyboard_layout", cfg.VirKeybLayout);

    for (int i = 0; i < MAX_BANK_ROOT_DIRS; ++i)
    {
        if (!cfg.bankRootDirList[i].empty())
        {
            xmlcfg.beginbranch("BANKROOT", i);
            xmlcfg.addparstr("bank_root", cfg.bankRootDirList[i]);
            xmlcfg.endbranch();
        }
    }

    for (int i = 0; i < MAX_BANK_ROOT_DIRS; ++i)
    {
        if (!cfg.presetsDirList[i].empty())
        {
            xmlcfg.beginbranch("PRESETSROOT", i);
            xmlcfg.addparstr("presets_root", cfg.presetsDirList[i]);
            xmlcfg.endbranch();
        }
    }

    xmlcfg.addpar("interpolation", cfg.Interpolation);

    //linux stuff
    xmlcfg.addparstr("linux_oss_wave_out_dev", cfg.LinuxOSSWaveOutDev);
    xmlcfg.addparstr("linux_oss_seq_in_dev", cfg.LinuxOSSSeqInDev);

    //windows stuff
    xmlcfg.addpar("windows_wave_out_id", cfg.WindowsWaveOutId);
    xmlcfg.addpar("windows_midi_in_id", cfg.WindowsMidiInId);

    xmlcfg.endbranch();

    int tmp = cfg.GzipCompression;
    cfg.GzipCompression = 0;
    xmlcfg.saveXMLfile(filename);
    cfg.GzipCompression = tmp;
}

void Config::getConfigFileName(char *name, int namesize)
{
    name[0] = 0;
#ifdef _WIN32
    char temp[FILENAME_MAX];
    snprintf(name, static_cast<size_t>(namesize), "%s%s", _getcwd(temp, FILENAME_MAX), "\\.zynaddsubfxXML.cfg");
#else
    snprintf(name, namesize, "%s%s", getenv("HOME"), "/.zynaddsubfxXML.cfg");
#endif
}
