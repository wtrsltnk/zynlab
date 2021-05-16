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

Mixer::Mixer()
{
    for (int se = 0; se < NUM_SYS_EFX; se++)
    {
        for (int t = 0; t < NUM_MIXER_TRACKS; t++)
        {
            Psysefxvol[se][t] = 0.0f;
            _sysefxvol[se][t] = 0.0f;
        }
        for (int s = 0; s < NUM_SYS_EFX; s++)
        {
            Psysefxsend[se][s] = 0.0f;
            _sysefxsend[se][s] = 0.0f;
        }
    }

    for (int ie = 0; ie < NUM_INS_EFX; ie++)
    {
        Pinsparts[ie] = 0.0f;
    }
}

Mixer::~Mixer() = default;

void Mixer::Init()
{
    meter.Setup();
    ctl.Init();

    _tmpmixl = std::unique_ptr<float>(new float[this->BufferSize()]);
    _tmpmixr = std::unique_ptr<float>(new float[this->BufferSize()]);

    swaplr = false;

    shutup = false;

    for (auto &track : _tracks)
    {
        track.Init(this, &microtonal);
    }

    for (auto &effect : insefx)
    {
        effect.Init(this, true);
    }

    for (auto &effect : sysefx)
    {
        effect.Init(this, false);
    }

    Defaults();
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

    Psolotrack = DISABLED_MIXER_SOLO;

    for (int npart = 0; npart < NUM_MIXER_TRACKS; ++npart)
    {
        _tracks[npart].Defaults();
        _tracks[npart].Prcvchn = npart % NUM_MIDI_CHANNELS;
    }

    EnableTrack(0, 1);

    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        insefx[nefx].Defaults();
        Pinsparts[nefx] = -1;
    }

    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        sysefx[nefx].Defaults();
        for (int npart = 0; npart < NUM_MIXER_TRACKS; ++npart)
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
    _mutex.lock();
}

void Mixer::Unlock()
{
    _mutex.unlock();
}

std::mutex &Mixer::Mutex()
{
    return _mutex;
}

void Mixer::NoteOn(unsigned char chan, unsigned char note, unsigned char velocity)
{
    if (!velocity)
    {
        this->NoteOff(chan, note);
        return;
    }

    for (auto &track : _tracks)
    {
        if (chan == track.Prcvchn)
        {
            //            meter.SetFakePeak(npart, velocity * 2);
            if (track.Penabled)
            {
                track.NoteOn(note, velocity, _keyshift);
            }
        }
    }
}

void Mixer::NoteOff(unsigned char chan, unsigned char note)
{
    for (auto &npart : _tracks)
    {
        if ((chan == npart.Prcvchn) && npart.Penabled)
        {
            npart.NoteOff(note);
        }
    }
}

