/*
  ZynAddSubFX - a software synthesizer

  EffectMgr.h - Effect manager, an interface betwen the program and effects
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

#ifndef EFFECTMGR_H
#define EFFECTMGR_H

#include <zyn.common/IPresetsSerializer.h>
#include <zyn.common/Presets.h>
#include <zyn.dsp/FilterParams.h>

#include <pthread.h>

class Effect;
class FilterParams;
class IPresetsSerializer;

/**Effect manager, an interface betwen the program and effects*/
class EffectManager : public WrappedPresets
{
    friend class EffectManagerSerializer;

public:
    EffectManager();
    virtual ~EffectManager();

    void Init(IMixer *mixer, const bool _insertion);

    void InitPresets();

    void Serialize(IPresetsSerializer *xml);
    void Defaults();
    void Deserialize(IPresetsSerializer *xml);

    void out(float *smpsl, float *smpsr);

    void setdryonly(bool value);

    /**get the output(to speakers) volume of the systemeffect*/
    float sysefxgetvolume();

    void cleanup();

    void changeeffect(int nefx_);
    int geteffect();
    void changepreset(unsigned char npreset);
    void changepreset_nolock(unsigned char npreset);
    unsigned char getpreset();
    void seteffectpar(int npar, unsigned char value);
    void seteffectpar_nolock(int npar, unsigned char value);
    unsigned char geteffectpar(int npar);

    bool _insertion;
    float *_effectOutL, *_effectOutR;

    // used by UI
    float getEQfreqresponse(float freq);

    FilterParams *_filterpars;
    IMixer *_mixer;

private:
    int _effectType;
    Effect *_effect;
    bool _dryonly;
};

#endif
