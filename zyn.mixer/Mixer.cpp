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

#include "Mixer.h"

#include "Channel.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>
#include <sys/stat.h>
#include <sys/types.h>
#include <tuple>
#include <zyn.common/PresetsSerializer.h>
#include <zyn.fx/EffectMgr.h>
#include <zyn.synth/FFTwrapper.h>
#include <zyn.synth/LFOParams.h>

using namespace std;

static std::chrono::milliseconds::rep _lastSequencerTimeInMs;

Mixer::Mixer()
{}

Mixer::~Mixer()
{
    pthread_mutex_destroy(&_mutex);
}
void Mixer::Setup(IBankManager *bankManager)
{
    _bankManager = bankManager;
    meter.Setup();
    ctl.Init();

    _bufl = std::unique_ptr<float>(new float[this->BufferSize()]);
    _bufr = std::unique_ptr<float>(new float[this->BufferSize()]);
    swaplr = false;
    _off = 0;
    _smps = 0;

    _lastSequencerTimeInMs = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();

    pthread_mutex_init(&_mutex, nullptr);
    _fft = std::unique_ptr<IFFTwrapper>(new FFTwrapper(SystemSettings::Instance().oscilsize));

    shutup = 0;

    for (auto &npart : _channels)
    {
        npart.Init(this, &microtonal);
    }

    //Insertion Effects init
    for (auto &nefx : insefx)
    {
        nefx.Init(this, true);
    }

    //System Effects init
    for (auto &nefx : sysefx)
    {
        nefx.Init(this, false);
    }

    Defaults();

    for (unsigned char channelIndex = 0; channelIndex < NUM_MIXER_CHANNELS; channelIndex++)
    {
        for (unsigned char note = 0; note < POLIPHONY; note++)
        {
            std::cout << "setting to 0 " << __LINE__ << std::endl;
            _activeNotes[channelIndex][note] = 0;
        }
    }
}

IBankManager *Mixer::GetBankManager()
{
    return _bankManager;
}

IMeter *Mixer::GetMeter()
{
    return &meter;
}

void Mixer::Defaults()
{
    _volume = 1.0f;
    setPvolume(80);
    setPkeyshift(64);

    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        _channels[npart].Defaults();
        _channels[npart].Prcvchn = npart % NUM_MIDI_CHANNELS;
    }

    partonoff(0, 1); //enable the first part

    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        insefx[nefx].Defaults();
        Pinsparts[nefx] = -1;
    }

    //System Effects init
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        sysefx[nefx].Defaults();
        for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
        {
            setPsysefxvol(npart, nefx, 0);
        }

        for (int nefxto = 0; nefxto < NUM_SYS_EFX; ++nefxto)
        {
            SetSystemEffectSend(nefx, nefxto, 0);
        }
    }

    microtonal.Defaults();
    ShutUp();
}

unsigned int Mixer::SampleRate() const
{
    return SystemSettings::Instance().samplerate;
}

unsigned int Mixer::BufferSize() const
{
    return SystemSettings::Instance().buffersize;
}

unsigned int Mixer::BufferSizeInBytes() const
{
    return SystemSettings::Instance().bufferbytes;
}

float Mixer::BufferSizeFloat() const
{
    return SystemSettings::Instance().buffersize_f;
}

void Mixer::Lock()
{
    pthread_mutex_lock(&_mutex);
}

void Mixer::Unlock()
{
    pthread_mutex_unlock(&_mutex);
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

    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        if (chan == _channels[npart].Prcvchn)
        {
            meter.SetFakePeak(npart, velocity * 2);
            if (_channels[npart].Penabled)
            {
                _channels[npart].NoteOn(note, velocity, _keyshift);
            }
        }
    }
}

/*
 * Note Off Messages
 */
void Mixer::NoteOff(unsigned char chan, unsigned char note)
{
    for (auto &npart : _channels)
    {
        if ((chan == npart.Prcvchn) && npart.Penabled)
        {
            npart.NoteOff(note);
        }
    }
}

static unsigned char lastChan = 0;
static unsigned char lastNote = 0;

void Mixer::HitNote(unsigned char chan, unsigned char note, unsigned char velocity, int durationInMs)
{
    if (!velocity)
    {
        return;
    }

    Lock();
    std::cout << "setting to _activeNotes[" << int(chan) << "][" << int(note) << "] to " << durationInMs << " @ " << __LINE__ << std::endl;
    _activeNotes[chan][note] = durationInMs;
    std::cout << "_activeNotes[" << int(chan) << "][" << int(note) << "] == " << _activeNotes[chan][note] << std::endl;
    Unlock();

    lastChan = chan;
    lastNote = note;

    NoteOn(chan, note, velocity);
}

