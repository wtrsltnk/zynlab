/*
  ZynAddSubFX - a software synthesizer

  EffectManager.cpp - Effect manager, an interface betwen the program and effects
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

#include "EffectMgr.h"
#include "Alienwah.h"
#include "Chorus.h"
#include "Distorsion.h"
#include "DynamicFilter.h"
#include "EQ.h"
#include "Echo.h"
#include "Effect.h"
#include "Phaser.h"
#include "Reverb.h"
#include <zyn.common/IPresetsSerializer.h>
#include <zyn.dsp/FilterParams.h>

#include <cstring>
#include <iostream>

using namespace std;

EffectManager::EffectManager() {}

void EffectManager::Init(IMixer *mixer, const bool insertion_)
{
    _mixer = mixer;
    _insertion = insertion_;
    _effectOutL = new float[_mixer->GetSettings()->buffersize];
    _effectOutR = new float[_mixer->GetSettings()->buffersize];
    _filterpars = nullptr;
    _effectType = 0;
    _effect = nullptr;
    _dryonly = false;
    setpresettype("Peffect");
    memset(_effectOutL, 0, mixer->GetSettings()->bufferbytes);
    memset(_effectOutR, 0, mixer->GetSettings()->bufferbytes);
    Defaults();
}

EffectManager::~EffectManager()
{
    delete _effect;
    delete[] _effectOutL;
    delete[] _effectOutR;
}

void EffectManager::Defaults()
{
    changeeffect(0);
    setdryonly(false);
}

//Change the effect
void EffectManager::changeeffect(int _nefx)
{
    cleanup();
    if (_effectType == _nefx)
    {
        return;
    }

    _effectType = _nefx;
    memset(_effectOutL, 0, _mixer->GetSettings()->bufferbytes);
    memset(_effectOutR, 0, _mixer->GetSettings()->bufferbytes);
    delete _effect;
    switch (_effectType)
    {
        case 1:
        {
            _effect = new Reverb(_insertion, _effectOutL, _effectOutR, _mixer->GetSettings());
            break;
        }
        case 2:
        {
            _effect = new Echo(_insertion, _effectOutL, _effectOutR, _mixer->GetSettings());
            break;
        }
        case 3:
        {
            _effect = new Chorus(_insertion, _effectOutL, _effectOutR, _mixer->GetSettings());
            break;
        }
        case 4:
        {
            _effect = new Phaser(_insertion, _effectOutL, _effectOutR, _mixer->GetSettings());
            break;
        }
        case 5:
        {
            _effect = new Alienwah(_insertion, _effectOutL, _effectOutR, _mixer->GetSettings());
            break;
        }
        case 6:
        {
            _effect = new Distorsion(_insertion, _effectOutL, _effectOutR, _mixer->GetSettings());
            break;
        }
        case 7:
        {
            _effect = new EQ(_insertion, _effectOutL, _effectOutR, _mixer->GetSettings());
            break;
        }
        case 8:
        {
            _effect = new DynamicFilter(_insertion, _effectOutL, _effectOutR, _mixer->GetSettings());
            break;
        }
        //put more effect here
        default:
        {
            _effect = nullptr;
            break; //no effect (thru)
        }
    }

    if (_effect != nullptr)
    {
        _filterpars = _effect->filterpars;
    }
}

//Obtain the effect number
int EffectManager::geteffect()
{
    return _effectType;
}

// Cleanup the current effect
void EffectManager::cleanup()
{
    if (_effect != nullptr)
    {
        _effect->Cleanup();
    }
}

// Get the preset of the current effect
unsigned char EffectManager::getpreset()
{
    return _effect != nullptr ? _effect->Ppreset : 0;
}

// Change the preset of the current effect
void EffectManager::changepreset_nolock(unsigned char npreset)
{
    if (_effect != nullptr)
    {
        _effect->SetPreset(npreset);
    }
}

//Change the preset of the current effect(with thread locking)
void EffectManager::changepreset(unsigned char npreset)
{
    _mixer->Lock();
    changepreset_nolock(npreset);
    _mixer->Unlock();
}

//Change a parameter of the current effect
void EffectManager::seteffectpar_nolock(int npar, unsigned char value)
{
    if (_effect != nullptr)
    {
        _effect->ChangeParameter(npar, value);
    }
}

// Change a parameter of the current effect (with thread locking)
void EffectManager::seteffectpar(int npar, unsigned char value)
{
    _mixer->Lock();
    seteffectpar_nolock(npar, value);
    _mixer->Unlock();
}

//Get a parameter of the current effect
unsigned char EffectManager::geteffectpar(int npar)
{
    return _effect != nullptr ? _effect->GetParameter(npar) : 0;
}

// Apply the effect
void EffectManager::out(float *smpsl, float *smpsr)
{
    if (_effect == nullptr)
    {
        if (!_insertion)
        {
            for (unsigned int i = 0; i < _mixer->GetSettings()->buffersize; ++i)
            {
                smpsl[i] = 0.0f;
                smpsr[i] = 0.0f;
                _effectOutL[i] = 0.0f;
                _effectOutR[i] = 0.0f;
            }
        }
        return;
    }

    for (unsigned int i = 0; i < _mixer->GetSettings()->buffersize; ++i)
    {
        smpsl[i] += _mixer->GetSettings()->denormalkillbuf[i];
        smpsr[i] += _mixer->GetSettings()->denormalkillbuf[i];
        _effectOutL[i] = 0.0f;
        _effectOutR[i] = 0.0f;
    }
    _effect->out(Stereo<float *>(smpsl, smpsr));

    float volume = _effect->volume;

    if (_effectType == 7)
    { //this is need only for the EQ effect
        memcpy(smpsl, _effectOutL, _mixer->GetSettings()->bufferbytes);
        memcpy(smpsr, _effectOutR, _mixer->GetSettings()->bufferbytes);
        return;
    }

    //Insertion effect
    if (_insertion != 0)
    {
        float v1, v2;
        if (volume < 0.5f)
        {
            v1 = 1.0f;
            v2 = volume * 2.0f;
        }
        else
        {
            v1 = (1.0f - volume) * 2.0f;
            v2 = 1.0f;
        }
        if ((_effectType == 1) || (_effectType == 2))
        {
            v2 *= v2; //for Reverb and Echo, the wet function is not liniar
        }

        if (_dryonly) //this is used for instrument effect only
        {
            for (unsigned int i = 0; i < _mixer->GetSettings()->buffersize; ++i)
            {
                smpsl[i] *= v1;
                smpsr[i] *= v1;
                _effectOutL[i] *= v2;
                _effectOutR[i] *= v2;
            }
        }
        else // normal instrument/insertion effect
        {
            for (unsigned int i = 0; i < _mixer->GetSettings()->buffersize; ++i)
            {
                smpsl[i] = smpsl[i] * v1 + _effectOutL[i] * v2;
                smpsr[i] = smpsr[i] * v1 + _effectOutR[i] * v2;
            }
        }
    }
    else // System effect
    {
        for (unsigned int i = 0; i < _mixer->GetSettings()->buffersize; ++i)
        {
            _effectOutL[i] *= 2.0f * volume;
            _effectOutR[i] *= 2.0f * volume;
            smpsl[i] = _effectOutL[i];
            smpsr[i] = _effectOutR[i];
        }
    }
}

// Get the effect volume for the system effect
float EffectManager::sysefxgetvolume()
{
    return (_effect == nullptr) ? 1.0f : _effect->outvolume;
}

// Get the EQ response
float EffectManager::getEQfreqresponse(float freq)
{
    return (_effectType == 7) ? _effect->GetFrequencyResponse(freq) : 0.0f;
}

void EffectManager::setdryonly(bool value)
{
    _dryonly = value;
}

void EffectManager::Serialize(IPresetsSerializer *xml)
{
    xml->addpar("type", geteffect());

    if (_effect == nullptr || !geteffect())
    {
        return;
    }
    xml->addpar("preset", _effect->Ppreset);

    xml->beginbranch("EFFECT_PARAMETERS");
    for (int n = 0; n < 128; ++n)
    {
        int par = geteffectpar(n);
        if (par == 0)
        {
            continue;
        }
        xml->beginbranch("par_no", n);
        xml->addpar("par", par);
        xml->endbranch();
    }
    if (_filterpars)
    {
        xml->beginbranch("FILTER");
        _filterpars->Serialize(xml);
        xml->endbranch();
    }
    xml->endbranch();
}

void EffectManager::Deserialize(IPresetsSerializer *xml)
{
    changeeffect(xml->getpar127("type", geteffect()));

    if (_effect == nullptr || !geteffect())
    {
        return;
    }

    _effect->Ppreset = xml->getpar127("preset", _effect->Ppreset);

    if (xml->enterbranch("EFFECT_PARAMETERS"))
    {
        for (int n = 0; n < 128; ++n)
        {
            seteffectpar_nolock(n, 0); //erase effect parameter
            if (xml->enterbranch("par_no", n) == 0)
            {
                continue;
            }
            int par = geteffectpar(n);
            seteffectpar_nolock(n, xml->getpar127("par", par));
            xml->exitbranch();
        }
        if (_filterpars)
        {
            if (xml->enterbranch("FILTER"))
            {
                _filterpars->Deserialize(xml);
                xml->exitbranch();
            }
        }
        xml->exitbranch();
    }
    cleanup();
}
