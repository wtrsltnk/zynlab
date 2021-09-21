/*
  ZynAddSubFX - a software synthesizer

  Bank.cpp - Instrument Bank
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Copyright (C) 2010-2010 Mark McCurry
  Author: Nasca Octavian Paul
          Mark McCurry

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

#include "BankManager.h"

#include "SaveToFileSerializer.h"
#include <algorithm>
#include <fcntl.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <zyn.common/Config.h>
#include <zyn.common/PresetsSerializer.h>
#include <zyn.common/Util.h>
#include <zyn.mixer/Track.h>

#define INSTRUMENT_EXTENSION ".xiz"

//if this file exists into a directory, this make the directory to be considered as a bank, even if it not contains a instrument file
#define FORCE_BANK_DIR_FILE ".bankdir"

BankManager::BankManager()
    : _defaultinsname("defaults")
{
    ClearBank();
    _bankfiletitle = _dirname;
    LoadBankByDirectoryName(Config::Current().cfg.currentBankDir);
}

BankManager::~BankManager()
{
    ClearBank();
}

/*
 * Get the name of an instrument from the bank
 */
std::string BankManager::GetName(unsigned int ninstrument)
{
    if (EmptySlot(ninstrument))
    {
        return _defaultinsname;
    }

    return _instrumentsInCurrentBank[ninstrument].name;
}

/*
 * Get the numbered name of an instrument from the bank
 */
std::string BankManager::GetNameNumbered(unsigned int ninstrument)
{
    if (EmptySlot(ninstrument))
    {
        return _defaultinsname;
    }

    return stringFrom(ninstrument + 1) + ". " + GetName(ninstrument);
}

/*
 * Changes the name of an instrument (and the filename)
 */
void BankManager::SetName(unsigned int ninstrument, const std::string &newname, int newslot)
{
    if (EmptySlot(ninstrument))
    {
        return;
    }

    std::string newfilename;
    char tmpfilename[100 + 1];
    tmpfilename[100] = 0;

    if (newslot >= 0)
    {
        snprintf(tmpfilename, 100, "%4d-%s", newslot + 1, newname.c_str());
    }
    else
    {
        snprintf(tmpfilename, 100, "%4d-%s", ninstrument + 1, newname.c_str());
    }

    //add the zeroes at the start of filename
    for (int i = 0; i < 4; ++i)
    {
        if (tmpfilename[i] == ' ')
        {
            tmpfilename[i] = '0';
        }
    }

    newfilename = _dirname + '/' + legalizeFilename(tmpfilename) + ".xiz";

    rename(_instrumentsInCurrentBank[ninstrument].filename.c_str(), newfilename.c_str());

    _instrumentsInCurrentBank[ninstrument].filename = newfilename;
    _instrumentsInCurrentBank[ninstrument].name = newname;
}

/*
 * Check if there is no instrument on a slot from the bank
 */
bool BankManager::EmptySlot(unsigned int ninstrument)
{
    if (ninstrument >= BANK_SIZE)
    {
        return true;
    }

    if (_instrumentsInCurrentBank[ninstrument].filename.empty())
    {
        return true;
    }

    return !_instrumentsInCurrentBank[ninstrument].used;
}

/*
 * Removes the instrument from the bank
 */
void BankManager::ClearSlot(unsigned int ninstrument)
{
    if (EmptySlot(ninstrument))
    {
        return;
    }

    remove(_instrumentsInCurrentBank[ninstrument].filename.c_str());
    DeleteFromBank(ninstrument);
}

/*
 * Save the instrument to a slot
 */
void BankManager::SaveToSlot(unsigned int ninstrument, Track *track)
{
    ClearSlot(ninstrument);

    const int maxfilename = 200;
    char tmpfilename[maxfilename + 20];
    ZERO(tmpfilename, maxfilename + 20);

    snprintf(tmpfilename, maxfilename, "%4d-%s", ninstrument + 1, track->Pname);

    //add the zeroes at the start of filename
    for (int i = 0; i < 4; ++i)
    {
        if (tmpfilename[i] == ' ')
        {
            tmpfilename[i] = '0';
        }
    }

    std::string filename = _dirname + '/' + legalizeFilename(tmpfilename) + ".xiz";

    remove(filename.c_str());

    SaveToFileSerializer().SaveTrack(track, filename);

    AddToBank(ninstrument, legalizeFilename(tmpfilename) + ".xiz", reinterpret_cast<char *>(track->Pname));
}

/*
 * Loads the instrument from the bank
 */
void BankManager::LoadFromSlot(unsigned int ninstrument, Track *track)
{
    if (EmptySlot(ninstrument))
    {
        return;
    }

    track->AllNotesOff();
    track->InstrumentDefaults();

    SaveToFileSerializer().LoadTrack(track, _instrumentsInCurrentBank[ninstrument].filename);
}

