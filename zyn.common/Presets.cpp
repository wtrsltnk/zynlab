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

Presets::Presets() = default;

Presets::~Presets() = default;

void Presets::setpresettype(const char *type)
{
    _type = type;
}

void Presets::copy(const char *name)
{
    PresetsSerializer xml;

    //used only for the clipboard
    if (name == nullptr)
    {
        xml.minimal = false;
    }

    std::string type = this->_type;

    if (name == nullptr)
    {
        if (type.find("Plfo") != std::string::npos)
        {
            type = "Plfo";
        }
    }

    xml.beginbranch(type);
    Serialize(&xml);
    xml.endbranch();

    if (name == nullptr)
    {
        presetsstore.CopyClipboard(&xml, type.c_str());
    }
    else
    {
        presetsstore.CopyPreset(&xml, type.c_str(), name);
    }
}

void Presets::paste(unsigned int npreset)
{
    std::string type = this->_type;

    if (npreset == 0)
    {
        if (type.find("Plfo") != std::string::npos)
        {
            type = "Plfo";
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
    return presetsstore.CheckClipboardType(_type.c_str());
}

void Presets::rescanforpresets()
{
    presetsstore.RescaneForPresets(_type);
}

void Presets::deletepreset(unsigned int npreset)
{
    presetsstore.DeletePreset(npreset);
}

Preset::Preset(std::string const &name) : _name(name), _type(PresetTypes::Container), _id(-1) {}
Preset::Preset(std::string const &name, int id) : _name(name), _type(PresetTypes::Container), _id(id) {}
Preset::Preset(std::string const &name, Preset const &container)
    : _name(name), _type(PresetTypes::Container), _id(container._id),
      valueReference(container.valueReference),
      _rangeMin(container._rangeMin), _rangeMax(container._rangeMax),
      _presets(container._presets)
{}
Preset::Preset(std::string const &name, int id, Preset const &container)
    : _name(name), _type(PresetTypes::Container), _id(id),
      valueReference(container.valueReference),
      _rangeMin(container._rangeMin), _rangeMax(container._rangeMax),
      _presets(container._presets)
{}
Preset::Preset(Preset const &preset)
    : _name(preset._name), _type(preset._type), _id(preset._id),
      valueReference(preset.valueReference),
      _rangeMin(preset._rangeMin), _rangeMax(preset._rangeMax),
      _presets(preset._presets)
{}
Preset::Preset(std::string const &name, char *value, int max)
    : _name(name), _type(PresetTypes::String), _id(-1)
{
    valueReference.string_v = value;
    _rangeMax.string_max = max;
}
Preset::Preset(std::string const &name, unsigned char *value, unsigned char min, unsigned char max)
    : _name(name), _type(PresetTypes::UnsignedChar), _id(-1)
{
    valueReference.uchar_v = value;
    _rangeMin.uchar_min = min;
    _rangeMax.uchar_max = max;
    if (min == 0 && max == 1)
    {
        _type = PresetTypes::Boolean;
    }
}
Preset::Preset(std::string const &name, unsigned short int *value, unsigned short int min, unsigned short int max)
    : _name(name), _type(PresetTypes::UnsignedShort), _id(-1)
{
    valueReference.ushort_v = value;
    _rangeMin.ushort_min = min;
    _rangeMax.ushort_max = max;
}
Preset::Preset(std::string const &name, unsigned int *value, unsigned int min, unsigned int max)
    : _name(name), _type(PresetTypes::UnsignedInt), _id(-1)
{
    valueReference.uint_v = value;
    _rangeMin.uint_min = min;
    _rangeMax.uint_max = max;
}
Preset::Preset(std::string const &name, int *value, int min, int max)
    : _name(name), _type(PresetTypes::Int), _id(-1)
{
    valueReference.int_v = value;
    _rangeMin.int_min = min;
    _rangeMax.int_max = max;
}
Preset::Preset(std::string const &name, short int *value, short int min, short int max)
    : _name(name), _type(PresetTypes::Short), _id(-1)
{
    valueReference.short_v = value;
    _rangeMin.short_min = min;
    _rangeMax.short_max = max;
}
Preset::Preset(std::string const &name, float *value, float min, float max)
    : _name(name), _type(PresetTypes::Float), _id(-1)
{
    valueReference.float_v = value;
    _rangeMin.float_min = min;
    _rangeMax.float_max = max;
}
Preset::~Preset() {}

PresetTypes Preset::Type() const { return _type; }

std::string const &Preset::Name() const { return _name; }

int Preset::Id() const { return _id; }

void Preset::Id(int id) { _id = id; }

Preset::operator char *const() const { return valueReference.string_v; }
Preset::operator unsigned char() const { return *valueReference.uchar_v; }
Preset::operator unsigned short int() const { return *valueReference.ushort_v; }
Preset::operator unsigned int() const { return *valueReference.uint_v; }
Preset::operator int() const { return *valueReference.int_v; }
Preset::operator short int() const { return *valueReference.short_v; }
Preset::operator float() const { return *valueReference.float_v; }

void Preset::set(char *const v) { strcpy(valueReference.string_v, v); }
void Preset::set(unsigned char v) { *valueReference.uchar_v = v; }
void Preset::set(unsigned short int v) { *valueReference.ushort_v = v; }
void Preset::set(unsigned int v) { *valueReference.uint_v = v; }
void Preset::set(int v) { *valueReference.int_v = v; }
void Preset::set(short int v) { *valueReference.short_v = v; }
void Preset::set(float v) { *valueReference.float_v = v; }

Preset &Preset::AddPreset(Preset const &preset)
{
    _presets.push_back(Preset(preset));
    return *this;
}
Preset &Preset::AddPreset(std::string const &name, char *value, int max)
{
    _presets.push_back(Preset(name, value, max));
    return *this;
}
Preset &Preset::AddPreset(std::string const &name, unsigned char *value, unsigned char min, unsigned char max)
{
    _presets.push_back(Preset(name, value, min, max));
    return *this;
}
Preset &Preset::AddPreset(std::string const &name, unsigned short int *value, unsigned short int min, unsigned short int max)
{
    _presets.push_back(Preset(name, value, min, max));
    return *this;
}
Preset &Preset::AddPreset(std::string const &name, unsigned int *value, unsigned int min, unsigned int max)
{
    _presets.push_back(Preset(name, value, min, max));
    return *this;
}
Preset &Preset::AddPreset(std::string const &name, int *value, int min, int max)
{
    _presets.push_back(Preset(name, value, min, max));
    return *this;
}
Preset &Preset::AddPreset(std::string const &name, short int *value, short int min, short int max)
{
    _presets.push_back(Preset(name, value, min, max));
    return *this;
}
Preset &Preset::AddPreset(std::string const &name, float *value, float min, float max)
{
    _presets.push_back(Preset(name, value, min, max));
    return *this;
}
Preset &Preset::AddPresetAsBool(std::string const &name, unsigned char *value)
{
    _presets.push_back(Preset(name, value, 0, 1));
    return *this;
}
Preset &Preset::AddContainer(Preset const &container)
{
    _presets.push_back(Preset(container));
    return *this;
}
Preset &Preset::AddContainer(int index, Preset const &container)
{
    _presets.push_back(Preset(container));
    _presets.back().Id(index);
    return *this;
}
Preset &Preset::AddContainer(std::string const &name, int index, Preset const &container)
{
    _presets.push_back(Preset(name, container));
    _presets.back().Id(index);
    return *this;
}
Preset &Preset::AddContainer(std::string const &name, Preset const &container)
{
    _presets.push_back(Preset(name, container));
    return *this;
}

void Preset::WriteToBlob(IPresetsSerializer *xml)
{
    switch (Type())
    {
        case PresetTypes::String:
        {
            if (valueReference.string_v == nullptr) break;
            xml->addparstr(Name(), valueReference.string_v);
            break;
        }
        case PresetTypes::UnsignedChar:
        {
            if (valueReference.uchar_v == nullptr) break;
            xml->addpar(Name(), static_cast<int>(*(valueReference.uchar_v)));
            break;
        }
        case PresetTypes::UnsignedShort:
        {
            if (valueReference.ushort_v == nullptr) break;
            xml->addpar(Name(), *(valueReference.ushort_v));
            break;
        }
        case PresetTypes::UnsignedInt:
        {
            if (valueReference.uint_v == nullptr) break;
            xml->addparunsigned(Name(), *(valueReference.uint_v));
            break;
        }
        case PresetTypes::Int:
        {
            if (valueReference.int_v == nullptr) break;
            xml->addpar(Name(), *(valueReference.int_v));
            break;
        }
        case PresetTypes::Short:
        {
            if (valueReference.short_v == nullptr) break;
            xml->addpar(Name(), *(valueReference.short_v));
            break;
        }
        case PresetTypes::Float:
        {
            if (valueReference.float_v == nullptr) break;
            xml->addparreal(Name(), *(valueReference.float_v));
            break;
        }
        case PresetTypes::Boolean:
        {
            if (valueReference.uchar_v == nullptr) break;
            xml->addparbool(Name(), static_cast<int>(*valueReference.uchar_v));
            break;
        }
        case PresetTypes::Container:
        {
            if (Id() != -1)
            {
                xml->beginbranch(Name(), Id());
            }
            else
            {
                xml->beginbranch(Name());
            }

            for (auto &preset : _presets)
            {
                preset.WriteToBlob(xml);
            }
            xml->endbranch();
            break;
        }
    }
}

void Preset::ReadFromBlob(IPresetsSerializer *xml)
{
    switch (Type())
    {
        case PresetTypes::String:
        {
            if (valueReference.string_v == nullptr) break;
            xml->getparstr(Name(), valueReference.string_v, _rangeMax.string_max);
            break;
        }
        case PresetTypes::UnsignedChar:
        {
            if (valueReference.uchar_v == nullptr) break;
            *(valueReference.uchar_v) = xml->getpar127(Name(), *(valueReference.uchar_v));
            break;
        }
        case PresetTypes::UnsignedShort:
        {
            if (valueReference.ushort_v == nullptr) break;
            *(valueReference.ushort_v) = static_cast<unsigned short int>(xml->getpar(Name(), static_cast<unsigned short int>(*(valueReference.ushort_v)), _rangeMin.ushort_min, _rangeMax.ushort_max));
            break;
        }
        case PresetTypes::UnsignedInt:
        {
            if (valueReference.uint_v == nullptr) break;
            *(valueReference.uint_v) = static_cast<unsigned int>(xml->getparunsigned(Name(), static_cast<unsigned int>(*(valueReference.uint_v)), _rangeMin.uint_min, _rangeMax.uint_max));
            break;
        }
        case PresetTypes::Int:
        {
            if (valueReference.int_v == nullptr) break;
            *(valueReference.int_v) = static_cast<int>(xml->getpar(Name(), *(valueReference.int_v), _rangeMin.int_min, _rangeMax.int_max));
            break;
        }
        case PresetTypes::Short:
        {
            if (valueReference.short_v == nullptr) break;
            *(valueReference.short_v) = static_cast<short int>(xml->getpar(Name(), static_cast<int>(*(valueReference.short_v)), _rangeMin.short_min, _rangeMax.short_max));
            break;
        }
        case PresetTypes::Float:
        {
            if (valueReference.float_v == nullptr) break;
            *(valueReference.float_v) = xml->getparreal(Name(), *(valueReference.float_v));
            break;
        }
        case PresetTypes::Boolean:
        {
            if (valueReference.uchar_v == nullptr) break;
            *valueReference.uchar_v = static_cast<unsigned char>(xml->getparbool(Name(), static_cast<int>(*valueReference.uchar_v)));
            break;
        }
        case PresetTypes::Container:
        {
            if (xml->enterbranch(Name(), Id()) == 0)
            {
                break;
            }

            for (auto &preset : _presets)
            {
                preset.ReadFromBlob(xml);
            }

            xml->exitbranch();
            break;
        }
    }
}

void Preset::WritePresetsToBlob(IPresetsSerializer *xml)
{
    for (auto &preset : _presets)
    {
        preset.WriteToBlob(xml);
    }
}

void Preset::ReadPresetsFromBlob(IPresetsSerializer *xml)
{
    for (auto &preset : _presets)
    {
        preset.ReadFromBlob(xml);
    }
}

WrappedPresets::WrappedPresets() : Preset("tbd") {}

WrappedPresets::~WrappedPresets() {}