void Mixer::UpdateActiveNotes()
{
    std::chrono::milliseconds::rep currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    auto deltaTime = currentTime - _lastSequencerTimeInMs;
    _lastSequencerTimeInMs = currentTime;

    std::cout << " -> _activeNotes[" << int(lastChan) << "][" << int(lastNote) << "] == " << _activeNotes[lastChan][lastNote] << std::endl;

    std::vector<std::tuple<unsigned char, unsigned char>> notesToEnd;
    for (int chan = 0; chan < NUM_MIXER_CHANNELS; chan++)
    {
        for (int note = 0; note < POLIPHONY; note++)
        {
            int duration = _activeNotes[chan][note];
            if (duration <= 0)
            {
                continue;
            }

            std::cout << " -> _activeNotes[" << int(chan) << "][" << int(note) << "] == " << duration << std::endl;
            if (_activeNotes[chan][note] <= 0)
            {
                notesToEnd.push_back(std::make_tuple(chan, note));
            }
        }
    }

    for (auto &t : notesToEnd)
    {
        NoteOff(std::get<0>(t), std::get<1>(t));
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

    for (auto &npart : _channels)
    {
        if (chan == npart.Prcvchn)
        {
            if (npart.Penabled)
            {
                npart.PolyphonicAftertouch(note, velocity, _keyshift);
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
                        sysefx[parlo].seteffectpar_nolock(valhi, static_cast<unsigned char>(vallo));
                    }
                    break;
                }
                case 0x08: //Insertion Effects
                {
                    if (parlo < NUM_INS_EFX)
                    {
                        insefx[parlo].seteffectpar_nolock(valhi, static_cast<unsigned char>(vallo));
                    }
                    break;
                }
            };
        }
    }
    else if (type == C_bankselectmsb)
    {
        _bankManager->LoadBank(par);
    }
    else
    {                                 //other controllers
        for (auto &npart : _channels) //Send the controller to all part assigned to the channel
        {
            if ((chan == npart.Prcvchn) && (npart.Penabled != 0))
            {
                npart.SetController(static_cast<unsigned int>(type), par);
            }
        }

        if (type == C_allsoundsoff)
        { //cleanup insertion/system FX
            for (auto &nefx : sysefx)
            {
                nefx.cleanup();
            }
            for (auto &nefx : insefx)
            {
                nefx.cleanup();
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

    for (auto &npart : _channels)
    {
        if (chan == npart.Prcvchn)
        {
            _bankManager->LoadFromSlot(pgm, &npart);

            //Hack to get pad note parameters to update
            //this is not real time safe and makes assumptions about the calling
            //convention of this function...
            Unlock();
            npart.ApplyParameters();
            Lock();
        }
    }
}

/*
 * Enable/Disable a part
 */
void Mixer::partonoff(int npart, int what)
{
    if (npart >= NUM_MIXER_CHANNELS)
    {
        return;
    }

    meter.SetFakePeak(npart, 0);

    if (what != 0)
    { //enabled
        _channels[npart].Penabled = 1;
        return;
    }

    _channels[npart].Penabled = 0;
    _channels[npart].Cleanup();
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        if (Pinsparts[nefx] == npart)
        {
            insefx[nefx].cleanup();
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

    UpdateActiveNotes();

    //clean up the output samples (should not be needed?)
    memset(outl, 0, this->BufferSizeInBytes());
    memset(outr, 0, this->BufferSizeInBytes());

    //Compute part samples and store them part[npart].partoutl,partoutr
    for (auto &channel : _channels)
    {
        if (channel.Penabled != 0 && !channel.TryLock())
        {
            channel.ComputeInstrumentSamples();
            channel.Unlock();
        }
    }

    //Insertion effects
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        if (Pinsparts[nefx] >= 0)
        {
            int efxpart = Pinsparts[nefx];
            if (_channels[efxpart].Penabled)
            {
                insefx[nefx].out(_channels[efxpart].partoutl, _channels[efxpart].partoutr);
            }
        }
    }

    //Apply the part volumes and pannings (after insertion effects)
    for (auto &npart : _channels)
    {
        if (npart.Penabled == 0)
        {
            continue;
        }

        Stereo<float> newvol(npart.volume);
        Stereo<float> oldvol(npart.oldvolumel, npart.oldvolumer);

        float pan = npart.panning;
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
            for (unsigned int i = 0; i < this->BufferSize(); ++i)
            {
                Stereo<float> vol(INTERPOLATE_AMPLITUDE(oldvol._left, newvol._left, i, this->BufferSize()),
                                  INTERPOLATE_AMPLITUDE(oldvol._right, newvol._right, i, this->BufferSize()));
                npart.partoutl[i] *= vol._left;
                npart.partoutr[i] *= vol._right;
            }
            npart.oldvolumel = newvol._left;
            npart.oldvolumer = newvol._right;
        }
        else
        {
            for (unsigned int i = 0; i < this->BufferSize(); ++i)
            { //the volume did not changed
                npart.partoutl[i] *= newvol._left;
                npart.partoutr[i] *= newvol._right;
            }
        }
    }

    auto tmpmixl = new float[this->BufferSize()];
    auto tmpmixr = new float[this->BufferSize()];

    //System effects
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        if (sysefx[nefx].geteffect() == 0)
        {
            continue; //the effect is disabled
        }

        //Clean up the samples used by the system effects
        memset(tmpmixl, 0, this->BufferSizeInBytes());
        memset(tmpmixr, 0, this->BufferSizeInBytes());

        //Mix the channels according to the part settings about System Effect
        for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
        {
            //skip if the part has no output to effect
            if (Psysefxvol[nefx][npart] == 0)
            {
                continue;
            }
            //skip if the part is disabled
            if (_channels[npart].Penabled == 0)
            {
                continue;
            }
            //the output volume of each part to system effect
            const float vol = _sysefxvol[nefx][npart];
            for (unsigned int i = 0; i < this->BufferSize(); ++i)
            {
                tmpmixl[i] += _channels[npart].partoutl[i] * vol;
                tmpmixr[i] += _channels[npart].partoutr[i] * vol;
            }
        }

        // system effect send to next ones
        for (int nefxfrom = 0; nefxfrom < nefx; ++nefxfrom)
        {
            if (Psysefxsend[nefxfrom][nefx] != 0)
            {
                const float vol = _sysefxsend[nefxfrom][nefx];
                for (unsigned int i = 0; i < this->BufferSize(); ++i)
                {
                    tmpmixl[i] += sysefx[nefxfrom]._effectOutL[i] * vol;
                    tmpmixr[i] += sysefx[nefxfrom]._effectOutR[i] * vol;
                }
            }
        }

        sysefx[nefx].out(tmpmixl, tmpmixr);

        //Add the System Effect to sound output
        const float outvol = sysefx[nefx].sysefxgetvolume();
        for (unsigned int i = 0; i < this->BufferSize(); ++i)
        {
            outl[i] += tmpmixl[i] * outvol;
            outr[i] += tmpmixr[i] * outvol;
        }
    }

    delete[] tmpmixl;
    delete[] tmpmixr;

    //Mix all parts
    for (auto &npart : _channels)
    {
        if (npart.Penabled) //only mix active parts
        {
            for (unsigned int i = 0; i < this->BufferSize(); ++i)
            { //the volume did not changed
                outl[i] += npart.partoutl[i];
                outr[i] += npart.partoutr[i];
            }
        }
    }

    //Insertion effects for Master Out
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        if (Pinsparts[nefx] == -2)
        {
            insefx[nefx].out(outl, outr);
        }
    }

    //Master Volume
    for (unsigned int i = 0; i < this->BufferSize(); ++i)
    {
        outl[i] *= _volume;
        outr[i] *= _volume;
    }

    meter.Tick(outl, outr, _channels, _volume);

    //Shutup if it is asked (with fade-out)
    if (shutup)
    {
        for (unsigned int i = 0; i < this->BufferSize(); ++i)
        {
            float tmp = (this->BufferSizeFloat() - i) / this->BufferSizeFloat();
            outl[i] *= tmp;
            outr[i] *= tmp;
        }
        ShutUp();
    }

    //update the LFO's time
    LFOParams::time++;
}

