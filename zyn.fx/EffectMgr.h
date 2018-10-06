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

#include "Alienwah.h"
#include "Distorsion.h"
#include "DynamicFilter.h"
#include "EQ.h"
#include "Phaser.h"
#include <zyn.common/IPresetsSerializer.h>
#include <zyn.common/Presets.h>
#include <zyn.dsp/FilterParams.h>

#include <pthread.h>

class Effect;
class FilterParams;
class IPresetsSerializer;

/**Effect manager, an interface betwen the program and effects*/
class EffectManager : public Presets
{
public:
    EffectManager(const bool insertion_, pthread_mutex_t *mutex_, SystemSettings *synth_);
    virtual ~EffectManager();

    void Serialize(IPresetsSerializer *xml);
    void Defaults(void);
    void Deserialize(IPresetsSerializer *xml);

    void out(float *smpsl, float *smpsr);

    void setdryonly(bool value);

    /**get the output(to speakers) volume of the systemeffect*/
    float sysefxgetvolume(void);

    void cleanup(void);

    void changeeffect(int nefx_);
    int geteffect(void);
    void changepreset(unsigned char npreset);
    void changepreset_nolock(unsigned char npreset);
    unsigned char getpreset(void);
    void seteffectpar(int npar, unsigned char value);
    void seteffectpar_nolock(int npar, unsigned char value);
    unsigned char geteffectpar(int npar);

    const bool insertion;
    float *efxoutl, *efxoutr;

    // used by UI
    float getEQfreqresponse(float freq);

    FilterParams *filterpars;
    SystemSettings *_synth;

private:
    int nefx;
    Effect *efx;
    pthread_mutex_t *mutex;
    bool dryonly;
};

#endif
