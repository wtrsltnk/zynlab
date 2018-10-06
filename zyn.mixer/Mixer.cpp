/*
  ZynAddSubFX - a software synthesizer

  Mixer.cpp - It sends Midi Messages to Parts, receives samples from parts,
             process them with system/insertion effects and mix them
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

#include "Instrument.h"
#include "Mixer.h"
#include <zyn.common/PresetsSerializer.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.synth/FFTwrapper.h>
#include <zyn.synth/LFOParams.h>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

vuData::vuData()
    : outpeakl(0.0f), outpeakr(0.0f), maxoutpeakl(0.0f), maxoutpeakr(0.0f),
      rmspeakl(0.0f), rmspeakr(0.0f), clipped(0)
{}

Mixer::Mixer(SystemSettings *synth_, IBankManager *bank_)
    : ctl(synth_)
{
    this->_synth = synth_;
    bank = bank_;

    swaplr = false;
    off = 0;
    smps = 0;
    bufl = new float[this->_synth->buffersize];
    bufr = new float[this->_synth->buffersize];

    pthread_mutex_init(&mutex, nullptr);
    pthread_mutex_init(&vumutex, nullptr);
    fft = new FFTwrapper(this->_synth->oscilsize);

    shutup = 0;
    for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
    {
        vuoutpeakpart[npart] = 1e-9f;
        fakepeakpart[npart] = 0;
    }

    for (auto &npart : part)
    {
        npart = new Instrument(this->_synth, &microtonal, fft, &mutex);
    }

    //Insertion Effects init
    for (auto &nefx : insefx)
    {
        nefx = new EffectManager(true, &mutex, this->_synth);
    }

    //System Effects init
    for (auto &nefx : sysefx)
    {
        nefx = new EffectManager(false, &mutex, this->_synth);
    }

    defaults();
}

Mixer::~Mixer()
{
    delete[] bufl;
    delete[] bufr;

    for (auto &npart : part)
    {
        delete npart;
    }
    for (auto &nefx : insefx)
    {
        delete nefx;
    }
    for (auto &nefx : sysefx)
    {
        delete nefx;
    }

    delete fft;

    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&vumutex);
}

IBankManager *Mixer::GetBankManager()
{
    return bank;
}

void Mixer::defaults()
{
    volume = 1.0f;
    setPvolume(80);
    setPkeyshift(64);

    for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
    {
        part[npart]->Defaults();
        part[npart]->Prcvchn = npart % NUM_MIDI_CHANNELS;
    }

    partonoff(0, 1); //enable the first part

    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        insefx[nefx]->Defaults();
        Pinsparts[nefx] = -1;
    }

    //System Effects init
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        sysefx[nefx]->Defaults();
        for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
        {
            setPsysefxvol(npart, nefx, 0);
        }

        for (int nefxto = 0; nefxto < NUM_SYS_EFX; ++nefxto)
        {
            setPsysefxsend(nefx, nefxto, 0);
        }
    }

    microtonal.defaults();
    ShutUp();
}

void Mixer::Lock()
{
    pthread_mutex_lock(&mutex);
}

void Mixer::Unlock()
{
    pthread_mutex_unlock(&mutex);
}

/*
 * Note On Messages (velocity=0 for NoteOff)
 */
void Mixer::NoteOn(unsigned char chan, unsigned char note, unsigned char velocity)
{
    if (!velocity)
    {
        this->NoteOff(chan, note);
        return;
    }

    for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
    {
        if (chan == part[npart]->Prcvchn)
        {
            fakepeakpart[npart] = velocity * 2;
            if (part[npart]->Penabled)
            {
                part[npart]->NoteOn(note, velocity, keyshift);
            }
        }
    }
}

/*
 * Note Off Messages
 */
void Mixer::NoteOff(unsigned char chan, unsigned char note)
{
    for (auto &npart : part)
    {
        if ((chan == npart->Prcvchn) && npart->Penabled)
        {
            npart->NoteOff(note);
        }
    }
}

