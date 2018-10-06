/*
  ZynAddSubFX - a software synthesizer

  Presets.cpp - Presets and Clipboard management
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

#include "Presets.h"
#include "PresetsSerializer.h"
#include <string>

Presets::Presets()
{
    type[0] = 0;
}

Presets::~Presets() = default;

void Presets::setpresettype(const char *type)
{
    strcpy(this->type, type);
}

void Presets::copy(const char *name)
{
    PresetsSerializer xml;

    //used only for the clipboard
    if (name == nullptr)
    {
        xml.minimal = false;
    }

    char type[MAX_PRESETTYPE_SIZE];
    strcpy(type, this->type);

    if (name == nullptr)
    {
        if (strstr(type, "Plfo") != nullptr)
        {
            strcpy(type, "Plfo");
        }
    }

    xml.beginbranch(type);
    add2XML(&xml);
    xml.endbranch();

    if (name == nullptr)
    {
        presetsstore.CopyClipboard(&xml, type);
    }
    else
    {
        presetsstore.CopyPreset(&xml, type, name);
    }
}

void Presets::paste(int npreset)
{
    char type[MAX_PRESETTYPE_SIZE];
    strcpy(type, this->type);

    if (npreset == 0)
    {
        if (strstr(type, "Plfo") != nullptr)
        {
            strcpy(type, "Plfo");
        }
    }

    PresetsSerializer xml;
    if (npreset == 0)
    {
        if (!checkclipboardtype())
        {
            return;
        }
        if (!presetsstore.PasteClipboard(&xml))
        {
            return;
        }
    }
    else if (!presetsstore.PastePreset(&xml, npreset))
    {
        return;
    }

    if (xml.enterbranch(type) == 0)
    {
        return;
    }

    defaults();
    getfromXML(&xml);

    xml.exitbranch();
}

bool Presets::checkclipboardtype()
{
    return presetsstore.CheckClipboardType(type);
}

void Presets::rescanforpresets()
{
    presetsstore.RescaneForPresets(type);
}

void Presets::deletepreset(int npreset)
{
    presetsstore.DeletePreset(npreset);
}