/*
 * Makes current a bank directory
 */
int BankManager::LoadBank(int index)
{
    if (static_cast<size_t>(index) >= banks.size())
    {
        return -1;
    }

    if (banks[static_cast<size_t>(index)].dir == _bankfiletitle)
    {
        return -1;
    }

    return LoadBankByDirectoryName(banks[static_cast<size_t>(index)].dir);
}

/*
 * Makes current a bank directory
 */
int BankManager::LoadBankByDirectoryName(std::string const &bankdirname)
{
    ClearBank();

    auto dir = std::filesystem::path(bankdirname);
    if (!std::filesystem::exists(dir))
    {
        return -1;
    }

    _dirname = bankdirname;

    _bankfiletitle = _dirname;

    for (auto const &dir_entry : std::filesystem::directory_iterator{dir})
    {
        auto filename = dir_entry.path().filename().string();

        //check for extension
        if (dir_entry.path().extension() != INSTRUMENT_EXTENSION)
        {
            continue;
        }

        //verify if the name is like this NNNN-name (where N is a digit)
        int no = 0;
        unsigned int startname = 0;

        for (unsigned int i = 0; i < 4; ++i)
        {
            if (filename.size() <= i)
            {
                break;
            }

            if ((filename[i] >= '0') && (filename[i] <= '9'))
            {
                no = no * 10 + (filename[i] - '0');
                startname++;
            }
        }

        if ((startname + 1) < filename.size())
        {
            startname++; //to take out the "-"
        }

        std::string name = filename;

        //remove the file extension
        for (unsigned int i = name.size() - 1; i >= 2; i--)
        {
            if (name[i] == '.')
            {
                name = name.substr(0, i);
                break;
            }
        }

        if (no != 0) //the instrument position in the bank is found
        {
            AddToBank(static_cast<unsigned int>(no - 1), filename, name.substr(startname));
        }
        else
        {
            AddToBank(0, filename, name);
        }
    }

    if (!_dirname.empty())
    {
        Config::Current().cfg.currentBankDir = _dirname;
    }

    return 0;
}

/*
 * Makes a new bank, put it on a file and makes it current bank
 */
int BankManager::NewBank(std::string const &newbankdirname)
{
    std::string bankdir;
    bankdir = Config::Current().cfg.bankRootDirList[0];

    if (((bankdir[bankdir.size() - 1]) != '/') && ((bankdir[bankdir.size() - 1]) != '\\'))
    {
        bankdir += "/";
    }

    bankdir += newbankdirname;

    if (!std::filesystem::create_directory(bankdir))
    {
        return -1;
    }

    const std::string tmpfilename = bankdir + '/' + FORCE_BANK_DIR_FILE;

    FILE *tmpfile = fopen(tmpfilename.c_str(), "w+");
    fclose(tmpfile);

    return LoadBankByDirectoryName(bankdir);
}

int BankManager::GetBankCount()
{
    return static_cast<int>(banks.size());
}

std::vector<char const *> const &BankManager::GetBankNames()
{
    if (_bankNames.empty())
    {
        for (auto &bank : banks)
        {
            _bankNames.push_back(bank.name.c_str());
        }
    }
    return _bankNames;
}

IBankManager::InstrumentBank &BankManager::GetBank(int index)
{
    if (index >= 0 && index < static_cast<int>(banks.size()))
    {
        return banks[static_cast<size_t>(index)];
    }

    static IBankManager::InstrumentBank emptyBank = {};

    return emptyBank;
}

std::string const &BankManager::GetBankFileTitle()
{
    return _bankfiletitle;
}

/*
 * Check if the bank is locked (i.e. the file opened was readonly)
 */
int BankManager::Locked()
{
    return _dirname.empty();
}

/*
 * Swaps a slot with another
 */
void BankManager::SwapSlot(unsigned int n1, unsigned int n2)
{
    if ((n1 == n2) || (Locked()))
    {
        return;
    }
    if (EmptySlot(n1) && (EmptySlot(n2)))
    {
        return;
    }
    if (EmptySlot(n1)) //change n1 to n2 in order to make
    {
        std::swap(n1, n2);
    }

    if (EmptySlot(n2))
    { //this is just a movement from slot1 to slot2
        SetName(n1, GetName(n1), static_cast<int>(n2));
        _instrumentsInCurrentBank[n2] = _instrumentsInCurrentBank[n1];
        _instrumentsInCurrentBank[n1] = ins_t();
    }
    else
    {                                                                                 //if both slots are used
        if (_instrumentsInCurrentBank[n1].name == _instrumentsInCurrentBank[n2].name) //change the name of the second instrument if the name are equal
        {
            _instrumentsInCurrentBank[n2].name += "2";
        }

        SetName(n1, GetName(n1), static_cast<int>(n2));
        SetName(n2, GetName(n2), static_cast<int>(n1));
        std::swap(_instrumentsInCurrentBank[n2], _instrumentsInCurrentBank[n1]);
    }
}