/*
 * Pressure Messages (velocity=0 for NoteOff)
 */
void Mixer::PolyphonicAftertouch(unsigned char chan, unsigned char note, unsigned char velocity)
{
    if (!velocity)
    {
        this->NoteOff(chan, note);
        return;
    }

    for (auto &npart : part)
    {
        if (chan == npart->Prcvchn)
        {
            if (npart->Penabled)
            {
                npart->PolyphonicAftertouch(note, velocity, keyshift);
            }
        }
    }
}

/*
 * Controllers
 */
void Mixer::SetController(unsigned char chan, int type, int par)
{
    if ((type == C_dataentryhi) || (type == C_dataentrylo) || (type == C_nrpnhi) || (type == C_nrpnlo))
    { //Process RPN and NRPN by the Master (ignore the chan)
        ctl.setparameternumber(static_cast<unsigned int>(type), par);

        int parhi = -1, parlo = -1, valhi = -1, vallo = -1;
        if (ctl.getnrpn(&parhi, &parlo, &valhi, &vallo) == 0) //this is NRPN
        {                                                     //fprintf(stderr,"rcv. NRPN: %d %d %d %d\n",parhi,parlo,valhi,vallo);
            switch (parhi)
            {
                case 0x04: //System Effects
                {
                    if (parlo < NUM_SYS_EFX)
                    {
                        sysefx[parlo]->seteffectpar_nolock(valhi, static_cast<unsigned char>(vallo));
                    }
                    break;
                }
                case 0x08: //Insertion Effects
                {
                    if (parlo < NUM_INS_EFX)
                    {
                        insefx[parlo]->seteffectpar_nolock(valhi, static_cast<unsigned char>(vallo));
                    }
                    break;
                }
            };
        }
    }
    else if (type == C_bankselectmsb)
    {
        bank->LoadBank(par);
    }
    else
    {                            //other controllers
        for (auto &npart : part) //Send the controller to all part assigned to the channel
        {
            if ((chan == npart->Prcvchn) && (npart->Penabled != 0))
            {
                npart->SetController(static_cast<unsigned int>(type), par);
            }
        }

        if (type == C_allsoundsoff)
        { //cleanup insertion/system FX
            for (auto &nefx : sysefx)
            {
                nefx->cleanup();
            }
            for (auto &nefx : insefx)
            {
                nefx->cleanup();
            }
        }
    }
}

void Mixer::SetProgram(unsigned char chan, unsigned int pgm)
{
    if (Config::Current().cfg.IgnoreProgramChange)
    {
        return;
    }

    for (auto &npart : part)
    {
        if (chan == npart->Prcvchn)
        {
            bank->LoadFromSlot(pgm, npart);

            //Hack to get pad note parameters to update
            //this is not real time safe and makes assumptions about the calling
            //convention of this function...
            Unlock();
            npart->applyparameters();
            Lock();
        }
    }
}

void Mixer::vuUpdate(const float *outl, const float *outr)
{
    //Peak computation (for vumeters)
    vu.outpeakl = 1e-12f;
    vu.outpeakr = 1e-12f;
    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        if (fabs(outl[i]) > vu.outpeakl)
        {
            vu.outpeakl = fabs(outl[i]);
        }
        if (fabs(outr[i]) > vu.outpeakr)
        {
            vu.outpeakr = fabs(outr[i]);
        }
    }
    if ((vu.outpeakl > 1.0f) || (vu.outpeakr > 1.0f))
    {
        vu.clipped = 1;
    }
    if (vu.maxoutpeakl < vu.outpeakl)
    {
        vu.maxoutpeakl = vu.outpeakl;
    }
    if (vu.maxoutpeakr < vu.outpeakr)
    {
        vu.maxoutpeakr = vu.outpeakr;
    }

    //RMS Peak computation (for vumeters)
    vu.rmspeakl = 1e-12f;
    vu.rmspeakr = 1e-12f;
    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        vu.rmspeakl += outl[i] * outl[i];
        vu.rmspeakr += outr[i] * outr[i];
    }
    vu.rmspeakl = sqrt(vu.rmspeakl / this->_synth->buffersize_f);
    vu.rmspeakr = sqrt(vu.rmspeakr / this->_synth->buffersize_f);

    //Part Peak computation (for Part vumeters or fake part vumeters)
    for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
    {
        vuoutpeakpart[npart] = 1.0e-12f;
        if (part[npart]->Penabled != 0)
        {
            float *outl = part[npart]->partoutl,
                  *outr = part[npart]->partoutr;
            for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
            {
                float tmp = fabs(outl[i] + outr[i]);
                if (tmp > vuoutpeakpart[npart])
                {
                    vuoutpeakpart[npart] = tmp;
                }
            }
            vuoutpeakpart[npart] *= volume;
        }
        else if (fakepeakpart[npart] > 1)
        {
            fakepeakpart[npart]--;
        }
    }
}