void Mixer::PolyphonicAftertouch(unsigned char chan, unsigned char note, unsigned char velocity)
{
    if (!velocity)
    {
        this->NoteOff(chan, note);
        return;
    }

    for (auto &npart : _tracks)
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
        // TODO reinstate this MIDI command
        // _bankManager->LoadBank(par);
    }
    else
    {                               //other controllers
        for (auto &track : _tracks) //Send the controller to all part assigned to the track
        {
            if ((chan == track.Prcvchn) && (track.Penabled != 0))
            {
                track.SetController(static_cast<unsigned int>(type), par);
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

    for (auto &npart : _tracks)
    {
        if (chan == npart.Prcvchn)
        {
            // TODO reinstate this MIDI command
            // _bankManager->LoadFromSlot(pgm, &npart);

            //Hack to get pad note parameters to update
            //this is not real time safe and makes assumptions about the calling
            //convention of this function...
            Unlock();
            npart.ApplyParameters();
            Lock();
        }
    }
}
void Mixer::PreviewNote(unsigned int channel, unsigned int note, unsigned int length, unsigned int velocity)
{
    NoteOn(channel, note, velocity);

    std::chrono::milliseconds::rep currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    auto found = std::find_if(_instrumentsPreview.begin(), _instrumentsPreview.end(), [channel, note](InstrumentPreview p) {
        return (p.note == note && p.channel == channel);
    });
    if (found != _instrumentsPreview.end())
    {
        (*found).done = false;
        (*found).playUntil = currentTime + length;
        return;
    }

    InstrumentPreview n;
    n.playUntil = currentTime + length;
    n.note = note;
    n.channel = channel;
    n.done = false;

    _instrumentsPreview.push_back(n);
}

INoteSource *Mixer::GetNoteSource() const
{
    return _noteSource;
}

void Mixer::SetNoteSource(INoteSource *source)
{
    _noteSource = source;
}

void Mixer::EnableTrack(int track, int enable)
{
    if (track >= NUM_MIXER_TRACKS)
    {
        return;
    }

    meter.SetFakePeak(track, 0);

    if (enable != 0)
    { //enabled
        _tracks[track].Penabled = 1;
        return;
    }

    _tracks[track].Penabled = 0;
    _tracks[track].Cleanup();
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        if (Pinsparts[nefx] == track)
        {
            insefx[nefx].cleanup();
        }
    }
}

void Mixer::PreviewSample(std::string const &filename)
{
    if (_samplePreview.wavData != nullptr)
    {
        delete _samplePreview.wavData;
        _samplePreview.wavData = nullptr;
    }

    _samplePreview.wavData = WavData::Load(filename);

    if (_samplePreview.wavData != nullptr)
    {
        _samplePreview.done = false;
        _samplePreview.wavProgress = 0;
    }
}

SamplePreview::SamplePreview()
    : wavData(nullptr), done(true)
{}

SamplePreview::~SamplePreview()
{}

void SamplePreview::noteout(float *outl, float *outr)
{
    if (done || wavData == nullptr)
    {
        return;
    }

    auto panning = 0.5f;

    for (unsigned int i = 0; i < SystemSettings::Instance().buffersize; ++i)
    {
        if (wavProgress < (wavData->samplesPerChannel * wavData->channels))
        {
            outl[i] += (wavData->PwavData[wavProgress++] * panning);
            outr[i] += (wavData->PwavData[wavProgress++] * (1.0f - panning));
        }
        else
        {
            done = true;
        }
    }
}

/*
 * Master audio out (the final sound)
 */
void Mixer::AudioOut(float *outl, float *outr)
{
    std::chrono::milliseconds::rep currentTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();

    if (_noteSource != nullptr)
    {
        auto notes = _noteSource->GetNotes(BufferSize(), SampleRate());
        for (auto n : notes)
        {
            PreviewNote(n.channel, n.note, n.length * 10, n.velocity);
        }
    }

    for (auto &tn : _instrumentsPreview)
    {
        if (tn.done) continue;
        if (tn.playUntil < currentTime)
        {
            tn.done = true;
            NoteOff(tn.channel, tn.note);
        }
    }

    //Swaps the Left channel with Right Channel
    if (swaplr)
    {
        swap(outl, outr);
    }

    //clean up the output samples (should not be needed?)
    memset(outl, 0, this->BufferSizeInBytes());
    memset(outr, 0, this->BufferSizeInBytes());

    //Compute part samples and store them _tracks[trackIndex].partoutl,partoutr
    for (int trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; ++trackIndex)
    {
        //skip if the part is disabled
        if (_tracks[trackIndex].Penabled == 0)
        {
            continue;
        }

        //skip if the part is not the solo track
        if (Psolotrack != DISABLED_MIXER_SOLO && Psolotrack != trackIndex)
        {
            continue;
        }

        _tracks[trackIndex].ComputeInstrumentSamples();
    }

    //Insertion effects
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        int trackIndex = Pinsparts[nefx];
        if (trackIndex < 0)
        {
            continue;
        }

        //skip if the part is disabled
        if (_tracks[trackIndex].Penabled == 0)
        {
            continue;
        }

        //skip if the part is not the solo track
        if (Psolotrack != DISABLED_MIXER_SOLO && Psolotrack != trackIndex)
        {
            continue;
        }

        insefx[nefx].out(_tracks[trackIndex].partoutl, _tracks[trackIndex].partoutr);
    }

    //Apply the part volumes and pannings (after insertion effects)
    for (auto &track : _tracks)
    {
        if (track.Penabled == 0)
        {
            continue;
        }

        Stereo<float> newvol(track.volume);
        Stereo<float> oldvol(track.oldvolumel, track.oldvolumer);

        float pan = track.panning;
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
                track.partoutl[i] *= vol._left;
                track.partoutr[i] *= vol._right;
            }
            track.oldvolumel = newvol._left;
            track.oldvolumer = newvol._right;
        }
        else
        {
            for (unsigned int i = 0; i < this->BufferSize(); ++i)
            { //the volume did not changed
                track.partoutl[i] *= newvol._left;
                track.partoutr[i] *= newvol._right;
            }
        }
    }

    //System effects
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        if (sysefx[nefx].geteffect() == 0)
        {
            continue; //the effect is disabled
        }

        //Clean up the samples used by the system effects
        memset(_tmpmixl.get(), 0, this->BufferSizeInBytes());
        memset(_tmpmixr.get(), 0, this->BufferSizeInBytes());

        //Mix the tracks according to the track settings about System Effect
        for (int trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; ++trackIndex)
        {
            //skip if the part has no output to effect
            if (Psysefxvol[nefx][trackIndex] == 0)
            {
                continue;
            }

            //skip if the part is disabled
            if (_tracks[trackIndex].Penabled == 0)
            {
                continue;
            }

            //skip if the part is not the solo track
            if (Psolotrack != DISABLED_MIXER_SOLO && Psolotrack != trackIndex)
            {
                continue;
            }

            //the output volume of each part to system effect
            const float vol = _sysefxvol[nefx][trackIndex];
            for (unsigned int i = 0; i < this->BufferSize(); ++i)
            {
                _tmpmixl.get()[i] += _tracks[trackIndex].partoutl[i] * vol;
                _tmpmixr.get()[i] += _tracks[trackIndex].partoutr[i] * vol;
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
                    _tmpmixl.get()[i] += sysefx[nefxfrom]._effectOutL[i] * vol;
                    _tmpmixr.get()[i] += sysefx[nefxfrom]._effectOutR[i] * vol;
                }
            }
        }

        sysefx[nefx].out(_tmpmixl.get(), _tmpmixr.get());

        //Add the System Effect to sound output
        const float outvol = sysefx[nefx].sysefxgetvolume();
        for (unsigned int i = 0; i < this->BufferSize(); ++i)
        {
            outl[i] += _tmpmixl.get()[i] * outvol;
            outr[i] += _tmpmixr.get()[i] * outvol;
        }
    }

    //Mix all parts
    for (int trackIndex = 0; trackIndex < NUM_MIXER_TRACKS; ++trackIndex)
    {
        //skip if the part is disabled
        if (_tracks[trackIndex].Penabled == 0)
        {
            continue;
        }

        //skip if the part is not the solo track
        if (Psolotrack != DISABLED_MIXER_SOLO && Psolotrack != trackIndex)
        {
            continue;
        }

        for (unsigned int i = 0; i < this->BufferSize(); ++i)
        { //the volume did not changed
            outl[i] += _tracks[trackIndex].partoutl[i];
            outr[i] += _tracks[trackIndex].partoutr[i];
        }
    }

    _samplePreview.noteout(outl, outr);

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

    meter.Tick(outl, outr, _tracks, _volume);

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

