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
    Serialize(&xml);
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

    Defaults();
    Deserialize(&xml);

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

Preset::Preset(std::string const &name):_name(name),_type(PresetTypes::Container){}
Preset::Preset(Preset const &preset):_name(preset._name),_type(preset._type),valueReference(preset.valueReference){}
Preset::Preset(std::string const &name, unsigned char *value, unsigned char min, unsigned char max):_name(name),_type(PresetTypes::UnsignedChar)
{
    valueReference.uchar_v=value;
    _rangeMin.uchar_min = min;
    _rangeMax.uchar_max = max;
}
Preset::Preset(std::string const &name, unsigned short int *value, unsigned short int min, unsigned short int max):_name(name),_type(PresetTypes::UnsignedShort)
{
    valueReference.ushort_v=value;
    _rangeMin.ushort_min = min;
    _rangeMax.ushort_max = max;
}
Preset::Preset(std::string const &name, short int *value, short int min, short int max):_name(name),_type(PresetTypes::Short)
{
    valueReference.short_v=value;
    _rangeMin.short_min = min;
    _rangeMax.short_max = max;
}
Preset::Preset(std::string const &name, float *value, float min, float max):_name(name),_type(PresetTypes::Float)
{
    valueReference.float_v=value;
    _rangeMin.float_min = min;
    _rangeMax.float_max = max;
}
Preset::~Preset(){}

PresetTypes Preset::Type() const{return _type;}
    
Preset::operator unsigned char () const { return *valueReference.uchar_v; }
Preset::operator unsigned short int () const { return *valueReference.ushort_v; }
Preset::operator short int () const { return *valueReference.short_v; }
Preset::operator float () const { return *valueReference.float_v; }

void Preset::set(unsigned char v){*valueReference.uchar_v = v;}
void Preset::set(unsigned short int v){*valueReference.ushort_v = v;}
void Preset::set(short int v){*valueReference.short_v = v;}
void Preset::set(float v){*valueReference.float_v = v;}

PresetContainer::PresetContainer(std::string const &name):Preset(name){}
PresetContainer::PresetContainer(PresetContainer const &presets):Preset(presets._name), _presets(presets._presets){}
PresetContainer::~PresetContainer(){}
void PresetContainer::AddPreset(Preset const &preset){_presets.push_back(preset);}
void PresetContainer::AddPreset(std::string const &name, unsigned char *value, unsigned char min, unsigned char max){_presets.push_back(Preset(name,value,min,max));}
void PresetContainer::AddPreset(std::string const &name, unsigned short int *value, unsigned short int min, unsigned short int max){_presets.push_back(Preset(name,value,min,max));}
void PresetContainer::AddPreset(std::string const &name, short int *value, short int min, short int max){_presets.push_back(Preset(name,value,min,max));}
void PresetContainer::AddPreset(std::string const &name, float *value, float min, float max){_presets.push_back(Preset(name,value,min,max));}

void WrappedPresets::Serialize(IPresetsSerializer *xml)
{
    for (auto &preset : _presets)
    {
        switch (preset.Type())
        {
            case PresetTypes::UnsignedChar:
            {
                xml->addpar(_name.c_str(), *(valueReference.uchar_v));
                break;
            }
            case PresetTypes::UnsignedShort:
            {
                xml->addpar(_name.c_str(), *(valueReference.ushort_v));
                break;
            }
            case PresetTypes::Short:
            {
                xml->addpar(_name.c_str(), *(valueReference.short_v));
                break;
            }
            case PresetTypes::Float:
            {
                xml->addparreal(_name.c_str(), *(valueReference.float_v));
                break;
            }
            case PresetTypes::Container:
            {
                xml->beginbranch(_name.c_str());
                auto container = dynamic_cast<PresetContainer*>(&preset);
                container->Serialize(xml);
                xml->endbranch();
                break;
            }
        }
    }
}

void WrappedPresets::Deserialize(IPresetsSerializer *xml)
{

}