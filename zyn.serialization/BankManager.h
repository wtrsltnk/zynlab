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

#ifndef BANKMANAGER_H
#define BANKMANAGER_H

#include <string>
#include <vector>
#include <zyn.common/globals.h>

class Track;

/**The instrument Bank*/
class BankManager : public IBankManager
{
public:
    /**Constructor*/
    BankManager();
    virtual ~BankManager();

    virtual std::string GetName(
        unsigned int ninstrument);

    virtual std::string GetNameNumbered(
        unsigned int ninstrument);

    virtual std::string GetInstrumentPath(
        unsigned int ninstrument);

    //if newslot==-1 then this is ignored, else it will be put on that slot
    virtual void SetName(
        unsigned int ninstrument,
        const std::string &newname,
        int newslot);

    virtual bool isPADsynth_used(
        unsigned int ninstrument);

    /**returns true when slot is empty*/
    virtual bool EmptySlot(
        unsigned int ninstrument);

    /**Empties out the selected slot*/
    virtual void ClearSlot(
        unsigned int ninstrument);

    /**Saves the given Part to slot*/
    virtual void SaveToSlot(
        unsigned int ninstrument,
        Track *track);

    /**Loads the given slot into a Part*/
    virtual void LoadFromSlot(
        unsigned int ninstrument,
        Track *track);

    /**Swaps Slots*/
    virtual void SwapSlot(
        unsigned int n1,
        unsigned int n2);

    virtual int LoadBank(
        int index);

    virtual int NewBank(
        std::string const &newbankdirname);

    virtual int GetBankCount();

    virtual std::vector<char const *> const &GetBankNames();

    virtual InstrumentBank &GetBank(
        int index);

    virtual std::string const &GetBankFileTitle();

    virtual int Locked();

    virtual void RescanForBanks();

    std::vector<InstrumentBank> banks;

private:
    //it adds a filename to the bank
    //if pos is -1 it try to find a position
    //returns -1 if the bank is full, or 0 if the instrument was added
    int AddToBank(
        unsigned int pos,
        std::string const &filename,
        std::string const &name);

    int LoadBankByDirectoryName(
        std::string const &bankdirname);

    void DeleteFromBank(
        unsigned int pos);

    void ClearBank();

    //scans a root dir for banks
    void ScanRootDirectory(
        std::string const &rootdir);

    std::string _defaultinsname;
    std::string _bankfiletitle; //this is shown on the UI of the bank (the title of the window)
    std::vector<char const *> _bankNames;
    std::string _dirname;

    struct ins_t
    {
        ins_t();
        bool used;
        std::string name;
        std::string filename;
        struct
        {
            bool PADsynth_used;
        } info;
    } _instrumentsInCurrentBank[BANK_SIZE];
};

#endif // BANKMANAGER_H