unsigned int Mixer::GetTrackCount() const
{
    return NUM_MIXER_TRACKS;
}

Track *Mixer::GetTrack(int index)
{
    if (index >= 0 && index < NUM_MIXER_TRACKS)
    {
        return &_tracks[index];
    }

    return nullptr;
}

void Mixer::EnableTrack(int index, bool enabled)
{
    if (index >= NUM_MIXER_TRACKS)
    {
        return;
    }

    meter.SetFakePeak(index, 0);

    if (enabled)
    { //enabled
        _tracks[index].Penabled = 1;
        return;
    }

    _tracks[index].Penabled = 0;
    _tracks[index].Cleanup();
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

void Mixer::ShutUp()
{
    for (int npart = 0; npart < NUM_MIXER_TRACKS; ++npart)
    {
        _tracks[npart].Cleanup();
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

    shutup = false;
}

void Mixer::ApplyParameters()
{
    for (auto &npart : _tracks)
    {
        npart.ApplyParameters();
    }
}

void Mixer::InitPresets()
{
    AddPreset("volume", &Pvolume);
    AddPreset("key_shift", &Pkeyshift);
    AddPresetAsBool("nrpn_receive", &ctl.NRPN.receive);

    microtonal.InitPresets();
    AddContainer(Preset("MICROTONAL", microtonal));

    for (int npart = 0; npart < NUM_MIXER_TRACKS; ++npart)
    {
        _tracks[npart].InitPresets();
        AddContainer(Preset("PART", npart, _tracks[npart]));
    }

    Preset systemEffects("SYSTEM_EFFECTS");
    for (int nefx = 0; nefx < NUM_SYS_EFX; ++nefx)
    {
        Preset systemEffect("SYSTEM_EFFECT", nefx);

        sysefx[nefx].InitPresets();
        systemEffect.AddContainer(Preset("EFFECT", sysefx[nefx]));

        for (int pefx = 0; pefx < NUM_MIXER_TRACKS; ++pefx)
        {
            Preset volume("VOLUME", pefx);
            volume.AddPreset("vol", &Psysefxvol[nefx][pefx]);
            systemEffect.AddContainer(volume);
        }

        for (int tonefx = nefx + 1; tonefx < NUM_SYS_EFX; ++tonefx)
        {
            Preset sendTo("SENDTO", tonefx);
            sendTo.AddPreset("send_vol", &Psysefxsend[nefx][tonefx]);
            systemEffect.AddContainer(sendTo);
        }

        systemEffects.AddContainer(systemEffect);
    }
    AddContainer(systemEffects);

    Preset insertionEffects("INSERTION_EFFECTS");
    for (int nefx = 0; nefx < NUM_INS_EFX; ++nefx)
    {
        Preset insertionEffect("INSERTION_EFFECT", nefx);
        {
            insertionEffect.AddPreset("part", &Pinsparts[nefx]);

            insefx[nefx].InitPresets();
            insertionEffect.AddContainer(Preset("EFFECT", insefx[nefx]));
        }
        insertionEffects.AddContainer(insertionEffect);
    }
    AddContainer(insertionEffects);
}
