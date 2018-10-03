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

#include "Bank.h"
#include "Instrument.h"
#include <algorithm>
#include <dirent.h>
#include <iostream>
#include <string>
#include <sys/stat.h>

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <zyn.common/Config.h>
#include <zyn.common/Util.h>

#define INSTRUMENT_EXTENSION ".xiz"

//if this file exists into a directory, this make the directory to be considered as a bank, even if it not contains a instrument file
#define FORCE_BANK_DIR_FILE ".bankdir"

Bank::Bank()
    : defaultinsname("defaults")
{
    ClearBank();
    bankfiletitle = dirname;
    LoadBank(Config::Current().cfg.currentBankDir);
}

Bank::~Bank()
{
    ClearBank();
}

/*
 * Get the name of an instrument from the bank
 */
std::string Bank::GetName(unsigned int ninstrument)
{
    if (EmptySlot(ninstrument))
    {
        return defaultinsname;
    }

    return ins[ninstrument].name;
}

/*
 * Get the numbered name of an instrument from the bank
 */
std::string Bank::GetNameNumbered(unsigned int ninstrument)
{
    if (EmptySlot(ninstrument))
    {
        return defaultinsname;
    }

    return stringFrom(ninstrument + 1) + ". " + GetName(ninstrument);
}

/*
 * Changes the name of an instrument (and the filename)
 */
void Bank::SetName(unsigned int ninstrument, const std::string &newname, int newslot)
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

    newfilename = dirname + '/' + legalizeFilename(tmpfilename) + ".xiz";

    rename(ins[ninstrument].filename.c_str(), newfilename.c_str());

    ins[ninstrument].filename = newfilename;
    ins[ninstrument].name = newname;
}

/*
 * Check if there is no instrument on a slot from the bank
 */
bool Bank::EmptySlot(unsigned int ninstrument)
{
    if (ninstrument >= BANK_SIZE)
    {
        return true;
    }

    if (ins[ninstrument].filename.empty())
    {
        return true;
    }

    return !ins[ninstrument].used;
}

/*
 * Removes the instrument from the bank
 */
void Bank::ClearSlot(unsigned int ninstrument)
{
    if (EmptySlot(ninstrument))
    {
        return;
    }

    remove(ins[ninstrument].filename.c_str());
    DeleteFromBank(ninstrument);
}

/*
 * Save the instrument to a slot
 */
void Bank::SaveToSlot(unsigned int ninstrument, Instrument *part)
{
    ClearSlot(ninstrument);

    const int maxfilename = 200;
    char tmpfilename[maxfilename + 20];
    ZERO(tmpfilename, maxfilename + 20);

    snprintf(tmpfilename,
             maxfilename,
             "%4d-%s",
             ninstrument + 1,
             reinterpret_cast<char *>(part->Pname));

    //add the zeroes at the start of filename
    for (int i = 0; i < 4; ++i)
    {
        if (tmpfilename[i] == ' ')
        {
            tmpfilename[i] = '0';
        }
    }

    std::string filename = dirname + '/' + legalizeFilename(tmpfilename) + ".xiz";

    remove(filename.c_str());
    part->saveXML(filename.c_str());
    AddToBank(ninstrument, legalizeFilename(tmpfilename) + ".xiz", reinterpret_cast<char *>(part->Pname));
}

/*
 * Loads the instrument from the bank
 */
void Bank::LoadFromSlot(unsigned int ninstrument, Instrument *part)
{
    if (EmptySlot(ninstrument))
    {
        return;
    }

    part->AllNotesOff();
    part->defaultsinstrument();

    part->loadXMLinstrument(ins[ninstrument].filename.c_str());
}

/*
 * Makes current a bank directory
 */