/*
 * Enable/Disable a part
 */
void Mixer::partonoff(int npart, int what)
{
    if (npart >= NUM_MIDI_PARTS)
    {
        return;
    }

    if (what != 0)
    { //enabled
        part[npart]->Penabled = 1;
        fakepeakpart[npart] = 0;
        return;
    }

    fakepeakpart[npart] = 0;
    part[npart]->Penabled = 0;
    part[npart]->cleanup();
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        if (Pinsparts[nefx] == npart)
        {
            insefx[nefx]->cleanup();
        }
    }
}

/*
 * Master audio out (the final sound)
 */
void Mixer::AudioOut(float *outl, float *outr)
{
    //Swaps the Left channel with Right Channel
    if (swaplr)
    {
        swap(outl, outr);
    }

    //clean up the output samples (should not be needed?)
    memset(outl, 0, this->_synth->bufferbytes);
    memset(outr, 0, this->_synth->bufferbytes);

    //Compute part samples and store them part[npart]->partoutl,partoutr
    for (auto &npart : part)
    {
        if (npart->Penabled != 0 && !pthread_mutex_trylock(&npart->load_mutex))
        {
            npart->ComputePartSmps();
            pthread_mutex_unlock(&npart->load_mutex);
        }
    }

    //Insertion effects
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        if (Pinsparts[nefx] >= 0)
        {
            int efxpart = Pinsparts[nefx];
            if (part[efxpart]->Penabled)
            {
                insefx[nefx]->out(part[efxpart]->partoutl, part[efxpart]->partoutr);
            }
        }
    }

    //Apply the part volumes and pannings (after insertion effects)
    for (auto &npart : part)
    {
        if (npart->Penabled == 0)
        {
            continue;
        }

        Stereo<float> newvol(npart->volume),
            oldvol(npart->oldvolumel,
                   npart->oldvolumer);

        float pan = npart->panning;
        if (pan < 0.5f)
        {
            newvol._left *= pan * 2.0f;
        }
        else
        {
            newvol._right *= (1.0f - pan) * 2.0f;
        }

        //the volume or the panning has changed and needs interpolation
        if (ABOVE_AMPLITUDE_THRESHOLD(oldvol._left, newvol._left) || ABOVE_AMPLITUDE_THRESHOLD(oldvol._right, newvol._right))
        {
            for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
            {
                Stereo<float> vol(INTERPOLATE_AMPLITUDE(oldvol._left, newvol._left, i, this->_synth->buffersize),
                                  INTERPOLATE_AMPLITUDE(oldvol._right, newvol._right, i, this->_synth->buffersize));
                npart->partoutl[i] *= vol._left;
                npart->partoutr[i] *= vol._right;
            }
            npart->oldvolumel = newvol._left;
            npart->oldvolumer = newvol._right;
        }
        else
        {
            for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
            { //the volume did not changed
                npart->partoutl[i] *= newvol._left;
                npart->partoutr[i] *= newvol._right;
            }
        }
    }

    //System effects
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        if (sysefx[nefx]->geteffect() == 0)
        {
            continue; //the effect is disabled
        }

        float tmpmixl[this->_synth->buffersize];
        float tmpmixr[this->_synth->buffersize];
        //Clean up the samples used by the system effects
        memset(tmpmixl, 0, this->_synth->bufferbytes);
        memset(tmpmixr, 0, this->_synth->bufferbytes);

        //Mix the channels according to the part settings about System Effect
        for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
        {
            //skip if the part has no output to effect
            if (Psysefxvol[nefx][npart] == 0)
            {
                continue;
            }
            //skip if the part is disabled
            if (part[npart]->Penabled == 0)
            {
                continue;
            }
            //the output volume of each part to system effect
            const float vol = sysefxvol[nefx][npart];
            for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
            {
                tmpmixl[i] += part[npart]->partoutl[i] * vol;
                tmpmixr[i] += part[npart]->partoutr[i] * vol;
            }
        }

        // system effect send to next ones
        for (int nefxfrom = 0; nefxfrom < nefx; ++nefxfrom)
        {
            if (Psysefxsend[nefxfrom][nefx] != 0)
            {
                const float vol = sysefxsend[nefxfrom][nefx];
                for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
                {
                    tmpmixl[i] += sysefx[nefxfrom]->efxoutl[i] * vol;
                    tmpmixr[i] += sysefx[nefxfrom]->efxoutr[i] * vol;
                }
            }
        }

        sysefx[nefx]->out(tmpmixl, tmpmixr);

        //Add the System Effect to sound output
        const float outvol = sysefx[nefx]->sysefxgetvolume();
        for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
        {
            outl[i] += tmpmixl[i] * outvol;
            outr[i] += tmpmixr[i] * outvol;
        }
    }

    //Mix all parts
    for (auto &npart : part)
    {
        if (npart->Penabled) //only mix active parts
        {
            for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
            { //the volume did not changed
                outl[i] += npart->partoutl[i];
                outr[i] += npart->partoutr[i];
            }
        }
    }

    //Insertion effects for Master Out
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        if (Pinsparts[nefx] == -2)
        {
            insefx[nefx]->out(outl, outr);
        }
    }

    //Master Volume
    for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
    {
        outl[i] *= volume;
        outr[i] *= volume;
    }

    if (!pthread_mutex_trylock(&vumutex))
    {
        vuUpdate(outl, outr);
        pthread_mutex_unlock(&vumutex);
    }

    //Shutup if it is asked (with fade-out)
    if (shutup)
    {
        for (unsigned int i = 0; i < this->_synth->buffersize; ++i)
        {
            float tmp = (this->_synth->buffersize_f - i) / this->_synth->buffersize_f;
            outl[i] *= tmp;
            outr[i] *= tmp;
        }
        ShutUp();
    }

    //update the LFO's time
    LFOParams::time++;
}

