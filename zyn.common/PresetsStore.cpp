/*
  ZynAddSubFX - a software synthesizer

  PresetsStore.cpp - Presets and Clipboard store
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

#include "PresetsStore.h"
#include "Util.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include <sys/stat.h>

using namespace std;

PresetsStore presetsstore;

PresetsStore::PresetsStore()
{
    clipboard.data = nullptr;
    clipboard.type[0] = 0;
}

PresetsStore::~PresetsStore()
{
    if (clipboard.data != nullptr)
    {
        free(clipboard.data);
    }
    ClearPresets();
}

// Clipboard management

void PresetsStore::CopyClipboard(IPresetsSerializer *xml, char const *type)
{
    strcpy(clipboard.type, type);
    if (clipboard.data != nullptr)
    {
        free(clipboard.data);
    }
    clipboard.data = xml->getXMLdata();
}

bool PresetsStore::PasteClipboard(IPresetsSerializer *xml)
{
    if (clipboard.data != nullptr)
    {
        xml->putXMLdata(clipboard.data);
        return true;
    }

    return false;
}

bool PresetsStore::CheckClipboardType(const char *type)
{
    // makes LFO's compatible
    if ((strstr(type, "Plfo") != nullptr) && (strstr(clipboard.type, "Plfo") != nullptr))
    {
        return true;
    }

    return strcmp(type, clipboard.type) == 0;
}

// Presets management
void PresetsStore::ClearPresets()
{
    presets.clear();
}

// a helper function that compares 2 presets[]
bool PresetsStore::presetstruct::operator<(const presetstruct &b) const
{
    return name < b.name;
}

void PresetsStore::RescaneForPresets(const string &type)
{
    ClearPresets();
    string ftype = "." + type.substr(1) + ".xpz";

    for (auto &i : Config::Current().cfg.presetsDirList)
    {
        if (i.empty())
        {
            continue;
        }

        std::filesystem::path dir(i);

        if (!std::filesystem::exists(dir))
        {
            continue;
        }

        for (auto &p : std::filesystem::directory_iterator(dir))
        {
            if (p.path().string().find(ftype) == string::npos)
            {
                continue;
            }

            auto name = p.path().stem().string();

            presets.emplace_back(p.path().string(), name);
        }

        //        //open directory
        //        string dirname = i;
        //        DIR *dir = opendir(dirname.c_str());
        //        if (dir == nullptr)
        //        {
        //            continue;
        //        }
        //        struct dirent *fn;

        //        //check all files in directory
        //        while ((fn = readdir(dir)))
        //        {
        //            string filename = fn->d_name;
        //            if (filename.find(ftype) == string::npos)
        //            {
        //                continue;
        //            }

        //            //ensure proper path is formed
        //            char tmpc = dirname[dirname.size() - 1];
        //            const char *tmps;
        //            if ((tmpc == '/') || (tmpc == '\\'))
        //            {
        //                tmps = "";
        //            }
        //            else
        //            {
        //                tmps = "/";
        //            }

        //            string location = "" + dirname + tmps + filename;

        //            //trim file type off of name
        //            string name = filename.substr(0, filename.find(ftype));

        //            //put on list
        //            presets.emplace_back(location, name);
        //        }

        //        closedir(dir);
    }

    // sort the presets
    sort(presets.begin(), presets.end());
}

void PresetsStore::CopyPreset(IPresetsSerializer *xml, char const *type, string name)
{
    if (Config::Current().cfg.presetsDirList[0].empty())
    {
        return;
    }

    // make the filenames legal
    name = legalizeFilename(name);

    // make path legal
    const string dirname = Config::Current().cfg.presetsDirList[0];
    char tmpc = dirname[dirname.size() - 1];
    const char *tmps;
    if ((tmpc == '/') || (tmpc == '\\'))
    {
        tmps = "";
    }
    else
    {
        tmps = "/";
    }

    string filename("" + dirname + tmps + name + "." + &type[1] + ".xpz");

    xml->saveXMLfile(filename);
}

bool PresetsStore::PastePreset(IPresetsSerializer *xml, unsigned int npreset)
{
    npreset--;
    if (npreset >= presets.size())
    {
        return false;
    }

    string filename = presets[npreset].file;
    if (filename.empty())
    {
        return false;
    }

    return xml->loadXMLfile(filename) >= 0;
}

void PresetsStore::DeletePreset(unsigned int npreset)
{
    npreset--;
    if (npreset >= presets.size())
    {
        return;
    }

    string filename = presets[npreset].file;
    if (filename.empty())
    {
        return;
    }
    remove(filename.c_str());
}