int Bank::LoadBank(std::string const &bankdirname)
{
    DIR *dir = opendir(bankdirname.c_str());
    ClearBank();

    if (dir == nullptr)
    {
        return -1;
    }

    dirname = bankdirname;

    bankfiletitle = dirname;

    struct dirent *fn;

    while ((fn = readdir(dir)))
    {
        const char *filename = fn->d_name;

        //check for extension
        if (strstr(filename, INSTRUMENT_EXTENSION) == nullptr)
        {
            continue;
        }

        //verify if the name is like this NNNN-name (where N is a digit)
        int no = 0;
        unsigned int startname = 0;

        for (unsigned int i = 0; i < 4; ++i)
        {
            if (strlen(filename) <= i)
            {
                break;
            }

            if ((filename[i] >= '0') && (filename[i] <= '9'))
            {
                no = no * 10 + (filename[i] - '0');
                startname++;
            }
        }

        if ((startname + 1) < strlen(filename))
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

    closedir(dir);

    if (!dirname.empty())
    {
        Config::Current().cfg.currentBankDir = dirname;
    }

    return 0;
}

/*
 * Makes a new bank, put it on a file and makes it current bank
 */
int Bank::NewBank(std::string const &newbankdirname)
{
    std::string bankdir;
    bankdir = Config::Current().cfg.bankRootDirList[0];

    if (((bankdir[bankdir.size() - 1]) != '/') && ((bankdir[bankdir.size() - 1]) != '\\'))
    {
        bankdir += "/";
    }

    bankdir += newbankdirname;
#ifndef _WIN32
    if (mkdir(bankdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
#else
    if (mkdir(bankdir.c_str()) < 0)
#endif
    {
        return -1;
    }

    const std::string tmpfilename = bankdir + '/' + FORCE_BANK_DIR_FILE;

    FILE *tmpfile = fopen(tmpfilename.c_str(), "w+");
    fclose(tmpfile);

    return LoadBank(bankdir);
}

/*
 * Check if the bank is locked (i.e. the file opened was readonly)
 */
int Bank::Locked()
{
    return dirname.empty();
}

/*
 * Swaps a slot with another
 */
void Bank::SwapSlot(unsigned int n1, unsigned int n2)
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
        ins[n2] = ins[n1];
        ins[n1] = ins_t();
    }
    else
    {                                     //if both slots are used
        if (ins[n1].name == ins[n2].name) //change the name of the second instrument if the name are equal
        {
            ins[n2].name += "2";
        }

        SetName(n1, GetName(n1), static_cast<int>(n2));
        SetName(n2, GetName(n2), static_cast<int>(n1));
        std::swap(ins[n2], ins[n1]);
    }
}

bool Bank::bankstruct::operator<(const bankstruct &b) const
{
    return name < b.name;
}

/*
 * Re-scan for directories containing instrument banks
 */

void Bank::RescanForBanks()
{
    //remove old banks
    banks.clear();

    for (auto &i : Config::Current().cfg.bankRootDirList)
    {
        if (!i.empty())
        {
            ScanRootDirectory(i);
        }
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

std::vector<Bank::banksearchstruct> Bank::search(const char *searchFor)
{
    std::vector<banksearchstruct> result;
    std::string strSearchFor(searchFor);
    transform(strSearchFor.begin(), strSearchFor.end(), strSearchFor.begin(), [](unsigned char c) { return std::tolower(c); });

    for (bankstruct b : this->banks)
    {
        for (std::string const &i : b.instrumentNames)
        {
            if (i.find(strSearchFor) != std::string::npos)
            {
                int slotnr = 0;
                sscanf(i.c_str(), "%4d", &slotnr);
                banksearchstruct sr = {b.name, b.dir, slotnr, i};
                result.push_back(sr);
            }
        }
    }

    return result;
}

// private stuff

void Bank::ScanRootDirectory(std::string const &rootdir)
{
    DIR *dir = opendir(rootdir.c_str());
    if (dir == nullptr)
    {
        return;
    }

    bankstruct bank;

    const char *separator = "/";
    if (!rootdir.empty())
    {
        char tmp = rootdir[rootdir.size() - 1];
        if ((tmp == '/') || (tmp == '\\'))
        {
            separator = "";
        }
    }

    struct dirent *fn;
    while ((fn = readdir(dir)))
    {
        const char *dirname = fn->d_name;
        if (dirname[0] == '.')
        {
            continue;
        }

        bank.dir = rootdir + separator + dirname + '/';
        bank.name = dirname;
        bank.instrumentNames.clear();
        //find out if the directory contains at least 1 instrument
        bool isbank = false;

        DIR *d = opendir(bank.dir.c_str());
        if (d == nullptr)
        {
            continue;
        }

        struct dirent *fname;

        while ((fname = readdir(d)))
        {
            if ((strstr(fname->d_name, INSTRUMENT_EXTENSION) != nullptr) || (strstr(fname->d_name, FORCE_BANK_DIR_FILE) != nullptr))
            {
                isbank = true;
                std::string name(fname->d_name);
                transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
                bank.instrumentNames.push_back(name);
            }
        }

        if (isbank)
            banks.push_back(bank);

        closedir(d);
    }

    closedir(dir);
}

void Bank::ClearBank()
{
    for (auto &in : ins)
    {
        in = ins_t();
    }

    bankfiletitle.clear();
    dirname.clear();
}

int Bank::AddToBank(unsigned int pos, std::string const &filename, std::string const &name)
{
    // Atually this is wrong input, but lets just reset
    if (pos >= BANK_SIZE)
    {
        pos = 0;
    }

    if (pos < BANK_SIZE && ins[pos].used)
    {
        for (unsigned int i = 0; i < BANK_SIZE; i--)
        {
            if (!ins[i].used)
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

    ins[pos].used = true;
    ins[pos].name = name;
    ins[pos].filename = dirname + '/' + filename;

    //see if PADsynth is used
    if (Config::Current().cfg.CheckPADsynth)
    {
        XMLwrapper xml;
        xml.loadXMLfile(ins[pos].filename);

        ins[pos].info.PADsynth_used = xml.hasPadSynth();
    }
    else
    {
        ins[pos].info.PADsynth_used = false;
    }

    return 0;
}

bool Bank::isPADsynth_used(unsigned int ninstrument)
{
    if (Config::Current().cfg.CheckPADsynth == 0)
    {
        return false;
    }

    return ins[ninstrument].info.PADsynth_used;
}

void Bank::DeleteFromBank(unsigned int pos)
{
    if (pos >= BANK_SIZE)
    {
        return;
    }

    ins[pos] = ins_t();
}

Bank::ins_t::ins_t()
    : used(false), name(""), filename("")
{
    info.PADsynth_used = false;
}