//TODO review the respective code from yoshimi for this
//If memory serves correctly, libsamplerate was used
void Mixer::GetAudioOutSamples(size_t nsamples,
                               unsigned samplerate,
                               float *outl,
                               float *outr)
{
    off_t out_off = 0;

    //Fail when resampling rather than doing a poor job
    if (this->_synth->samplerate != samplerate)
    {
        printf("darn it: %d vs %d\n", this->_synth->samplerate, samplerate);
        return;
    }

    while (nsamples)
    {
        //use all available samples
        if (nsamples >= smps)
        {
            memcpy(outl + out_off, bufl + off, sizeof(float) * smps);
            memcpy(outr + out_off, bufr + off, sizeof(float) * smps);
            nsamples -= smps;

            //generate samples
            AudioOut(bufl, bufr);
            off = 0;
            out_off += smps;
            smps = this->_synth->buffersize;
        }
        else
        { //use some samples
            memcpy(outl + out_off, bufl + off, sizeof(float) * nsamples);
            memcpy(outr + out_off, bufr + off, sizeof(float) * nsamples);
            smps -= nsamples;
            off += nsamples;
            nsamples = 0;
        }
    }
}

int Mixer::GetInstrumentCount()
{
    return NUM_MIDI_PARTS;
}

Instrument *Mixer::GetInstrument(int index)
{
    if (index >= 0 && index < NUM_MIDI_PARTS)
    {
        return part[index];
    }

    return nullptr;
}

