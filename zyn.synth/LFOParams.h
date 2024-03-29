/*
  ZynAddSubFX - a software synthesizer

  LFOParams.h - Parameters for LFO
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

#ifndef LFO_PARAMS_H
#define LFO_PARAMS_H

#include <zyn.common/IPresetsSerializer.h>
#include <zyn.common/Presets.h>
#include <zyn.common/globals.h>

class LFOParams : public WrappedPresets
{
public:
    LFOParams(unsigned char freq,
              unsigned char intensity,
              unsigned char startphase,
              unsigned char LFOtype,
              unsigned char randomness,
              unsigned char delay,
              unsigned char continous,
              unsigned char fel);
    virtual ~LFOParams();

    void InitPresets();

    void Defaults();

    /*  MIDI Parameters*/
    float Pfreq;               /**<frequency*/
    unsigned char Pintensity;  /**<intensity*/
    unsigned char Pstartphase; /**<start phase (0=random)*/
    unsigned char PLFOtype;    /**<LFO type (sin,triangle,square,ramp,...)*/
    unsigned char Prandomness; /**<randomness (0=off)*/
    unsigned char Pfreqrand;   /**<frequency randomness (0=off)*/
    unsigned char Pdelay;      /**<delay (0=off)*/
    unsigned char Pcontinous;  /**<1 if LFO is continous*/
    unsigned char Pstretch;    /**<how the LFO is "stretched" according the note frequency (64=no stretch)*/

    int Pkind;         //what kind is the LFO (0 - frequency, 1 - amplitude, 2 - filter)
    static int time; //is used by Pcontinous parameter
private:
    /* Default parameters */
    unsigned char Dfreq;
    unsigned char Dintensity;
    unsigned char Dstartphase;
    unsigned char DLFOtype;
    unsigned char Drandomness;
    unsigned char Ddelay;
    bool Dcontinous;
};

#endif
