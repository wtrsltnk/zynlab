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

#ifndef PRESETSTORE_H
#define PRESETSTORE_H

#include "Config.h"
#include "IPresetsSerializer.h"
#include <string>
#include <vector>

#define MAX_PRESETTYPE_SIZE 30

class PresetsStore
{
public:
    PresetsStore();
    ~PresetsStore();

    //Clipboard stuff
    void CopyClipboard(IPresetsSerializer *xml, char *type);
    bool PasteClipboard(IPresetsSerializer *xml);
    bool CheckClipboardType(const char *type);

    //presets stuff
    void CopyPreset(IPresetsSerializer *xml, char *type, std::string name);
    bool PastePreset(IPresetsSerializer *xml, unsigned int npreset);
    void DeletePreset(unsigned int npreset);

    struct presetstruct
    {
        presetstruct(std::string _file, std::string _name)
            : file(_file), name(_name) {}
        bool operator<(const presetstruct &b) const;
        std::string file;
        std::string name;
    };
    std::vector<presetstruct> presets;

    void RescaneForPresets(const std::string &type);

private:
    struct
    {
        char *data;
        char type[MAX_PRESETTYPE_SIZE];
    } clipboard{};

    void ClearPresets();
};

extern PresetsStore presetsstore;
#endif