IFFTwrapper *Mixer::GetFFT()
{
    return _fft.get();
}

int Mixer::GetChannelCount() const
{
    return NUM_MIXER_CHANNELS;
}

Channel *Mixer::GetChannel(int index)
{
    if (index >= 0 && index < NUM_MIXER_CHANNELS)
    {
        return &_channels[index];
    }

    return nullptr;
}

void Mixer::EnableChannel(int index, bool enabled)
{
    if (index >= NUM_MIXER_CHANNELS)
    {
        return;
    }

    meter.SetFakePeak(index, 0);

    if (enabled)
    { //enabled
        _channels[index].Penabled = 1;
        return;
    }

    _channels[index].Penabled = 0;
    _channels[index].Cleanup();
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        if (Pinsparts[nefx] == index)
        {
            insefx[nefx].cleanup();
        }
    }
}

/*
 * Parameter control
 */
void Mixer::setPvolume(unsigned char Pvolume_)
{
    Pvolume = Pvolume_;
    _volume = dB2rap((Pvolume - 96.0f) / 96.0f * 40.0f);
}

void Mixer::setPkeyshift(unsigned char Pkeyshift_)
{
    Pkeyshift = Pkeyshift_;
    _keyshift = static_cast<int>(Pkeyshift) - 64;
}

