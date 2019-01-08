/*
  ZynAddSubFX - a software synthesizer

  EnvelopeParams.h - Parameters for Envelope
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

#ifndef ENVELOPE_PARAMS_H
#define ENVELOPE_PARAMS_H

#include <zyn.common/IPresetsSerializer.h>
#include <zyn.common/Presets.h>
#include <zyn.common/globals.h>

#define MAX_ENVELOPE_POINTS 40
#define MIN_ENVELOPE_DB -400

class EnvelopeParams : public WrappedPresets
{
public:
    EnvelopeParams(unsigned char envstretch, unsigned char forcedrelease);
    ~EnvelopeParams();

    void InitPresets();

    void Serialize(IPresetsSerializer *xml);
    void Deserialize(IPresetsSerializer *xml);
    void Defaults();

    float getdt(char i);

    /* MIDI Parameters */
    unsigned char Pfreemode; //1 daca este in modul free sau 0 daca este in mod ADSR,ASR,...
    unsigned char Penvpoints;
    unsigned char Penvsustain; //127 pentru dezactivat
    unsigned char Penvdt[MAX_ENVELOPE_POINTS];
    unsigned char Penvval[MAX_ENVELOPE_POINTS];
    unsigned char Penvstretch;     //64=normal stretch (piano-like), 0=no stretch
    unsigned char Pforcedrelease;  //0 - OFF, 1 - ON
    unsigned char Plinearenvelope; //if the amplitude envelope is linear

    unsigned char PA_dt, PD_dt, PR_dt, PA_val, PD_val, PS_val, PR_val;

    int Envmode;
    // 1 for ADSR parameters (linear amplitude)
    // 2 for ADSR_dB parameters (dB amplitude)
    // 3 for ASR parameters (frequency LFO)
    // 4 for ADSR_filter parameters (filter parameters)
    // 5 for ASR_bw parameters (bandwidth parameters)

public:
    static void ConvertToFree(EnvelopeParams *envelope);
    static EnvelopeParams *ADSRinit(unsigned char Penvstretch_, unsigned char Pforcedrelease_, unsigned char A_dt, unsigned char D_dt, unsigned char S_val, unsigned char R_dt);
    static EnvelopeParams *ADSRinit_dB(unsigned char Penvstretch_, unsigned char Pforcedrelease_, unsigned char A_dt, unsigned char D_dt, unsigned char S_val, unsigned char R_dt);
    static EnvelopeParams *ASRinit(unsigned char Penvstretch_, unsigned char Pforcedrelease_, unsigned char A_val, unsigned char A_dt, unsigned char R_val, unsigned char R_dt);
    static EnvelopeParams *ADSRinit_filter(unsigned char Penvstretch_, unsigned char Pforcedrelease_, unsigned char A_val, unsigned char A_dt, unsigned char D_val, unsigned char D_dt, unsigned char R_dt, unsigned char R_val);
    static EnvelopeParams *ASRinit_bw(unsigned char Penvstretch_, unsigned char Pforcedrelease_, unsigned char A_val, unsigned char A_dt, unsigned char R_val, unsigned char R_dt);

private:
    void store2defaults();

    /* Default parameters */
    unsigned char Denvstretch;
    unsigned char Dforcedrelease;
    unsigned char Dlinearenvelope;
    unsigned char DA_dt, DD_dt, DR_dt, DA_val, DD_val, DS_val, DR_val;
};

#endif
