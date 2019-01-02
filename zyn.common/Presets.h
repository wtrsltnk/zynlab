/*
  ZynAddSubFX - a software synthesizer

  Presets.h - Presets and Clipboard management
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

#ifndef PRESETS_H
#define PRESETS_H

#include "IPresetsSerializer.h"
#include "PresetsStore.h"
#include <string>
#include <vector>

enum class PresetTypes
{
    UnsignedChar,
    UnsignedShort,
    Short,
    Float,
    Container,
};

class Preset
{
protected:
    PresetTypes _type;
    union
    {
        unsigned char *uchar_v;
        unsigned short int *ushort_v;
        short int *short_v;
        float *float_v;
    } valueReference;
    union
    {
        unsigned char uchar_min;
        unsigned short int ushort_min;
        short int short_min;
        float float_min;
    } _rangeMin;
    union
    {
        unsigned char uchar_max;
        unsigned short int ushort_max;
        short int short_max;
        float float_max;
    } _rangeMax;

    std::string _name;

    Preset(std::string const &name);
public:
    Preset(Preset const &preset);
    Preset(std::string const &name, unsigned char *value, unsigned char min = 0, unsigned char max = 127);
    Preset(std::string const &name, unsigned short int *value, unsigned short int min = 0, unsigned short int max = 16383);
    Preset(std::string const &name, short int *value, short int min = 0, short int max = 16383);
    Preset(std::string const &name, float *value, float min = 0.0f, float max = 1.0f);
    virtual ~Preset();

    PresetTypes Type() const;

    operator unsigned char () const;
    operator unsigned short int () const;
    operator short int () const;
    operator float () const;

    void set(unsigned char v);
    void set(unsigned short int v);
    void set(short int v);
    void set(float v);
};

class PresetContainer : public Preset
{
protected:
    std::vector<Preset> _presets;
    
public:
    PresetContainer(std::string const &name);
    PresetContainer(PresetContainer const &presets);
    virtual ~PresetContainer();

    void AddPreset(Preset const &preset);
    void AddPreset(std::string const &name, unsigned char *value, unsigned char min = 0, unsigned char max = 127);
    void AddPreset(std::string const &name, unsigned short int *value, unsigned short int min = 0, unsigned short int max = 16383);
    void AddPreset(std::string const &name, short int *value, short int min = 0, short int max = 16383);
    void AddPreset(std::string const &name, float *value, float min = 0.0f, float max = 1.0f);
};

/**Presets and Clipboard management*/
class Presets
{
public:
    Presets();
    virtual ~Presets();

    virtual void copy(const char *name); /**<if name==NULL, the clipboard is used*/
    virtual void paste(int npreset);     //npreset==0 for clipboard
    virtual bool checkclipboardtype();
    void deletepreset(int npreset);

    char type[MAX_PRESETTYPE_SIZE]{};
    //void setelement(int n);

    void rescanforpresets();

protected:
    void setpresettype(const char *type);

    virtual void Serialize(IPresetsSerializer *xml) = 0;
    virtual void Deserialize(IPresetsSerializer *xml) = 0;
    virtual void Defaults() = 0;
};

class WrappedPresets : public Presets, public PresetContainer
{
protected:
    virtual void Serialize(IPresetsSerializer *xml);
    virtual void Deserialize(IPresetsSerializer *xml);
};

#endif