/*
 * Parameter control
 */
void Mixer::setPvolume(unsigned char Pvolume_)
{
    Pvolume = Pvolume_;
    volume = dB2rap((Pvolume - 96.0f) / 96.0f * 40.0f);
}

void Mixer::setPkeyshift(unsigned char Pkeyshift_)
{
    Pkeyshift = Pkeyshift_;
    keyshift = static_cast<int>(Pkeyshift) - 64;
}

void Mixer::setPsysefxvol(int Ppart, int Pefx, unsigned char Pvol)
{
    Psysefxvol[Pefx][Ppart] = Pvol;
    sysefxvol[Pefx][Ppart] = powf(0.1f, (1.0f - Pvol / 96.0f) * 2.0f);
}

void Mixer::setPsysefxsend(int Pefxfrom, int Pefxto, unsigned char Pvol)
{
    Psysefxsend[Pefxfrom][Pefxto] = Pvol;
    sysefxsend[Pefxfrom][Pefxto] = powf(0.1f, (1.0f - Pvol / 96.0f) * 2.0f);
}

/*
 * Panic! (Clean up all parts and effects)
 */
void Mixer::ShutUp()
{
    for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
    {
        part[npart]->cleanup();
        fakepeakpart[npart] = 0;
    }
    for (auto &nefx : insefx)
    {
        nefx->cleanup();
    }
    for (auto &nefx : sysefx)
    {
        nefx->cleanup();
    }
    vuresetpeaks();
    shutup = 0;
}

/*
 * Reset peaks and clear the "cliped" flag (for VU-meter)
 */
void Mixer::vuresetpeaks()
{
    pthread_mutex_lock(&vumutex);
    vu.outpeakl = 1e-9f;
    vu.outpeakr = 1e-9f;
    vu.maxoutpeakl = 1e-9f;
    vu.maxoutpeakr = 1e-9f;
    vu.clipped = 0;
    pthread_mutex_unlock(&vumutex);
}

vuData Mixer::getVuData()
{
    vuData tmp;
    pthread_mutex_lock(&vumutex);
    tmp = vu;
    pthread_mutex_unlock(&vumutex);
    return tmp;
}

void Mixer::applyparameters(bool lockmutex)
{
    for (auto &npart : part)
    {
        npart->applyparameters(lockmutex);
    }
}

int Mixer::getalldata(char **data)
{
    PresetsSerializer xml;

    xml.beginbranch("MASTER");

    Lock();
    add2XML(&xml);
    Unlock();

    xml.endbranch();

    *data = xml.getXMLdata();

    return static_cast<int>(strlen(*data) + 1);
}

void Mixer::putalldata(char *data, int /*size*/)
{
    PresetsSerializer xml;
    if (!xml.putXMLdata(data))
    {
        return;
    }

    if (xml.enterbranch("MASTER") == 0)
    {
        return;
    }

    Lock();
    getfromXML(&xml);
    Unlock();

    xml.exitbranch();
}

int Mixer::saveXML(const char *filename)
{
    PresetsSerializer xml;

    xml.beginbranch("MASTER");
    add2XML(&xml);
    xml.endbranch();

    return xml.saveXMLfile(filename);
}

int Mixer::loadXML(const char *filename)
{
    PresetsSerializer xml;
    if (xml.loadXMLfile(filename) < 0)
    {
        return -1;
    }

    if (xml.enterbranch("MASTER") == 0)
    {
        return -10;
    }

    getfromXML(&xml);
    xml.exitbranch();

    return 0;
}