void Mixer::setPsysefxvol(int Ppart, int Pefx, unsigned char Pvol)
{
    Psysefxvol[Pefx][Ppart] = Pvol;
    _sysefxvol[Pefx][Ppart] = powf(0.1f, (1.0f - Pvol / 96.0f) * 2.0f);
}

unsigned char Mixer::GetSystemEffectSend(int Pefxfrom, int Pefxto)
{
    return Psysefxsend[Pefxfrom][Pefxto];
}

void Mixer::SetSystemEffectSend(int Pefxfrom, int Pefxto, unsigned char Pvol)
{
    Psysefxsend[Pefxfrom][Pefxto] = Pvol;
    _sysefxsend[Pefxfrom][Pefxto] = powf(0.1f, (1.0f - Pvol / 96.0f) * 2.0f);
}

/*
 * Panic! (Clean up all parts and effects)
 */
void Mixer::ShutUp()
{
    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        _channels[npart].Cleanup();
        meter.SetFakePeak(npart, 0);
    }
    for (auto &nefx : insefx)
    {
        nefx.cleanup();
    }
    for (auto &nefx : sysefx)
    {
        nefx.cleanup();
    }
    meter.ResetPeaks();
    shutup = 0;
}

void Mixer::applyparameters(bool lockmutex)
{
    for (auto &npart : _channels)
    {
        npart.ApplyParameters(lockmutex);
    }
}

int Mixer::getalldata(char **data)
{
    PresetsSerializer xml;

    xml.beginbranch("MASTER");

    Lock();
    Serialize(&xml);
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
    Deserialize(&xml);
    Unlock();

    xml.exitbranch();
}

int Mixer::saveXML(const char *filename)
{
    PresetsSerializer xml;

    xml.beginbranch("MASTER");
    Serialize(&xml);
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

    Deserialize(&xml);
    xml.exitbranch();

    return 0;
}

void Mixer::Serialize(IPresetsSerializer *xml)
{
    xml->addpar("volume", Pvolume);
    xml->addpar("key_shift", Pkeyshift);
    xml->addparbool("nrpn_receive", ctl.NRPN.receive);

    xml->beginbranch("MICROTONAL");
    microtonal.Serialize(xml);
    xml->endbranch();

    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        xml->beginbranch("PART", npart);
        _channels[npart].Serialize(xml);
        xml->endbranch();
    }

    xml->beginbranch("SYSTEM_EFFECTS");
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        xml->beginbranch("SYSTEM_EFFECT", nefx);
        xml->beginbranch("EFFECT");
        sysefx[nefx].Serialize(xml);
        xml->endbranch();

        for (int pefx = 0; pefx < NUM_MIXER_CHANNELS; ++pefx)
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
        insefx[nefx].Serialize(xml);
        xml->endbranch();
        xml->endbranch();
    }

    xml->endbranch();
}

void Mixer::Deserialize(IPresetsSerializer *xml)
{
    setPvolume(static_cast<unsigned char>(xml->getpar127("volume", Pvolume)));
    setPkeyshift(static_cast<unsigned char>(xml->getpar127("key_shift", Pkeyshift)));
    ctl.NRPN.receive = static_cast<unsigned char>(xml->getparbool("nrpn_receive", ctl.NRPN.receive));

    _channels[0].Penabled = 0;
    for (int npart = 0; npart < NUM_MIXER_CHANNELS; ++npart)
    {
        if (xml->enterbranch("PART", npart) == 0)
        {
            continue;
        }
        _channels[npart].Deserialize(xml);
        xml->exitbranch();
    }

    if (xml->enterbranch("MICROTONAL"))
    {
        microtonal.Deserialize(xml);
        xml->exitbranch();
    }

    sysefx[0].changeeffect(0);
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
                sysefx[nefx].Deserialize(xml);
                xml->exitbranch();
            }

            for (int partefx = 0; partefx < NUM_MIXER_CHANNELS; ++partefx)
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
                SetSystemEffectSend(nefx, tonefx, static_cast<unsigned char>(xml->getpar127("send_vol", Psysefxsend[nefx][tonefx])));
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
            Pinsparts[nefx] = static_cast<short>(xml->getpar("part", Pinsparts[nefx], -2, NUM_MIXER_CHANNELS));
            if (xml->enterbranch("EFFECT"))
            {
                insefx[nefx].Deserialize(xml);
                xml->exitbranch();
            }
            xml->exitbranch();
        }

        xml->exitbranch();
    }
}
