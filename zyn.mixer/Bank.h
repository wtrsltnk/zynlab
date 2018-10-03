/*
  ZynAddSubFX - a software synthesizer

  Bank.h - Instrument Bank
  Copyright (C) 2002-2005 Nasca Octavian Paul
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

#ifndef BANK_H
#define BANK_H

#include <string>
#include <vector>

//entries in a bank
#define BANK_SIZE 160

/**The instrument Bank*/
class Bank
{
public:
    /**Constructor*/
    Bank();
    virtual ~Bank();

    std::string GetName(unsigned int ninstrument);
    std::string GetNameNumbered(unsigned int ninstrument);
    //if newslot==-1 then this is ignored, else it will be put on that slot
    void SetName(unsigned int ninstrument, const std::string &newname, int newslot);

    bool isPADsynth_used(unsigned int ninstrument);

    /**returns true when slot is empty*/
    bool EmptySlot(unsigned int ninstrument);

    /**Empties out the selected slot*/
    void ClearSlot(unsigned int ninstrument);
    /**Saves the given Part to slot*/
    void SaveToSlot(unsigned int ninstrument, class Instrument *part);
    /**Loads the given slot into a Part*/
    void LoadFromSlot(unsigned int ninstrument, class Instrument *part);

    /**Swaps Slots*/
    void SwapSlot(unsigned int n1, unsigned int n2);

    int LoadBank(std::string const &bankdirname);
    int NewBank(std::string const &newbankdirname);

    std::string bankfiletitle; //this is shown on the UI of the bank (the title of the window)
    int Locked();

    void RescanForBanks();

    struct bankstruct
    {
        bool operator<(const bankstruct &b) const;
        std::string dir;
        std::string name;
        std::vector<std::string> instrumentNames;
    };

    std::vector<bankstruct> banks;

    struct banksearchstruct
    {
        std::string shortBankName;
        std::string fullBankName;
        int instrumentSlot;
        std::string instrumentName;
    };

    std::vector<banksearchstruct> search(const char *searchFor);

private:
    //it adds a filename to the bank
    //if pos is -1 it try to find a position
    //returns -1 if the bank is full, or 0 if the instrument was added
    int AddToBank(unsigned int pos, std::string const &filename, std::string const &name);

    void DeleteFromBank(unsigned int pos);

    void ClearBank();

    std::string defaultinsname;

    struct ins_t
    {
        ins_t();
        bool used;
        std::string name;
        std::string filename;
        struct
        {
            bool PADsynth_used;
        } info{};
    } ins[BANK_SIZE];

    std::string dirname;

    void ScanRootDirectory(std::string const &rootdir); //scans a root dir for banks
};

#endif