void Mixer::add2XML(IPresetsSerializer *xml)
{
    xml->addpar("volume", Pvolume);
    xml->addpar("key_shift", Pkeyshift);
    xml->addparbool("nrpn_receive", ctl.NRPN.receive);

    xml->beginbranch("MICROTONAL");
    microtonal.add2XML(xml);
    xml->endbranch();

    for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
    {
        xml->beginbranch("PART", npart);
        part[npart]->Serialize(xml);
        xml->endbranch();
    }

    xml->beginbranch("SYSTEM_EFFECTS");
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        xml->beginbranch("SYSTEM_EFFECT", nefx);
        xml->beginbranch("EFFECT");
        sysefx[nefx]->Serialize(xml);
        xml->endbranch();

        for (int pefx = 0; pefx < NUM_MIDI_PARTS; ++pefx)
        {
            xml->beginbranch("VOLUME", pefx);
            xml->addpar("vol", Psysefxvol[nefx][pefx]);
            xml->endbranch();
        }

        for (int tonefx = nefx + 1; tonefx < NUM_SYS_EFX; ++tonefx)
        {
            xml->beginbranch("SENDTO", tonefx);
            xml->addpar("send_vol", Psysefxsend[nefx][tonefx]);
            xml->endbranch();
        }

        xml->endbranch();
    }
    xml->endbranch();

    xml->beginbranch("INSERTION_EFFECTS");
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        xml->beginbranch("INSERTION_EFFECT", nefx);
        xml->addpar("part", Pinsparts[nefx]);

        xml->beginbranch("EFFECT");
        insefx[nefx]->Serialize(xml);
        xml->endbranch();
        xml->endbranch();
    }

    xml->endbranch();
}

void Mixer::getfromXML(IPresetsSerializer *xml)
{
    setPvolume(static_cast<unsigned char>(xml->getpar127("volume", Pvolume)));
    setPkeyshift(static_cast<unsigned char>(xml->getpar127("key_shift", Pkeyshift)));
    ctl.NRPN.receive = static_cast<unsigned char>(xml->getparbool("nrpn_receive", ctl.NRPN.receive));

    part[0]->Penabled = 0;
    for (int npart = 0; npart < NUM_MIDI_PARTS; ++npart)
    {
        if (xml->enterbranch("PART", npart) == 0)
        {
            continue;
        }
        part[npart]->Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("MICROTONAL"))
    {
        microtonal.getfromXML(xml);
        xml->exitbranch();
    }

    sysefx[0]->changeeffect(0);
    if (xml->enterbranch("SYSTEM_EFFECTS"))
    {
        for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
        {
            if (xml->enterbranch("SYSTEM_EFFECT", nefx) == 0)
            {
                continue;
            }
            if (xml->enterbranch("EFFECT"))
            {
                sysefx[nefx]->Deserialize(xml);
                xml->exitbranch();
            }

            for (int partefx = 0; partefx < NUM_MIDI_PARTS; ++partefx)
            {
                if (xml->enterbranch("VOLUME", partefx) == 0)
                {
                    continue;
                }
                setPsysefxvol(partefx, nefx, static_cast<unsigned char>(xml->getpar127("vol", Psysefxvol[partefx][nefx])));
                xml->exitbranch();
            }

            for (int tonefx = nefx + 1; tonefx < NUM_SYS_EFX; ++tonefx)
            {
                if (xml->enterbranch("SENDTO", tonefx) == 0)
                {
                    continue;
                }
                setPsysefxsend(nefx, tonefx, static_cast<unsigned char>(xml->getpar127("send_vol", Psysefxsend[nefx][tonefx])));
                xml->exitbranch();
            }
            xml->exitbranch();
        }
        xml->exitbranch();
    }

    if (xml->enterbranch("INSERTION_EFFECTS"))
    {
        for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
        {
            if (xml->enterbranch("INSERTION_EFFECT", nefx) == 0)
            {
                continue;
            }
            Pinsparts[nefx] = static_cast<short>(xml->getpar("part", Pinsparts[nefx], -2, NUM_MIDI_PARTS));
            if (xml->enterbranch("EFFECT"))
            {
                insefx[nefx]->Deserialize(xml);
                xml->exitbranch();
            }
            xml->exitbranch();
        }

        xml->exitbranch();
    }
}