/*
 * Re-scan for directories containing instrument banks
 */
void BankManager::RescanForBanks()
{
    //remove old banks
    banks.clear();
    _bankNames.clear();

    for (auto &i : Config::Current().cfg.bankRootDirList)
    {
        if (!i.empty())
        {
            ScanRootDirectory(i);
        }
    }

    if (banks.empty())
    {
        return;
    }

    //sort the banks
    sort(banks.begin(), banks.end());

    //remove duplicate bank names
    int dupl = 0;
    for (unsigned int j = 0; j < banks.size() - 1; ++j)
    {
        for (unsigned int i = j + 1; i < banks.size(); ++i)
        {
            if (banks[i].name == banks[j].name)
            {
                //add a [1] to the first bankname and [n] to others
                banks[i].name = banks[i].name + '[' + stringFrom(dupl + 2) + ']';
                if (dupl == 0)
                {
                    banks[j].name += "[1]";
                }
                dupl++;
            }
            else
            {
                dupl = 0;
            }
        }
    }
}

// private stuff

void BankManager::ScanRootDirectory(
    std::string const &rootdir)
{
    auto dir = std::filesystem::path(rootdir);
    if (!std::filesystem::exists(dir))
    {
        return;
    }

    InstrumentBank bank;

    for (auto const &dir_entry : std::filesystem::directory_iterator{dir})
    {
        auto dirname = dir_entry.path().filename().string();
        if (dirname[0] == '.')
        {
            continue;
        }

        if (!std::filesystem::is_directory(dir_entry.path()))
        {
            continue;
        }

        bank.dir = (std::filesystem::path(rootdir) / dir_entry.path()).string();
        bank.name = dirname;
        bank.instrumentNames.clear();
        //find out if the directory contains at least 1 instrument
        bool isbank = false;

        auto d = std::filesystem::path(bank.dir.c_str());
        if (!std::filesystem::exists(d))
        {
            continue;
        }

        for (auto const &fname : std::filesystem::directory_iterator{d})
        {
            if ((fname.path().extension() == INSTRUMENT_EXTENSION) || (fname.path().filename().string() == FORCE_BANK_DIR_FILE))
            {
                isbank = true;
                std::string name = fname.path().filename().string();
                transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
                bank.instrumentNames.push_back(name);
            }
        }

        if (isbank)
        {
            banks.push_back(bank);
        }
    }
}

void BankManager::ClearBank()
{
    for (auto &in : _instrumentsInCurrentBank)
    {
        in = ins_t();
    }

    _bankfiletitle.clear();
    _dirname.clear();
}

int BankManager::AddToBank(unsigned int pos, std::string const &filename, std::string const &name)
{
    // Atually this is wrong input, but lets just reset
    if (pos >= BANK_SIZE)
    {
        pos = 0;
    }

    if (pos < BANK_SIZE && _instrumentsInCurrentBank[pos].used)
    {
        for (unsigned int i = 0; i < BANK_SIZE; i--)
        {
            if (!_instrumentsInCurrentBank[i].used)
            {
                pos = i;
                break;
            }
        }
    }
    if (pos >= BANK_SIZE)
    {
        return -1; //the bank is full
    }

    DeleteFromBank(pos);

    _instrumentsInCurrentBank[pos].used = true;
    _instrumentsInCurrentBank[pos].name = name;
    _instrumentsInCurrentBank[pos].filename = _dirname + '/' + filename;

    //see if PADsynth is used
    if (Config::Current().cfg.CheckPADsynth)
    {
        PresetsSerializer xml;
        xml.loadXMLfile(_instrumentsInCurrentBank[pos].filename);

        _instrumentsInCurrentBank[pos].info.PADsynth_used = xml.hasPadSynth();
    }
    else
    {
        _instrumentsInCurrentBank[pos].info.PADsynth_used = false;
    }

    return 0;
}

bool BankManager::isPADsynth_used(unsigned int ninstrument)
{
    if (Config::Current().cfg.CheckPADsynth == 0)
    {
        return false;
    }

    return _instrumentsInCurrentBank[ninstrument].info.PADsynth_used;
}

void BankManager::DeleteFromBank(unsigned int pos)
{
    if (pos >= BANK_SIZE)
    {
        return;
    }

    _instrumentsInCurrentBank[pos] = ins_t();
}

BankManager::ins_t::ins_t()
    : used(false), name(""), filename("")
{
    info.PADsynth_used = false;
}
